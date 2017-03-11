/*
  Q Light Controller Plus
  fixturemanager.cpp

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

#include <QQuickItem>
#include <QQmlEngine>
#include <QVariant>
#include <QDebug>
#include <QtMath>

#include "fixturemanager.h"
#include "qlcfixturemode.h"
#include "qlccapability.h"
#include "qlcfixturedef.h"
#include "fixture.h"
#include "doc.h"
#include "app.h"

FixtureManager::FixtureManager(QQuickView *view, Doc *doc, QObject *parent)
    : QObject(parent)
    , m_view(view)
    , m_doc(doc)
    , m_fixtureTree(NULL)
    , m_universeFilter(Universe::invalid())
    , m_maxPanDegrees(0)
    , m_maxTiltDegrees(0)
    , m_colorsMask(0)
{
    Q_ASSERT(m_doc != NULL);

    qmlRegisterType<QLCCapability>("com.qlcplus.classes", 1, 0, "QLCCapability");

    connect(m_doc, SIGNAL(loaded()),
            this, SLOT(slotDocLoaded()));
}

quint32 FixtureManager::invalidFixture()
{
    return Fixture::invalidId();
}

quint32 FixtureManager::fixtureForAddress(quint32 index)
{
    return m_doc->fixtureForAddress(index);
}

QVariantList FixtureManager::fixtureSelection(quint32 address)
{
    QVariantList list;
    quint32 uniFilter = m_universeFilter == Universe::invalid() ? 0 : m_universeFilter;

    quint32 fxID = m_doc->fixtureForAddress((uniFilter << 9) | address);
    if (fxID == Fixture::invalidId())
        return list;

    Fixture *fixture = m_doc->fixture(fxID);
    if (fixture == NULL)
        return list;

    quint32 startAddr = fixture->address();
    for (quint32 i = 0; i < fixture->channels(); i++)
    {
        list.append(startAddr + i);
        QLCChannel::Group group = fixture->channel(i)->group();
        if (group == QLCChannel::Intensity)
            list.append(fixture->channel(i)->colour());
        else
            list.append(group - 1);
    }

    return list;
}

bool FixtureManager::addFixture(QString manuf, QString model, QString mode, QString name,
                                int uniIdx, int address, int channels, int quantity, quint32 gap,
                                qreal xPos, qreal yPos)
{
    qDebug() << Q_FUNC_INFO << manuf << model << quantity;

    QLCFixtureDef *fxiDef = m_doc->fixtureDefCache()->fixtureDef(manuf, model);
    Q_ASSERT(fxiDef != NULL);

    QLCFixtureMode *fxiMode = fxiDef->mode(mode);
    Q_ASSERT(fxiMode != NULL);

    for (int i = 0; i < quantity; i++)
    {
        Fixture *fxi = new Fixture(m_doc);

        /* If we're adding more than one fixture,
           append a number to the end of the name */
        if (quantity > 1)
            fxi->setName(QString("%1 #%2").arg(name).arg(i + 1));
        else
            fxi->setName(name);
        fxi->setAddress(address + (i * channels) + (i * gap));
        fxi->setUniverse(uniIdx);
        fxi->setFixtureDefinition(fxiDef, fxiMode);

        m_doc->addFixture(fxi);
        emit newFixtureCreated(fxi->id(), xPos, yPos);
    }
    m_fixtureList.clear();
    m_fixtureList = m_doc->fixtures();
    emit fixturesCountChanged();

    updateFixtureTree(m_doc, m_fixtureTree);
    emit groupsTreeModelChanged();
    emit groupsListModelChanged();
    emit fixtureNamesMapChanged();
    emit fixturesMapChanged();

    return true;
}

bool FixtureManager::moveFixture(quint32 fixtureID, quint32 newAddress)
{
    qDebug() << "[FixtureManager] requested to move fixture with ID" << fixtureID << "to address" << newAddress;
    Fixture *fixture = m_doc->fixture(fixtureID);
    if (fixture == NULL)
        return false;

    fixture->setAddress(newAddress);

    updateFixtureTree(m_doc, m_fixtureTree);
    emit groupsTreeModelChanged();
    emit groupsListModelChanged();
    emit fixtureNamesMapChanged();
    emit fixturesMapChanged();
    return true;
}

QString FixtureManager::channelIcon(quint32 fxID, quint32 chIdx)
{
    Fixture *fixture = m_doc->fixture(fxID);
    if (fixture == NULL)
        return QString();

    const QLCChannel *channel = fixture->channel(chIdx);
    if (channel == NULL)
        return QString();

    return channel->getIconNameFromGroup(channel->group(), true);
}

void FixtureManager::setChannelValue(quint32 fixtureID, quint32 channelIndex, quint8 value)
{
    emit channelValueChanged(fixtureID, channelIndex, value);
}

void FixtureManager::setIntensityValue(quint8 value)
{
    emit channelTypeValueChanged(QLCChannel::Intensity, value);
}

void FixtureManager::setColorValue(quint8 red, quint8 green, quint8 blue,
                                   quint8 white, quint8 amber, quint8 uv)
{
    emit colorChanged(QColor(red, green, blue), QColor(white, amber, uv));
}

void FixtureManager::setPanValue(int degrees)
{
    emit positionTypeValueChanged(QLCChannel::Pan, degrees);
}

void FixtureManager::setTiltValue(int degrees)
{
    emit positionTypeValueChanged(QLCChannel::Tilt, degrees);
}

void FixtureManager::setPresetValue(int index, quint8 value)
{
    qDebug() << "[FixtureManager] setPresetValue - index:" << index << "value:" << value;
    QList<const QLCChannel*>channels = m_presetsCache.keys();

    if (index < 0 || index >= channels.count())
        return;

    const QLCChannel* ch = channels.at(index);
    emit presetChanged(ch, value);
}

QMultiHash<int, SceneValue> FixtureManager::getFixtureCapabilities(quint32 fxID, bool enable)
{
    int capDelta = enable ? 1 : -1;
    bool hasDimmer = false, hasColor = false, hasPosition = false;
    bool hasColorWheel = false, hasGobos = false;
    int origColorsMask = m_colorsMask;

    QMultiHash<int, SceneValue> channelsMap;

    Fixture *fixture = m_doc->fixture(fxID);
    if (fixture == NULL)
        return channelsMap;

    for (quint32 ch = 0; ch < fixture->channels(); ch++)
    {
        const QLCChannel* channel(fixture->channel(ch));
        if(channel == NULL)
            continue;

        int chType = channel->group();

        switch (channel->group())
        {
            case QLCChannel::Intensity:
            {
                QLCChannel::PrimaryColour col = channel->colour();
                chType = col;
                switch (col)
                {
                    case QLCChannel::NoColour:
                        hasDimmer = true;
                        channelsMap.insert(chType, SceneValue(fxID, ch));
                    break;
                    case QLCChannel::Red:
                    case QLCChannel::Green:
                    case QLCChannel::Blue:
                    case QLCChannel::Cyan:
                    case QLCChannel::Magenta:
                    case QLCChannel::Yellow:
                    case QLCChannel::White:
                    case QLCChannel::Amber:
                    case QLCChannel::UV:
                    case QLCChannel::Lime:
                    case QLCChannel::Indigo:
                        hasColor = true;
                        updateColorsMap(col, capDelta);
                        channelsMap.insert(chType, SceneValue(fxID, ch));
                    break;
                    default: break;
                }
            }
            break;
            case QLCChannel::Pan:
            case QLCChannel::Tilt:
            {
                hasPosition = true;
                if(fixture->fixtureMode() != NULL)
                {
                    QLCPhysical phy = fixture->fixtureMode()->physical();
                    int panDeg = phy.focusPanMax();
                    int tiltDeg = phy.focusTiltMax();
                    // if not set, try to give them reasonable values
                    if (panDeg == 0) panDeg = 360;
                    if (tiltDeg == 0) tiltDeg = 270;

                    if (panDeg > m_maxPanDegrees)
                        m_maxPanDegrees = panDeg;
                    if (tiltDeg > m_maxTiltDegrees)
                        m_maxTiltDegrees = tiltDeg;

                    qDebug() << "Fixture" << fixture->name() << "Pan:" << panDeg << ", Tilt:" << tiltDeg;
                }
                channelsMap.insert(chType, SceneValue(fxID, ch));
            }
            break;
            case QLCChannel::Colour:
            {
                hasColorWheel = true;
                if (enable)
                {
                    if (m_presetsCache.contains(channel) == false)
                    {
                        m_presetsCache[channel] = fxID;
                        emit colorWheelChannelsChanged();
                    }
                }
                else
                {
                    m_presetsCache.remove(channel);
                    emit colorWheelChannelsChanged();
                }
                channelsMap.insert(chType, SceneValue(fxID, ch));
            }
            break;
            case QLCChannel::Gobo:
            {
                hasGobos = true;
                if (enable)
                {
                    if (m_presetsCache.contains(channel) == false)
                    {
                        m_presetsCache[channel] = fxID;
                        emit goboChannelsChanged();
                    }
                }
                else
                {
                    m_presetsCache.remove(channel);
                    emit goboChannelsChanged();
                }
                channelsMap.insert(chType, SceneValue(fxID, ch));
            }
            break;
            default:
            break;
        }
    }

    if (origColorsMask != m_colorsMask)
        emit colorsMaskChanged(m_colorsMask);

    if (hasDimmer)
    {
        QQuickItem *capItem = qobject_cast<QQuickItem*>(m_view->rootObject()->findChild<QObject *>("capIntensity"));
        if (capItem != NULL)
            capItem->setProperty("counter", capItem->property("counter").toInt() + capDelta);
    }
    if (hasColor)
    {
        QQuickItem *capItem = qobject_cast<QQuickItem*>(m_view->rootObject()->findChild<QObject *>("capColor"));
        capItem->setProperty("counter", capItem->property("counter").toInt() + capDelta);
    }
    if (hasPosition)
    {
        QQuickItem *capItem = qobject_cast<QQuickItem*>(m_view->rootObject()->findChild<QObject *>("capPosition"));
        if (capItem != NULL)
        {
            capItem->setProperty("counter", capItem->property("counter").toInt() + capDelta);
            capItem->setProperty("panDegrees", m_maxPanDegrees);
            capItem->setProperty("tiltDegrees", m_maxTiltDegrees);
        }
    }
    if (hasColorWheel)
    {
        QQuickItem *capItem = qobject_cast<QQuickItem*>(m_view->rootObject()->findChild<QObject *>("capColorWheel"));
        if (capItem != NULL)
            capItem->setProperty("counter", capItem->property("counter").toInt() + capDelta);
    }
    if (hasGobos)
    {
        QQuickItem *capItem = qobject_cast<QQuickItem*>(m_view->rootObject()->findChild<QObject *>("capGobos"));
        if (capItem != NULL)
            capItem->setProperty("counter", capItem->property("counter").toInt() + capDelta);
    }

    return channelsMap;
}

int FixtureManager::fixturesCount()
{
    return m_doc->fixtures().count();
}

QQmlListProperty<Fixture> FixtureManager::fixtures()
{
    m_fixtureList.clear();
    m_fixtureList = m_doc->fixtures();
    return QQmlListProperty<Fixture>(this, m_fixtureList);
}

QVariant FixtureManager::groupsTreeModel()
{
    if (m_fixtureTree == NULL)
    {
        m_fixtureTree = new TreeModel(this);
        QQmlEngine::setObjectOwnership(m_fixtureTree, QQmlEngine::CppOwnership);
        QStringList treeColumns;
        treeColumns << "classRef" << "type" << "id" << "subid" << "chIdx";
        m_fixtureTree->setColumnNames(treeColumns);
        m_fixtureTree->enableSorting(false);
        updateFixtureTree(m_doc, m_fixtureTree);
    }

    return QVariant::fromValue(m_fixtureTree);
}

QVariant FixtureManager::groupsListModel()
{
    QVariantList groupsList;

    foreach(FixtureGroup *grp, m_doc->fixtureGroups())
    {
        QVariantMap grpMap;
        grpMap.insert("mIcon", "qrc:/group.svg");
        grpMap.insert("mLabel", grp->name());
        grpMap.insert("mValue", grp->id());
        groupsList.append(grpMap);
    }

    return QVariant::fromValue(groupsList);
}

void FixtureManager::addFixturesToNewGroup(QList<quint32> fxList)
{
    FixtureGroup *group = new FixtureGroup(m_doc);
    m_doc->addFixtureGroup(group);
    group->setName(tr("New group %1").arg(group->id() + 1));

    // here we should perform a "smart" grid based
    // on the 2D position of the fixtures and their heads number.
    // For now we use the "old" QLC+ mechanism of calculating an
    // equilateral grid size
    int headsCount = 0;
    foreach(quint32 id, fxList)
    {
        Fixture* fxi = m_doc->fixture(id);
        if (fxi != NULL)
            headsCount += fxi->heads();
    }
    qreal side = qSqrt(headsCount);
    if (side != qFloor(side))
        side += 1; // Fixture number doesn't provide a full square

    group->setSize(QSize(side, side));
    foreach(quint32 id, fxList)
        group->assignFixture(id);

    updateFixtureTree(m_doc, m_fixtureTree);
    emit groupsTreeModelChanged();
    emit groupsListModelChanged();
}

QList<SceneValue> FixtureManager::getFixturePosition(quint32 fxID, int type, int degrees)
{
    QList<SceneValue> posList;
    // cache a list of channels processed, to avoid duplicates
    QList<quint32> chDone;

    Fixture *fixture = m_doc->fixture(fxID);
    if (fixture == NULL || fixture->fixtureMode() == NULL)
        return posList;

    QLCPhysical phy = fixture->fixtureMode()->physical();
    float maxDegrees;
    if (type == QLCChannel::Pan)
    {
        maxDegrees = phy.focusPanMax();
        if (maxDegrees == 0) maxDegrees = 360;

        for (int i = 0; i < fixture->heads(); i++)
        {
            quint32 panMSB = fixture->channelNumber(QLCChannel::Pan, QLCChannel::MSB, i);
            if (panMSB == QLCChannel::invalid() || chDone.contains(panMSB))
                continue;

            float dmxValue = (float)(degrees * UCHAR_MAX) / maxDegrees;
            posList.append(SceneValue(fixture->id(), panMSB, static_cast<uchar>(qFloor(dmxValue))));

            qDebug() << "[getFixturePosition] Pan MSB:" << dmxValue;

            quint32 panLSB = fixture->channelNumber(QLCChannel::Pan, QLCChannel::LSB, i);

            if (panLSB != QLCChannel::invalid())
            {
                float lsbDegrees = (float)maxDegrees / (float)UCHAR_MAX;
                float lsbValue = (float)((dmxValue - qFloor(dmxValue)) * UCHAR_MAX) / lsbDegrees;
                posList.append(SceneValue(fixture->id(), panLSB, static_cast<uchar>(lsbValue)));

                qDebug() << "[getFixturePosition] Pan LSB:" << lsbValue;
            }

            chDone.append(panMSB);
        }
    }
    else if (type == QLCChannel::Tilt)
    {
        maxDegrees = phy.focusTiltMax();
        if (maxDegrees == 0) maxDegrees = 270;

        for (int i = 0; i < fixture->heads(); i++)
        {
            quint32 tiltMSB = fixture->channelNumber(QLCChannel::Tilt, QLCChannel::MSB, i);
            if (tiltMSB == QLCChannel::invalid() || chDone.contains(tiltMSB))
                continue;

            float dmxValue = (float)(degrees * UCHAR_MAX) / maxDegrees;
            posList.append(SceneValue(fixture->id(), tiltMSB, static_cast<uchar>(qFloor(dmxValue))));

            qDebug() << "[getFixturePosition] Tilt MSB:" << dmxValue;

            quint32 tiltLSB = fixture->channelNumber(QLCChannel::Tilt, QLCChannel::LSB, i);

            if (tiltLSB != QLCChannel::invalid())
            {
                float lsbDegrees = (float)maxDegrees / (float)UCHAR_MAX;
                float lsbValue = (float)((dmxValue - qFloor(dmxValue)) * UCHAR_MAX) / lsbDegrees;
                posList.append(SceneValue(fixture->id(), tiltLSB, static_cast<uchar>(lsbValue)));

                qDebug() << "[getFixturePosition] Tilt LSB:" << lsbValue;
            }

            chDone.append(tiltMSB);
        }

    }

    return posList;
}

QVariantList FixtureManager::presetsChannels(QLCChannel::Group group)
{
    QVariantList prList;
    int i = 0;

    foreach(const QLCChannel *ch, m_presetsCache.keys())
    {
        if (ch->group() != group)
        {
            i++;
            continue;
        }
        quint32 fxID = m_presetsCache[ch];
        Fixture *fixture = m_doc->fixture(fxID);
        if (fixture == NULL)
            continue;

        const QLCFixtureDef *def = fixture->fixtureDef();
        if (def != NULL)
        {
            QVariantMap prMap;
            prMap.insert("name", QString("%1 - %2")
                                .arg(def->model())
                                .arg(ch->name()));
            prMap.insert("presetIndex", i);
            prList.append(prMap);
        }
        i++;
    }

    return prList;
}

void FixtureManager::updateFixtureTree(Doc *doc, TreeModel *treeModel)
{
    if (doc == NULL || treeModel == NULL)
        return;

    treeModel->clear();

    QStringList uniNames = doc->inputOutputMap()->universeNames();

    // add Fixture Groups first
    for (FixtureGroup* grp : doc->fixtureGroups()) // C++11
    {
        foreach(quint32 fxID, grp->fixtureList())
        {
            Fixture *fixture = doc->fixture(fxID);
            if (fixture == NULL)
                continue;

            QLCFixtureMode *mode = fixture->fixtureMode();
            if (mode == NULL)
                continue;

            int chIdx = 0;
            QString chPath = QString("%1/%2").arg(grp->name()).arg(fixture->name());
            for (QLCChannel *channel : mode->channels()) // C++11
            {
                QVariantList chParams;
                chParams.append(QVariant::fromValue(NULL)); // classRef
                chParams.append("FCG"); // type
                chParams.append(fixture->id()); // id
                chParams.append(grp->id()); // subid
                chParams.append(chIdx); // chIdx
                treeModel->addItem(channel->name(), chParams, chPath);
                chIdx++;
            }

            // when all the channel 'leaves' have been added, set the parent node data
            QVariantList params;
            params.append(QVariant::fromValue(fixture)); // classRef
            params.append("FXG"); // type
            params.append(fixture->id()); // id
            params.append(grp->id()); // subid
            params.append(0); // chIdx

            treeModel->setPathData(chPath, params);
        }
    }

    // add the current universes as groups
    for (Fixture *fixture : doc->fixtures()) // C++11
    {
        if (fixture->universe() >= (quint32)uniNames.count())
            continue;

        QString chPath = QString("%1/%2").arg(uniNames.at(fixture->universe())).arg(fixture->name());
        QLCFixtureMode *mode = fixture->fixtureMode();
        if (mode == NULL)
            continue;

        int chIdx = 0;
        for (QLCChannel *channel : mode->channels()) // C++11
        {
            QVariantList chParams;
            chParams.append(QVariant::fromValue(NULL)); // classRef
            chParams.append("FCU"); // type
            chParams.append(fixture->id()); // id
            chParams.append(fixture->universe()); // subid
            chParams.append(chIdx); // chIdx
            treeModel->addItem(channel->name(), chParams, chPath);
            chIdx++;
        }

        // when all the channel 'leaves' have been added, set the parent node data
        QVariantList params;
        params.append(QVariant::fromValue(fixture)); // classRef
        params.append("FXU"); // type
        params.append(fixture->id()); // id
        params.append(fixture->universe()); // subid
        params.append(0); // chIdx

        treeModel->setPathData(chPath, params);
    }

    //treeModel->printTree(); // enable for debug purposes
}

void FixtureManager::updateColorsMap(int type, int delta)
{
    int maskVal = 0;

    switch (type)
    {
        case QLCChannel::Red:
            maskVal = App::Red;
        break;
        case QLCChannel::Green:
            maskVal = App::Green;
        break;
        case QLCChannel::Blue:
            maskVal = App::Blue;
        break;
        case QLCChannel::Cyan:
            maskVal = App::Cyan;
        break;
        case QLCChannel::Magenta:
            maskVal = App::Magenta;
        break;
        case QLCChannel::Yellow:
            maskVal = App::Yellow;
        break;
        case QLCChannel::White:
            maskVal = App::White;
        break;
        case QLCChannel::Amber:
            maskVal = App::Amber;
        break;
        case QLCChannel::UV:
            maskVal = App::UV;
        break;
        case QLCChannel::Lime:
            maskVal = App::Lime;
        break;
        case QLCChannel::Indigo:
            maskVal = App::Indigo;
        break;
        default: return;
    }

    m_colorCounters[type] += delta;

    if (m_colorCounters[type] == 0)
        m_colorsMask &= ~maskVal;
    else
        m_colorsMask |= maskVal;
}

QVariantList FixtureManager::goboChannels()
{
    return presetsChannels(QLCChannel::Gobo);
}

QVariantList FixtureManager::colorWheelChannels()
{
    return presetsChannels(QLCChannel::Colour);
}

QVariantList FixtureManager::presetCapabilities(int index)
{
    QList<const QLCChannel*>channels = m_presetsCache.keys();

    qDebug() << "[FixtureManager] Requesting presets at index:" << index << "count:" << channels.count();

    if (index < 0 || index >= channels.count())
        return QVariantList();

    const QLCChannel* ch = channels.at(index);
    qDebug() << "[FixtureManager] Channel requested:" << ch->name() << "count:" << ch->capabilities().count();

    QVariantList var;
    foreach(QLCCapability *cap, ch->capabilities())
        var.append(QVariant::fromValue(cap));

    return var;
}

QVariantList FixtureManager::fixtureNamesMap()
{
    quint32 uniFilter = m_universeFilter == Universe::invalid() ? 0 : m_universeFilter;

    m_fixtureNamesMap.clear();

    for(Fixture *fx : m_doc->fixtures()) // C++11
    {
        if (fx == NULL)
            continue;

        if (fx->universe() != uniFilter)
            continue;

        m_fixtureNamesMap.append(fx->id());
        m_fixtureNamesMap.append(fx->universeAddress());
        m_fixtureNamesMap.append(fx->channels());
        m_fixtureNamesMap.append(fx->name());
    }

    return m_fixtureNamesMap;
}

QVariantList FixtureManager::fixturesMap()
{
    bool odd = true;
    quint32 uniFilter = m_universeFilter == Universe::invalid() ? 0 : m_universeFilter;

    /* There would be two ways to transfer organized data to QML.
     * The first is a QVariantList of QVariantMaps, to have named variables.
     * The second is a plain QVariantList of numbers, organized as contiguous
     * groups with a specific (implicit) meaning.
     * In this case since we have to transfer a lot of data and the rendering
     * is already heavy, I prefer the second option, organized as follows:
     *
     * Fixture ID | DMX address | isOdd | channel type (a lookup for icons)
     */

    m_fixturesMap.clear();

    for(Fixture *fx : m_doc->fixtures()) // C++11
    {
        if (fx == NULL)
            continue;

        if (fx->universe() != uniFilter)
            continue;

        quint32 startAddress = fx->address();
        for(quint32 cn = 0; cn < fx->channels(); cn++)
        {
            m_fixturesMap.append(fx->id());
            m_fixturesMap.append(startAddress + cn);

            if (odd)
                m_fixturesMap.append(1);
            else
                m_fixturesMap.append(0);

            QLCChannel::Group group = fx->channel(cn)->group();
            if (group == QLCChannel::Intensity)
                m_fixturesMap.append(fx->channel(cn)->colour());
            else
                m_fixturesMap.append(group - 1);
        }
        odd = !odd;

    }
    return m_fixturesMap;
}

quint32 FixtureManager::universeFilter() const
{
    return m_universeFilter;
}

void FixtureManager::setUniverseFilter(quint32 universeFilter)
{
    if (m_universeFilter == universeFilter)
        return;

    m_universeFilter = universeFilter;
    emit universeFilterChanged(universeFilter);
    emit fixtureNamesMapChanged();
    emit fixturesMapChanged();
}

int FixtureManager::colorsMask() const
{
    return m_colorsMask;
}

void FixtureManager::slotDocLoaded()
{
    QQuickItem *capItem = qobject_cast<QQuickItem*>(m_view->rootObject()->findChild<QObject *>("capIntensity"));
    if (capItem)
        capItem->setProperty("counter", 0);

    capItem = qobject_cast<QQuickItem*>(m_view->rootObject()->findChild<QObject *>("capColor"));
    if (capItem)
        capItem->setProperty("counter", 0);

    capItem = qobject_cast<QQuickItem*>(m_view->rootObject()->findChild<QObject *>("capPosition"));
    if (capItem)
        capItem->setProperty("counter", 0);

    capItem = qobject_cast<QQuickItem*>(m_view->rootObject()->findChild<QObject *>("capColorWheel"));
    if (capItem)
        capItem->setProperty("counter", 0);

    capItem = qobject_cast<QQuickItem*>(m_view->rootObject()->findChild<QObject *>("capGobos"));
    if (capItem)
        capItem->setProperty("counter", 0);

    m_colorCounters.clear();
    m_fixtureList.clear();
    m_fixtureList = m_doc->fixtures();
    emit fixturesCountChanged();

    updateFixtureTree(m_doc, m_fixtureTree);
    emit groupsTreeModelChanged();
    emit groupsListModelChanged();
}
