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

#include <QDebug>

#include "genericdmxsource.h"
#include "sceneeditor.h"
#include "scenevalue.h"
#include "listmodel.h"
#include "tardis.h"
#include "scene.h"
#include "doc.h"
#include "app.h"

SceneEditor::SceneEditor(QQuickView *view, Doc *doc, QObject *parent)
    : FunctionEditor(view, doc, parent)
    , m_scene(nullptr)
    , m_sceneConsole(nullptr)
    , m_source(nullptr)
{
    m_view->rootContext()->setContextProperty("sceneEditor", this);
    m_source = new GenericDMXSource(m_doc);
    m_fixtureList = new ListModel(this);
    QStringList fRoles;
    fRoles << "cRef" << "isSelected";
    m_fixtureList->setRoleNames(fRoles);

    m_componentList = new ListModel(this);
    QStringList cRoles;
    cRoles << "type" << "cRef" << "isSelected";
    m_componentList->setRoleNames(cRoles);
}

SceneEditor::~SceneEditor()
{
    m_view->rootContext()->setContextProperty("sceneEditor", nullptr);
    QQuickItem *bottomPanel = qobject_cast<QQuickItem*>(m_view->rootObject()->findChild<QObject *>("bottomPanelItem"));
    if (bottomPanel != nullptr)
        bottomPanel->setProperty("visible", false);

    delete m_source;
    delete m_fixtureList;
}

void SceneEditor::setFunctionID(quint32 id)
{
    QQuickItem *bottomPanel = qobject_cast<QQuickItem*>(m_view->rootObject()->findChild<QObject *>("bottomPanelItem"));

    if (id == Function::invalidId())
    {
        disconnect(m_scene, &Scene::valueChanged, this, &SceneEditor::slotSceneValueChanged);
        m_scene = nullptr;
        m_source->unsetAll();
        m_source->setOutputEnabled(false);
        m_fixtureList->clear();
        m_fixtureIDs.clear();
        m_selectedChannels.clear();
        if (bottomPanel != nullptr)
            bottomPanel->setProperty("visible", false);
        return;
    }
    m_scene = qobject_cast<Scene *>(m_doc->function(id));

    connect(m_scene, &Scene::valueChanged, this, &SceneEditor::slotSceneValueChanged);

    updateLists();
    cacheChannelValues();

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
    QByteArray values = m_channelsCache[fixtureID];

    for (int i = 0; i < values.length(); i++)
        dmxValues.append(QString::number((uchar)values.at(i)));

    item->setProperty("values", QVariant::fromValue(dmxValues));
}

void SceneEditor::unRegisterFixtureConsole(int index)
{
    qDebug() << "[SceneEditor] Fixture console unregistered at index" << index;
    m_fxConsoleMap.take(index);
}

bool SceneEditor::hasChannel(quint32 fxID, quint32 channel)
{
    if (m_scene == nullptr)
        return false;

    return m_scene->checkValue(SceneValue(fxID, channel));
}

double SceneEditor::channelValue(quint32 fxID, quint32 channel)
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

int SceneEditor::selectedChannelCount()
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
    for (SceneValue scv : m_selectedChannels)
    {
        Fixture *sourceFixture = m_doc->fixture(scv.fxi);
        if (sourceFixture == nullptr)
            continue;

        uchar currentValue = m_scene->value(scv.fxi, scv.channel);

        for (quint32 dstFxId : m_scene->fixtures())
        {
            Fixture *destFixture = m_doc->fixture(scv.fxi);
            if (dstFxId == scv.fxi || destFixture == nullptr)
                continue;

            if (sourceFixture->fixtureDef() == destFixture->fixtureDef() &&
                sourceFixture->fixtureMode() == destFixture->fixtureMode())
            {
                SceneValue dstScv(dstFxId, scv.channel, currentValue);
                m_scene->setValue(dstScv);
                slotSceneValueChanged(dstScv);
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

    emit componentListChanged();
    emit fixtureListChanged();
}

void SceneEditor::setCacheChannelValue(SceneValue scv)
{
    if (m_channelsCache.contains(scv.fxi))
    {
        QByteArray values = m_channelsCache[scv.fxi];
        values[scv.channel] = scv.value;
        m_channelsCache[scv.fxi] = values;
    }
    else
    {
        Fixture *fixture = m_doc->fixture(scv.fxi);
        int chNumber = fixture->channels();
        QByteArray values;

        values.fill(0, chNumber);
        values[scv.channel] = scv.value;
        m_channelsCache[scv.fxi] = values;
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

