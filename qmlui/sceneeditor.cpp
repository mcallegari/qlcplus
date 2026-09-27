/*
  Q Light Controller Plus
  sceneeditor.cpp

  Copyright (c) Massimo Callegari

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0.txt

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
*/

#include <algorithm>
#include <QTimer>
#include <QDebug>

#include "genericdmxsource.h"
#include "contextmanager.h"
#include "virtualconsole.h"
#include "qlcinputprofile.h"
#include "qlcinputchannel.h"
#include "inputoutputmap.h"
#include "inputpatch.h"
#include "sceneeditor.h"
#include "scenevalue.h"
#include "qlcfixturemode.h"
#include "qlccapability.h"
#include "qlcphysical.h"
#include "qlcchannel.h"
#include "listmodel.h"
#include "universe.h"
#include "tardis.h"
#include "functionmanager.h"
#include "fixture.h"
#include "scene.h"
#include "doc.h"
#include "app.h"

/** Rate at which pan/tilt are moved while a relative control is held.
 *  50ms is smooth enough and matches the QLCInputSource relative loop */
#define POSITION_TIMER_INTERVAL 50

/** Maximum movement speed of a fully pushed stick, in degrees per second.
 *  Working in degrees makes the movement feel the same on every Fixture,
 *  regardless of its DMX resolution and physical range */
#define POSITION_MAX_DEGREES_PER_SECOND 90.0

/** Fraction of the stick travel around the center that is ignored, to
 *  compensate the resting noise of the analog sticks */
#define POSITION_STICK_DEADZONE 0.12

SceneEditor::SceneEditor(QQuickView *view, Doc *doc, QObject *parent)
    : FunctionEditor(view, doc, parent)
    , m_scene(nullptr)
    , m_sceneConsole(nullptr)
    , m_source(nullptr)
    , m_highlightSource(nullptr)
    , m_highlightedFixture(Fixture::invalidId())
    , m_externalControlEnabled(false)
    , m_panTiltMode(false)
    , m_externalPage(0)
    , m_panSpeed(0)
    , m_tiltSpeed(0)
    , m_panDegrees(0)
    , m_tiltDegrees(0)
    , m_positionTimer(nullptr)
{
    m_view->rootContext()->setContextProperty("sceneEditor", this);
    m_editorObjectName = "sceneEditorRoot";
    m_source = new GenericDMXSource(m_doc);
    m_highlightSource = new GenericDMXSource(m_doc);
    m_fixtureList = new ListModel(this);
    QStringList fRoles;
    fRoles << "cRef" << "isSelected";
    m_fixtureList->setRoleNames(fRoles);

    m_componentList = new ListModel(this);
    QStringList cRoles;
    cRoles << "type" << "cRef" << "isSelected";
    m_componentList->setRoleNames(cRoles);

    m_positionTimer = new QTimer(this);
    m_positionTimer->setInterval(POSITION_TIMER_INTERVAL);
    connect(m_positionTimer, SIGNAL(timeout()), this, SLOT(slotPositionTimeout()));
}

SceneEditor::~SceneEditor()
{
    /** Give the external controllers back to the Virtual Console */
    setExternalControlEnabled(false);

    m_view->rootContext()->setContextProperty("sceneEditor", nullptr);
    QQuickItem *bottomPanel = qobject_cast<QQuickItem*>(m_view->rootObject()->findChild<QObject *>("bottomPanelItem"));
    if (bottomPanel != nullptr)
        bottomPanel->setProperty("visible", false);

    delete m_source;
    delete m_highlightSource;
    delete m_fixtureList;
}

void SceneEditor::setFunctionID(quint32 id)
{
    QQuickItem *bottomPanel = qobject_cast<QQuickItem*>(m_view->rootObject()->findChild<QObject *>("bottomPanelItem"));

    if (id == Function::invalidId())
    {
        disconnect(m_scene, SIGNAL(valueChanged(SceneValue)), this, SLOT(slotSceneValueChanged(SceneValue)));
        m_scene = nullptr;
        m_source->unsetAll();
        m_source->setOutputEnabled(false);
        m_fixtureList->clear();
        m_fixtureIDs.clear();
        m_selectedChannels.clear();
        setExternalControlEnabled(false);
        if (bottomPanel != nullptr)
            bottomPanel->setProperty("visible", false);
        return;
    }
    m_scene = qobject_cast<Scene *>(m_doc->function(id));

    connect(m_scene, SIGNAL(valueChanged(SceneValue)), this, SLOT(slotSceneValueChanged(SceneValue)));

    updateLists();
    cacheChannelValues();
    updateChannelMap();

    if (bottomPanel != nullptr)
    {
        bottomPanel->setProperty("visible", true);
        bottomPanel->setProperty("editorSource", "qrc:/SceneFixtureConsole.qml");
    }
    FunctionEditor::setFunctionID(id);
}

QVariant SceneEditor::fixtureList() const
{
    return QVariant::fromValue(m_fixtureList);
}

QVariant SceneEditor::componentList() const
{
    return QVariant::fromValue(m_componentList);
}

void SceneEditor::setPreviewEnabled(bool enable)
{
    if (m_previewEnabled == enable)
        return;

    qDebug() << "[SceneEditor] set preview" << enable;

    if (enable == true)
    {
        foreach (SceneValue sv, m_scene->values())
            m_source->set(sv.fxi, sv.channel, sv.value);
    }
    else
        m_source->unsetAll();

    m_source->setOutputEnabled(enable);
    m_previewEnabled = enable;
    emit previewEnabledChanged(enable);
}

void SceneEditor::sceneConsoleLoaded(bool status)
{
    if (status == false)
    {
        m_sceneConsole = nullptr;
        m_fxConsoleMap.clear();
    }
    else
    {
        m_sceneConsole = qobject_cast<QQuickItem*>(m_view->rootObject()->findChild<QObject *>("sceneFixtureConsole"));
    }
}

void SceneEditor::registerFixtureConsole(int index, QQuickItem *item)
{
    qDebug() << "[SceneEditor] Fixture console registered at index" << index;
    m_fxConsoleMap[index] = item;

    if (index >= m_fixtureIDs.count())
    {
        qDebug() << "[SceneEditor] index out of bounds";
        return;
    }

    quint32 fixtureID = m_fixtureIDs[index];
    QVariantList dmxValues;
    QByteArray values;

    // initialize cache if fixture is not present
    if (!m_channelsCache.contains(fixtureID))
    {
        Fixture *fixture = m_doc->fixture(fixtureID);
        values.fill(0, fixture->channels());
        m_channelsCache[fixtureID] = values;
    }
    else
        values = m_channelsCache[fixtureID];

    for (int i = 0; i < values.length(); i++)
        dmxValues.append(QString::number((uchar)values.at(i)));

    item->setProperty("values", QVariant::fromValue(dmxValues));

}

void SceneEditor::unRegisterFixtureConsole(int index)
{
    qDebug() << "[SceneEditor] Fixture console unregistered at index" << index;
    m_fxConsoleMap.take(index);
}

bool SceneEditor::hasChannel(quint32 fxID, quint32 channel) const
{
    if (m_scene == nullptr)
        return false;

    return m_scene->checkValue(SceneValue(fxID, channel));
}

double SceneEditor::channelValue(quint32 fxID, quint32 channel) const
{
    if (m_scene == nullptr)
        return 0;

    return (double)m_scene->value(fxID, channel);
}

void SceneEditor::slotSceneValueChanged(SceneValue scv)
{
    bool blindMode = false;

    //qDebug() << "slotSceneValueChanged---- " << scv;

    if (m_source->isOutputEnabled() == false)
        blindMode = true;

    Fixture *fixture = m_doc->fixture(scv.fxi);
    if (fixture == nullptr)
        return;

    int fxIndex = m_fixtureIDs.indexOf(scv.fxi);

    if (fxIndex == -1)
    {
        connect(fixture, SIGNAL(aliasChanged()), this, SLOT(slotAliasChanged()));

        // Add the fixture to the side panel list
        QVariantMap fxMap;
        fxMap.insert("cRef", QVariant::fromValue(fixture));
        fxMap.insert("isSelected", false);
        m_fixtureList->addDataMap(fxMap);
        m_fixtureIDs.append(scv.fxi);

        // update the fixture list for the UI
        updateLists();
    }

    // update the channel values cache
    setCacheChannelValue(scv);

    if (m_sceneConsole)
    {
        if (m_fxConsoleMap.contains(fxIndex))
        {
            QMetaObject::invokeMethod(m_fxConsoleMap[fxIndex], "setChannelValue",
                    Q_ARG(QVariant, scv.channel),
                    Q_ARG(QVariant, scv.value));

            fixture->checkAlias(scv.channel, scv.value);
        }
    }

    if (blindMode == false)
        m_source->set(scv.fxi, scv.channel, scv.value);
}

void SceneEditor::slotAliasChanged()
{
    if (m_sceneConsole == nullptr)
        return;

    qDebug() << "Fixture alias changed";

    Fixture *fxi = qobject_cast<Fixture *>(sender());
    int fxIndex = m_fixtureIDs.indexOf(fxi->id());
    if (m_fxConsoleMap.contains(fxIndex))
        QMetaObject::invokeMethod(m_fxConsoleMap[fxIndex], "updateChannels");
}

void SceneEditor::unsetChannel(quint32 fxID, quint32 channel)
{
    if (m_scene == nullptr || m_fixtureIDs.contains(fxID) == false)
        return;

    QVariant currentVal;
    uchar currDmxValue = m_scene->value(fxID, channel);
    currentVal.setValue(SceneValue(fxID, channel, currDmxValue));
    Tardis::instance()->enqueueAction(Tardis::SceneUnsetChannelValue, m_scene->id(), currentVal, QVariant());

    m_scene->unsetValue(fxID, channel);
    if (m_source->isOutputEnabled() == true)
        m_source->unset(fxID, channel);
}

void SceneEditor::setFixtureSelection(quint32 fxID)
{
    if (m_scene == nullptr || m_sceneConsole == nullptr ||
        m_fixtureIDs.contains(fxID) == false)
            return;

    int fxIndex = m_fixtureIDs.indexOf(fxID);
    QMetaObject::invokeMethod(m_sceneConsole, "scrollToItem",
                              Q_ARG(QVariant, fxIndex));
}

void SceneEditor::setChannelSelection(quint32 fxID, quint32 channel, bool selected)
{
    SceneValue scv(fxID, channel);

    if (selected)
    {
        if (m_selectedChannels.contains(scv) == false)
            m_selectedChannels.append(scv);
    }
    else
    {
        m_selectedChannels.removeAll(scv);
    }

    emit selectedChannelCountChanged();
}

int SceneEditor::selectedChannelCount() const
{
    return m_selectedChannels.count();
}

void SceneEditor::addComponent(int type, quint32 id)
{
    if (m_scene == nullptr)
        return;

    switch(type)
    {
        case App::UniverseDragItem:
        break;
        case App::FixtureGroupDragItem:
            Tardis::instance()->enqueueAction(Tardis::SceneAddFixtureGroup, m_scene->id(), QVariant(), id);
            m_scene->addFixtureGroup(id);
            m_doc->setModified();
        break;
        case App::FixtureDragItem:
            Tardis::instance()->enqueueAction(Tardis::SceneAddFixture, m_scene->id(), QVariant(), id);
            m_scene->addFixture(id);
            m_doc->setModified();
        break;
        case App::PaletteDragItem:
            Tardis::instance()->enqueueAction(Tardis::SceneAddPalette, m_scene->id(), QVariant(), id);
            m_scene->addPalette(id);
            m_doc->setModified();
        break;
        default:
        break;
    }

    updateLists();
}

void SceneEditor::pasteToAllFixtureSameType()
{
    FunctionManager *functionManager = qobject_cast<FunctionManager*>(
                m_view->rootContext()->contextProperty("functionManager").value<QObject*>());
    bool editSequenceStep = (functionManager != nullptr &&
                             functionManager->currentEditor() != nullptr &&
                             functionManager->currentEditor()->functionType() == Function::SequenceType);

    for (SceneValue &scv : m_selectedChannels)
    {
        Fixture *sourceFixture = m_doc->fixture(scv.fxi);
        if (sourceFixture == nullptr)
            continue;

        uchar currentValue = m_scene->value(scv.fxi, scv.channel);

        for (quint32 &dstFxId : m_scene->fixtures())
        {
            Fixture *destFixture = m_doc->fixture(dstFxId);
            if (dstFxId == scv.fxi || destFixture == nullptr)
                continue;

            if (sourceFixture->fixtureDef() == destFixture->fixtureDef() &&
                sourceFixture->fixtureMode() == destFixture->fixtureMode())
            {
                SceneValue dstScv(dstFxId, scv.channel, currentValue);
                if (editSequenceStep)
                {
                    functionManager->setChannelValue(dstScv.fxi, dstScv.channel, dstScv.value);
                }
                else
                {
                    m_scene->setValue(dstScv);
                    slotSceneValueChanged(dstScv);
                }
            }
        }
    }
}

void SceneEditor::deleteItems(QVariantList list)
{
    if (m_scene == nullptr || list.isEmpty())
        return;

    for (QVariant &vIdx : list)
    {
        int index = vIdx.toInt();
        QVariantMap dataMap = m_componentList->itemAt(index).toMap();
        int type = dataMap["type"].toInt();

        switch(type)
        {
            case App::FixtureDragItem:
            {
                Fixture *fixture = dataMap["cRef"].value<Fixture *>();
                quint32 fixtureID = fixture->id();
                qDebug() << "removing fixture with ID" << fixtureID;

                for (SceneValue &scv : m_scene->values())
                {
                    if (scv.fxi == fixtureID)
                    {
                        QVariant currentVal;
                        currentVal.setValue(scv);
                        Tardis::instance()->enqueueAction(Tardis::SceneUnsetChannelValue, m_scene->id(), currentVal, QVariant());
                        m_scene->unsetValue(fixtureID, scv.channel);
                    }
                }
                Tardis::instance()->enqueueAction(Tardis::SceneRemoveFixture, m_scene->id(), fixtureID, QVariant());
                m_scene->removeFixture(fixtureID);
            }
            break;
            case App::FixtureGroupDragItem:
            {
                FixtureGroup *group = dataMap["cRef"].value<FixtureGroup *>();
                qDebug() << "removing fixture group with ID" << group->id();
                Tardis::instance()->enqueueAction(Tardis::SceneRemoveFixtureGroup, m_scene->id(), group->id(), QVariant());
                m_scene->removeFixtureGroup(group->id());
            }
            break;
            case App::PaletteDragItem:
            {
                QLCPalette *palette = dataMap["cRef"].value<QLCPalette *>();
                qDebug() << "removing palette with ID" << palette->id();
                Tardis::instance()->enqueueAction(Tardis::SceneRemovePalette, m_scene->id(), palette->id(), QVariant());
                m_scene->removePalette(palette->id());
            }
            break;
        }
    }

    m_doc->setModified();

    updateLists();
}

void SceneEditor::addFixtureToList(quint32 fid)
{
    if (m_fixtureIDs.contains(fid))
        return;

    Fixture *fixture = m_doc->fixture(fid);
    if (fixture == nullptr)
        return;

    connect(fixture, SIGNAL(aliasChanged()), this, SLOT(slotAliasChanged()));

    QVariantMap fxMap;
    fxMap.insert("cRef", QVariant::fromValue(fixture));
    fxMap.insert("isSelected", false);
    m_fixtureList->addDataMap(fxMap);

    QVariantMap fxcMap;
    fxcMap.insert("type", App::FixtureDragItem);
    fxcMap.insert("cRef", QVariant::fromValue(fixture));
    fxcMap.insert("isSelected", false);
    m_componentList->addDataMap(fxcMap);

    m_fixtureIDs.append(fid);
}

void SceneEditor::updateLists()
{
    if (m_scene == nullptr)
        return;

    for (quint32 &fxID : m_fixtureIDs)
    {
        Fixture *fixture = m_doc->fixture(fxID);
        if (fixture == nullptr)
            continue;

        disconnect(fixture, SIGNAL(aliasChanged()), this, SLOT(slotAliasChanged()));
    }

    m_fixtureIDs.clear();
    m_fixtureList->clear();
    m_componentList->clear();

    /** The component list order is:
     *  - Fixture groups
     *  - Palettes
     *  - Fixtures (means additional values were set manually)
     *
     *  Fixture list, instead, is to display Fixture consoles
     *  in a bottom panel, so it's the expanded list of all
     *  the Fixtures involved, including those in groups
     */

    // fixture groups
    for (quint32 &grpId : m_scene->fixtureGroups())
    {
        FixtureGroup *grp = m_doc->fixtureGroup(grpId);
        if (grp == nullptr)
            continue;

        QVariantMap grpMap;
        grpMap.insert("type", App::FixtureGroupDragItem);
        grpMap.insert("cRef", QVariant::fromValue(grp));
        grpMap.insert("isSelected", false);
        m_componentList->addDataMap(grpMap);

        for (quint32 &fxId : grp->fixtureList())
        {
            if (m_fixtureIDs.contains(fxId) == false)
            {
                Fixture *fixture = m_doc->fixture(fxId);
                if (fixture != nullptr)
                {
                    QVariantMap fxMap;
                    fxMap.insert("cRef", QVariant::fromValue(fixture));
                    fxMap.insert("isSelected", false);
                    m_fixtureList->addDataMap(fxMap);
                }

                m_fixtureIDs.append(fxId);
            }
        }
    }

    // palettes
    for (quint32 &pId : m_scene->palettes())
    {
        QLCPalette *palette = m_doc->palette(pId);
        if (palette == nullptr)
            continue;

        QVariantMap pMap;
        pMap.insert("type", App::PaletteDragItem);
        pMap.insert("cRef", QVariant::fromValue(palette));
        pMap.insert("isSelected", false);
        m_componentList->addDataMap(pMap);
    }

    // fixtures (there might be fixtures with no values set)
    for (quint32 &fId : m_scene->fixtures())
        addFixtureToList(fId);

    // scene values
    for (SceneValue &sv : m_scene->values())
        addFixtureToList(sv.fxi);

    // the Fixture set changed, so the faders mapping must follow
    updateChannelMap();

    emit componentListChanged();
    emit fixtureListChanged();
}

void SceneEditor::setCacheChannelValue(SceneValue scv)
{
    if (m_channelsCache.contains(scv.fxi))
    {
        QByteArray values = m_channelsCache[scv.fxi];
        if (values.length() < qsizetype(scv.channel))
            return;

        values[scv.channel] = scv.value;
        m_channelsCache[scv.fxi] = values;
    }
    else
    {
        Fixture *fixture = m_doc->fixture(scv.fxi);
        if (fixture == nullptr)
            return;

        int chNumber = fixture->channels();
        QByteArray values;

        values.fill(0, chNumber);
        values[scv.channel] = scv.value;
        m_channelsCache[scv.fxi] = values;
    }
}

/*********************************************************************
 * External controllers
 *********************************************************************/

bool SceneEditor::externalControlEnabled() const
{
    return m_externalControlEnabled;
}

void SceneEditor::setExternalControlEnabled(bool enable)
{
    if (m_externalControlEnabled == enable)
        return;

    VirtualConsole *vc = qobject_cast<VirtualConsole *>(
                m_view->rootContext()->contextProperty("virtualConsole").value<QObject *>());

    /** Raise the flag first: the helpers below check it to decide
     *  whether the external control is actually running */
    m_externalControlEnabled = enable;

    if (enable)
    {
        updateExternalMap();
        updateChannelMap();
        resetFaderPickup();

        connect(m_doc->inputOutputMap(), SIGNAL(inputValueChanged(quint32,quint32,uchar,QString)),
                this, SLOT(slotInputValueChanged(quint32,quint32,uchar)));
    }
    else
    {
        disconnect(m_doc->inputOutputMap(), SIGNAL(inputValueChanged(quint32,quint32,uchar,QString)),
                   this, SLOT(slotInputValueChanged(quint32,quint32,uchar)));

        clearExternalMap();
    }

    /** Inhibit/restore the Virtual Console input handling: while this
     *  editor uses the controller surface, the VC must not react */
    if (vc != nullptr)
        vc->enableExternalInput(!enable);

    if (enable)
        selectCurrentPageFixture();

    emit externalControlEnabledChanged();
    emit externalMapChanged();
}

bool SceneEditor::panTiltMode() const
{
    return m_panTiltMode;
}

void SceneEditor::setPanTiltMode(bool enable)
{
    if (m_panTiltMode == enable)
        return;

    m_panTiltMode = enable;

    /** The two modes have a completely different page meaning,
     *  so restart from the beginning */
    m_externalPage = 0;

    /** Drop any pending stick movement, the sticks may well be
     *  held away from the center while the mode is switched */
    m_panSpeed = 0;
    m_tiltSpeed = 0;
    m_positionTimer->stop();

    /** resetFaderPickup() rebuilds the DMX source, which transmits the
     *  position of the whole rig when entering the mode and drops
     *  everything when leaving it */
    resetFaderPickup();
    selectCurrentPageFixture();

    emit panTiltModeChanged();
    emit externalMapChanged();
}

int SceneEditor::faderCount() const
{
    return m_faders.count();
}

int SceneEditor::externalPage() const
{
    return m_externalPage;
}

int SceneEditor::externalPageCount() const
{
    if (m_panTiltMode)
        return m_movingFixtures.count();

    return m_channelPages.count();
}

bool SceneEditor::hasJoystick() const
{
    return m_joystickAxes.count() >= 2;
}

void SceneEditor::changeExternalPage(int direction)
{
    int pageCount = externalPageCount();
    if (pageCount == 0)
        return;

    int newPage = m_externalPage + direction;

    /** Do not wrap around: stop at the boundaries */
    newPage = qBound(0, newPage, pageCount - 1);

    if (newPage == m_externalPage)
        return;

    m_externalPage = newPage;

    /** The faders now point to different channels, so they must
     *  catch up again before taking control */
    resetFaderPickup();

    selectCurrentPageFixture();

    emit externalMapChanged();
}

void SceneEditor::clearExternalMap()
{
    /** No control is left to move pan/tilt, so drop both the highlight
     *  and the positions being transmitted */
    m_panSpeed = 0;
    m_tiltSpeed = 0;
    m_positionTimer->stop();

    m_highlightChannels.clear();
    m_highlightedFixture = Fixture::invalidId();
    m_highlightSource->unsetAll();
    m_highlightSource->setOutputEnabled(false);

    /** Stop the worker threads of the relative input sources, otherwise
     *  they would keep running and emitting values in the background */
    for (ExternalControl &control : m_faders)
    {
        if (control.source.isNull())
            continue;

        disconnect(control.source.data(), SIGNAL(inputValueChanged(quint32,quint32,uchar)),
                   this, SLOT(slotRelativeValueChanged(quint32,quint32,uchar)));
        control.source->setWorkingMode(QLCInputSource::Absolute);
    }

    m_faders.clear();
    m_nextPageControls.clear();
    m_prevPageControls.clear();
    m_joystickAxes.clear();
}

void SceneEditor::updateExternalMap()
{
    clearExternalMap();

    for (Universe *universe : m_doc->inputOutputMap()->universes())
    {
        InputPatch *patch = universe->inputPatch();
        if (patch == nullptr)
            continue;

        QLCInputProfile *profile = patch->profile();
        if (profile == nullptr)
            continue;

        bool isHID = (profile->type() == QLCInputProfile::HID);

        QMapIterator <quint32, QLCInputChannel*> it(profile->channels());
        while (it.hasNext() == true)
        {
            QLCInputChannel *channel = it.next().value();
            ExternalControl control = { universe->id(), it.key(),
                                        QSharedPointer<QLCInputSource>() };

            /** A channel declared as a relative movement (e.g. a spring
             *  loaded gamepad stick) does not report an absolute position.
             *  Delegate the conversion to a QLCInputSource in Relative
             *  working mode, exactly like the Virtual Console does */
            if (channel->movementType() == QLCInputChannel::Relative)
            {
                control.source = QSharedPointer<QLCInputSource>(
                            new QLCInputSource(universe->id(), it.key()));
                control.source->setSensitivity(channel->movementSensitivity());
                control.source->setWorkingMode(QLCInputSource::Relative);

                connect(control.source.data(), SIGNAL(inputValueChanged(quint32,quint32,uchar)),
                        this, SLOT(slotRelativeValueChanged(quint32,quint32,uchar)));
            }

            switch (channel->type())
            {
                case QLCInputChannel::Slider:
                case QLCInputChannel::Knob:
                    m_faders.append(control);
                    /** On a HID profile, sliders/knobs are the analog axes
                     *  of a joystick or gamepad */
                    if (isHID)
                        m_joystickAxes.append(control);
                break;
                case QLCInputChannel::NextPage:
                    m_nextPageControls.append(control);
                break;
                case QLCInputChannel::PrevPage:
                    m_prevPageControls.append(control);
                break;
                default:
                break;
            }
        }
    }

    qDebug() << "[SceneEditor] external map:" << m_faders.count() << "faders,"
             << m_joystickAxes.count() << "joystick axes";

    m_faderStates.clear();
    for (int i = 0; i < m_faders.count(); i++)
        m_faderStates.append({ false, false, 0 });
}

void SceneEditor::updateChannelMap()
{
    m_channelPages.clear();
    m_movingFixtures.clear();

    int faderCount = m_faders.count();

    for (quint32 &fxID : m_fixtureIDs)
    {
        Fixture *fixture = m_doc->fixture(fxID);
        if (fixture == nullptr)
            continue;

        /** Normal mode walks through the Fixtures in the same order as
         *  the bottom panel consoles, splitting each of them in pages of
         *  at most $faderCount channels. A page never spans two Fixtures,
         *  so the last page of a Fixture may be partially filled and the
         *  next one restarts from the first channel of the next Fixture */
        if (faderCount > 0)
        {
            quint32 channels = fixture->channels();

            for (quint32 first = 0; first < channels; first += faderCount)
                m_channelPages.append({ fxID, first,
                                        int(qMin(quint32(faderCount), channels - first)) });
        }

        /** Pan & Tilt mode uses one Fixture per page, so only
         *  the Fixtures that can actually move are relevant */
        if (fixture->channelNumber(QLCChannel::Pan, QLCChannel::MSB) != QLCChannel::invalid() ||
            fixture->channelNumber(QLCChannel::Tilt, QLCChannel::MSB) != QLCChannel::invalid())
                m_movingFixtures.append(fxID);
    }

    /** The available pages may have shrunk */
    int pageCount = externalPageCount();
    if (m_externalPage >= pageCount)
        m_externalPage = pageCount > 0 ? pageCount - 1 : 0;

    emit externalMapChanged();
}

void SceneEditor::resetFaderPickup()
{
    for (FaderState &state : m_faderStates)
    {
        state.caught = false;
        state.hasValue = false;
        state.lastValue = 0;
    }

    /** Seed the relative sources with the value of the channel they now
     *  point to, so that a movement starts from the actual channel
     *  position instead of the one of the previous page. This is the
     *  same purpose the Virtual Console achieves by sending feedback */
    for (int i = 0; i < m_faders.count(); i++)
    {
        const ExternalControl &control = m_faders.at(i);
        if (control.source.isNull() || m_scene == nullptr)
            continue;

        SceneValue target = faderTarget(i);
        if (target.fxi != Fixture::invalidId() && target.channel != QLCChannel::invalid())
            control.source->updateOuputValue(m_scene->value(target.fxi, target.channel));
    }

    /** Restart the position accumulators from the Fixture of the new page */
    cachePositionDegrees();

    /** Rebuild the whole DMX source: this resets the highlight of the
     *  previously selected Fixture and enables the one of the new page.
     *  A full rebuild is needed because the channels of a GenericDMXSource
     *  can only be released all at once */
    outputAllPositions();
}

SceneValue SceneEditor::faderTarget(int faderIndex) const
{
    SceneValue invalidTarget(Fixture::invalidId(), QLCChannel::invalid());

    if (faderIndex < 0 || faderIndex >= m_faders.count())
        return invalidTarget;

    if (m_panTiltMode)
    {
        /** Only the position channels are used, and they are assigned to
         *  the faders in Fixture channel order, so that the faders layout
         *  matches the channel layout of the Fixture console */
        if (faderIndex > 3 || m_externalPage >= m_movingFixtures.count())
            return invalidTarget;

        Fixture *fixture = m_doc->fixture(m_movingFixtures.at(m_externalPage));
        if (fixture == nullptr)
            return invalidTarget;

        static const int types[4] = { QLCChannel::Pan, QLCChannel::Pan,
                                      QLCChannel::Tilt, QLCChannel::Tilt };
        static const int bytes[4] = { QLCChannel::MSB, QLCChannel::LSB,
                                      QLCChannel::MSB, QLCChannel::LSB };

        QList<quint32> positionChannels;

        for (int i = 0; i < 4; i++)
        {
            quint32 channel = fixture->channelNumber(types[i], bytes[i]);
            if (channel != QLCChannel::invalid())
                positionChannels.append(channel);
        }

        std::sort(positionChannels.begin(), positionChannels.end());

        if (faderIndex >= positionChannels.count())
            return invalidTarget;

        return SceneValue(fixture->id(), positionChannels.at(faderIndex));
    }

    if (m_externalPage >= m_channelPages.count())
        return invalidTarget;

    const ChannelPage &page = m_channelPages.at(m_externalPage);

    /** The last page of a Fixture may use less faders than available */
    if (faderIndex >= page.channelCount)
        return invalidTarget;

    return SceneValue(page.fixtureID, page.firstChannel + faderIndex);
}

int SceneEditor::controlledFaderIndex(quint32 fxID, quint32 channel) const
{
    if (m_externalControlEnabled == false)
        return -1;

    for (int i = 0; i < m_faders.count(); i++)
    {
        SceneValue target = faderTarget(i);
        if (target.fxi == fxID && target.channel == channel)
            return i;
    }

    return -1;
}

void SceneEditor::selectCurrentPageFixture()
{
    if (m_externalControlEnabled == false)
        return;

    SceneValue target = faderTarget(0);
    if (target.fxi == Fixture::invalidId())
        return;

    ContextManager *contextManager = qobject_cast<ContextManager *>(
                m_view->rootContext()->contextProperty("contextManager").value<QObject *>());

    /** Reuse the standard Fixture selection, so the Fixture is highlighted
     *  in every view exactly as if the user had picked it manually */
    if (contextManager != nullptr)
    {
        contextManager->resetFixtureSelection();
        contextManager->setFixtureIDSelection(target.fxi, true);
    }

    /** Highlight the Fixture in the side panel component list too */
    for (int i = 0; i < m_componentList->rowCount(); i++)
    {
        QVariantMap dataMap = m_componentList->itemAt(i).toMap();
        if (dataMap["type"].toInt() != App::FixtureDragItem)
            continue;

        Fixture *fixture = dataMap["cRef"].value<Fixture *>();
        bool selected = (fixture != nullptr && fixture->id() == target.fxi);

        m_componentList->setDataWithRole(m_componentList->index(i, 0, QModelIndex()),
                                         "isSelected", selected);
    }

    /** ...and scroll the bottom panel to it */
    setFixtureSelection(target.fxi);
}

void SceneEditor::applyFaderValue(int faderIndex, uchar value)
{
    if (faderIndex < 0 || faderIndex >= m_faderStates.count())
        return;

    SceneValue target = faderTarget(faderIndex);

    if (target.fxi == Fixture::invalidId() || target.channel == QLCChannel::invalid())
        return;

    FaderState &state = m_faderStates[faderIndex];
    uchar currentValue = m_scene != nullptr ? m_scene->value(target.fxi, target.channel) : 0;

    /** Pickup (catch up): the fader takes control of the channel only
     *  once its physical position crosses the current channel value.
     *  This avoids the value jumping when the mapping shifts */
    if (state.caught == false)
    {
        if (state.hasValue == false)
        {
            /** First value received: just remember the fader position,
             *  there is no movement direction to evaluate yet */
            state.hasValue = true;
            state.lastValue = value;

            /** ...unless the fader is already exactly on the value */
            if (value != currentValue)
                return;
        }
        else if ((state.lastValue < currentValue && value >= currentValue) ||
                 (state.lastValue > currentValue && value <= currentValue) ||
                 value == currentValue)
        {
            /** The fader crossed the channel value */
        }
        else
        {
            state.lastValue = value;
            return;
        }

        state.caught = true;
    }

    state.lastValue = value;

    FunctionManager *functionManager = qobject_cast<FunctionManager *>(
                m_view->rootContext()->contextProperty("functionManager").value<QObject *>());

    if (functionManager != nullptr)
        functionManager->setChannelValue(target.fxi, target.channel, value);
}

void SceneEditor::slotInputValueChanged(quint32 universe, quint32 channel, uchar value)
{
    if (m_externalControlEnabled == false || m_scene == nullptr)
        return;

    ExternalControl control = { universe, channel, QSharedPointer<QLCInputSource>() };

    /** Page buttons: react on press only, to not shift twice per click */
    if (m_nextPageControls.contains(control))
    {
        if (value > 0)
            changeExternalPage(1);
        return;
    }

    if (m_prevPageControls.contains(control))
    {
        if (value > 0)
            changeExternalPage(-1);
        return;
    }

    /** In Pan & Tilt mode a joystick, when present, drives the position
     *  with its analog sticks: axis 0/1 are pan/tilt of the left stick,
     *  axis 2/3 the fine adjustment of the right one */
    if (m_panTiltMode && hasJoystick())
    {
        int axisIndex = m_joystickAxes.indexOf(control);
        if (axisIndex >= 0)
        {
            if (axisIndex < 4)
                feedFaderValue(axisIndex, value);
            return;
        }
    }

    int faderIndex = m_faders.indexOf(control);
    if (faderIndex >= 0)
        feedFaderValue(faderIndex, value);
}

void SceneEditor::feedFaderValue(int faderIndex, uchar value)
{
    if (faderIndex < 0 || faderIndex >= m_faders.count())
        return;

    const ExternalControl &control = m_faders.at(faderIndex);

    /** A relative control reports a displacement, not a position */
    if (control.source.isNull() == false)
    {
        /** In Pan & Tilt mode the movement is expressed in degrees, so that
         *  it feels identical on any Fixture no matter its DMX resolution
         *  or physical range. Plain channels keep using the DMX stepping
         *  of QLCInputSource, whose result arrives asynchronously in
         *  slotRelativeValueChanged() */
        if (m_panTiltMode)
            feedPositionAxis(faderIndex, value);
        else
            control.source->updateInputValue(value);

        return;
    }

    applyFaderValue(faderIndex, value);
}

void SceneEditor::slotRelativeValueChanged(quint32 universe, quint32 channel, uchar value)
{
    if (m_externalControlEnabled == false || m_scene == nullptr)
        return;

    ExternalControl control = { universe, channel, QSharedPointer<QLCInputSource>() };

    int faderIndex = m_faders.indexOf(control);
    if (faderIndex < 0)
        return;

    SceneValue target = faderTarget(faderIndex);
    if (target.fxi == Fixture::invalidId() || target.channel == QLCChannel::invalid())
        return;

    /** The value is already absolute and relative to the current channel
     *  position, so no pickup check is needed here */
    FunctionManager *functionManager = qobject_cast<FunctionManager *>(
                m_view->rootContext()->contextProperty("functionManager").value<QObject *>());

    if (functionManager != nullptr)
        functionManager->setChannelValue(target.fxi, target.channel, value);

    /** Acknowledge the value back to the source, so it keeps stepping from
     *  the position the channel actually reached */
    m_faders.at(faderIndex).source->updateOuputValue(value);
}

void SceneEditor::feedPositionAxis(int faderIndex, uchar value)
{
    /** Convert the raw stick position into a normalized displacement from
     *  its center: -1.0 fully pushed one way, +1.0 the other way, 0 at rest */
    float speed = (float(value) - 127.0f) / 127.0f;

    /** Analog sticks never rest exactly at the center, so ignore
     *  the small displacements around it */
    if (qAbs(speed) < POSITION_STICK_DEADZONE)
        speed = 0;

    /** Faders 0/1 are the coarse pan/tilt axes. The fine ones (2/3) move
     *  at a fraction of the speed, to allow precise adjustments */
    switch (faderIndex)
    {
        case 0: m_panSpeed = speed; break;
        case 1: m_tiltSpeed = speed; break;
        case 2: m_panSpeed = speed * 0.1f; break;
        case 3: m_tiltSpeed = speed * 0.1f; break;
        default: return;
    }

    /** Run the timer only while there is something to move */
    if (m_panSpeed == 0 && m_tiltSpeed == 0)
        m_positionTimer->stop();
    else if (m_positionTimer->isActive() == false)
        m_positionTimer->start();
}

void SceneEditor::cachePositionDegrees()
{
    m_panDegrees = 0;
    m_tiltDegrees = 0;

    if (m_scene == nullptr || m_externalPage >= m_movingFixtures.count())
        return;

    Fixture *fixture = m_doc->fixture(m_movingFixtures.at(m_externalPage));
    if (fixture == nullptr || fixture->fixtureMode() == nullptr)
        return;

    QLCPhysical phy = fixture->fixtureMode()->physical();

    int panMax = phy.focusPanMax() ? phy.focusPanMax() : 360;
    int tiltMax = phy.focusTiltMax() ? phy.focusTiltMax() : 270;

    /** Rebuild the 16 bit position stored in the Scene and turn it
     *  back into degrees, so that a movement continues from there */
    quint32 panMSB = fixture->channelNumber(QLCChannel::Pan, QLCChannel::MSB);
    quint32 panLSB = fixture->channelNumber(QLCChannel::Pan, QLCChannel::LSB);
    quint32 tiltMSB = fixture->channelNumber(QLCChannel::Tilt, QLCChannel::MSB);
    quint32 tiltLSB = fixture->channelNumber(QLCChannel::Tilt, QLCChannel::LSB);

    if (panMSB != QLCChannel::invalid())
    {
        quint16 panDmx = quint16(m_scene->value(fixture->id(), panMSB)) << 8;
        if (panLSB != QLCChannel::invalid())
            panDmx |= m_scene->value(fixture->id(), panLSB);

        m_panDegrees = (float(panDmx) * panMax) / 65535.0f;
    }

    if (tiltMSB != QLCChannel::invalid())
    {
        quint16 tiltDmx = quint16(m_scene->value(fixture->id(), tiltMSB)) << 8;
        if (tiltLSB != QLCChannel::invalid())
            tiltDmx |= m_scene->value(fixture->id(), tiltLSB);

        m_tiltDegrees = (float(tiltDmx) * tiltMax) / 65535.0f;
    }
}

void SceneEditor::clearFixtureHighlight()
{
    m_highlightedFixture = Fixture::invalidId();
    m_highlightChannels.clear();
}

void SceneEditor::outputAllPositions()
{
    /** Zero the channels raised by the previous highlight before rebuilding.
     *  Note that GenericDMXSource::unsetAll() cannot be used here: it defers
     *  a clear request that is processed by writeDMX() *after* the values
     *  have been written, so it would wipe the values set right below.
     *  Overwriting with 0 releases the Fixture without that race */
    for (SceneValue &scv : m_highlightChannels)
        m_highlightSource->set(scv.fxi, scv.channel, 0);

    m_highlightChannels.clear();
    m_highlightedFixture = Fixture::invalidId();

    if (m_externalControlEnabled == false || m_panTiltMode == false || m_scene == nullptr)
    {
        m_highlightSource->setOutputEnabled(false);
        m_highlightSource->unsetAll();
        return;
    }

    /** Transmit the position of every moving Fixture, so that the whole
     *  rig shows where it is pointing as soon as the editing begins */
    for (quint32 &fxID : m_movingFixtures)
    {
        Fixture *fixture = m_doc->fixture(fxID);
        if (fixture == nullptr || fixture->fixtureMode() == nullptr)
            continue;

        QLCPhysical phy = fixture->fixtureMode()->physical();

        int panMax = phy.focusPanMax() ? phy.focusPanMax() : 360;
        int tiltMax = phy.focusTiltMax() ? phy.focusTiltMax() : 270;

        quint32 panMSB = fixture->channelNumber(QLCChannel::Pan, QLCChannel::MSB);
        quint32 panLSB = fixture->channelNumber(QLCChannel::Pan, QLCChannel::LSB);
        quint32 tiltMSB = fixture->channelNumber(QLCChannel::Tilt, QLCChannel::MSB);
        quint32 tiltLSB = fixture->channelNumber(QLCChannel::Tilt, QLCChannel::LSB);

        if (panMSB != QLCChannel::invalid())
        {
            quint16 dmx = quint16(m_scene->value(fxID, panMSB)) << 8;
            if (panLSB != QLCChannel::invalid())
                dmx |= m_scene->value(fxID, panLSB);

            for (SceneValue &scv : fixture->positionToValues(QLCChannel::Pan,
                                                             (float(dmx) * panMax) / 65535.0f))
                m_highlightSource->set(scv.fxi, scv.channel, scv.value);
        }

        if (tiltMSB != QLCChannel::invalid())
        {
            quint16 dmx = quint16(m_scene->value(fxID, tiltMSB)) << 8;
            if (tiltLSB != QLCChannel::invalid())
                dmx |= m_scene->value(fxID, tiltLSB);

            for (SceneValue &scv : fixture->positionToValues(QLCChannel::Tilt,
                                                             (float(dmx) * tiltMax) / 65535.0f))
                m_highlightSource->set(scv.fxi, scv.channel, scv.value);
        }
    }

    /** Light up the selected Fixture on top of the positions */
    highlightCurrentFixture();

    m_highlightSource->setOutputEnabled(true);
}

void SceneEditor::highlightCurrentFixture()
{
    clearFixtureHighlight();

    if (m_externalControlEnabled == false || m_panTiltMode == false ||
        m_externalPage >= m_movingFixtures.count())
            return;

    quint32 fxID = m_movingFixtures.at(m_externalPage);
    Fixture *fixture = m_doc->fixture(fxID);
    if (fixture == nullptr)
        return;

    /** Open only this Fixture up at full, so it can be told apart from the
     *  others while its position is adjusted. The positions of the whole
     *  rig are transmitted separately by outputAllPositions().
     *  Every value goes to the dedicated DMX source, so the Scene being
     *  edited is left untouched */
    for (quint32 i = 0; i < fixture->channels(); i++)
    {
        const QLCChannel *channel = fixture->channel(i);
        if (channel == nullptr)
            continue;

        switch (channel->group())
        {
            case QLCChannel::Intensity:
            {
                /** Dimmer and white light up, coloured channels stay off,
                 *  otherwise the beam would turn white on RGB Fixtures */
                if (channel->colour() == QLCChannel::NoColour ||
                    channel->colour() == QLCChannel::White)
                {
                    m_highlightSource->set(fxID, i, UCHAR_MAX);
                    m_highlightChannels.append(SceneValue(fxID, i));
                }
            }
            break;
            case QLCChannel::Shutter:
            {
                /** Look for an explicit "shutter open" capability */
                for (QLCCapability *cap : channel->capabilities())
                {
                    if (cap->preset() == QLCCapability::ShutterOpen)
                    {
                        m_highlightSource->set(fxID, i, cap->middle());
                        m_highlightChannels.append(SceneValue(fxID, i));
                        break;
                    }
                }
            }
            break;
            case QLCChannel::Maintenance:
            {
                /** Some Fixtures need the lamp to be explicitly struck */
                for (QLCCapability *cap : channel->capabilities())
                {
                    if (cap->preset() == QLCCapability::LampOn)
                    {
                        m_highlightSource->set(fxID, i, cap->middle());
                        m_highlightChannels.append(SceneValue(fxID, i));
                        break;
                    }
                }
            }
            break;
            default:
            break;
        }
    }

    m_highlightedFixture = fxID;
    m_highlightSource->setOutputEnabled(true);
}

void SceneEditor::slotPositionTimeout()
{
    if (m_externalControlEnabled == false || m_panTiltMode == false ||
        m_externalPage >= m_movingFixtures.count())
    {
        m_positionTimer->stop();
        return;
    }

    Fixture *fixture = m_doc->fixture(m_movingFixtures.at(m_externalPage));
    if (fixture == nullptr || fixture->fixtureMode() == nullptr)
        return;

    FunctionManager *functionManager = qobject_cast<FunctionManager *>(
                m_view->rootContext()->contextProperty("functionManager").value<QObject *>());
    if (functionManager == nullptr)
        return;

    QLCPhysical phy = fixture->fixtureMode()->physical();

    /** How many degrees to move on this tick, proportional to how far
     *  the stick is pushed away from its center */
    float step = (POSITION_MAX_DEGREES_PER_SECOND * POSITION_TIMER_INTERVAL) / 1000.0f;

    /** The absolute position is accumulated here instead of relying on
     *  Fixture::positionToValues() relative mode: that one derives the
     *  current position from the DMX output, which does not move while
     *  the Scene is being edited in blind mode */
    if (m_panSpeed != 0)
    {
        int panMax = phy.focusPanMax() ? phy.focusPanMax() : 360;

        m_panDegrees = qBound(0.0f, m_panDegrees + (m_panSpeed * step), float(panMax));

        for (SceneValue &scv : fixture->positionToValues(QLCChannel::Pan, m_panDegrees))
        {
            functionManager->setChannelValue(scv.fxi, scv.channel, scv.value);

            /** Keep the transmitted position up to date, otherwise the beam
             *  would not move while the Scene is edited in blind mode */
            m_highlightSource->set(scv.fxi, scv.channel, scv.value);
        }
    }

    if (m_tiltSpeed != 0)
    {
        int tiltMax = phy.focusTiltMax() ? phy.focusTiltMax() : 270;

        m_tiltDegrees = qBound(0.0f, m_tiltDegrees + (m_tiltSpeed * step), float(tiltMax));

        for (SceneValue &scv : fixture->positionToValues(QLCChannel::Tilt, m_tiltDegrees))
        {
            functionManager->setChannelValue(scv.fxi, scv.channel, scv.value);

            m_highlightSource->set(scv.fxi, scv.channel, scv.value);
        }
    }
}

void SceneEditor::cacheChannelValues()
{
    m_channelsCache.clear();

    for (quint32 &pId : m_scene->palettes())
    {
        QLCPalette *palette = m_doc->palette(pId);
        if (palette == nullptr)
            continue;

        for (SceneValue &scv : palette->valuesFromFixtureGroups(m_doc, m_scene->fixtureGroups()))
            setCacheChannelValue(scv);

        for (SceneValue &scv : palette->valuesFromFixtures(m_doc, m_scene->fixtures()))
            setCacheChannelValue(scv);
    }

    for (SceneValue &scv : m_scene->values())
        setCacheChannelValue(scv);
}
