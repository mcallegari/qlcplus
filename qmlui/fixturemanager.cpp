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

#include <QQmlContext>
#include <QQuickItem>
#include <QQmlEngine>
#include <QVariant>
#include <QDebug>
#include <QtMath>
#include <QDir>

#include "monitorproperties.h"
#include "channelmodifier.h"
#include "fixturemanager.h"
#include "qlcfixturemode.h"
#include "qlccapability.h"
#include "qlcfixturedef.h"
#include "treemodelitem.h"
#include "fixtureutils.h"
#include "treemodel.h"
#include "qlcconfig.h"
#include "qlcfile.h"
#include "fixture.h"
#include "tardis.h"
#include "doc.h"
#include "app.h"

FixtureManager::FixtureManager(QQuickView *view, Doc *doc, QObject *parent)
    : QObject(parent)
    , m_view(view)
    , m_doc(doc)
    , m_universeFilter(Universe::invalid())
    , m_searchFilter(QString())
    , m_propertyEditEnabled(false)
    , m_fixtureTree(nullptr)
    , m_treeShowFlags(ShowGroups | ShowLinked | ShowHeads)
    , m_colorFiltersFileIndex(0)
    , m_maxPanDegrees(0)
    , m_maxTiltDegrees(0)
    , m_minBeamDegrees(15.0)
    , m_maxBeamDegrees(0)
    , m_invertedZoom(false)
    , m_colorsMask(0)
    , m_capabilityMask(0)
    , m_selectedChannelModifier(nullptr)
{
    Q_ASSERT(m_doc != nullptr);

    m_monProps = m_doc->monitorProperties();

    m_view->rootContext()->setContextProperty("fixtureManager", this);
    qmlRegisterUncreatableType<FixtureManager>("org.qlcplus.classes", 1, 0,  "FixtureManager", "Can't create a FixtureManager!");
    qmlRegisterUncreatableType<QLCCapability>("org.qlcplus.classes", 1, 0, "QLCCapability", "Can't create a QLCCapability!");
    qmlRegisterUncreatableType<ColorFilters>("org.qlcplus.classes", 1, 0, "ColorFilters", "Can't create a ColorFilters!");

    connect(m_doc, SIGNAL(loaded()), this, SLOT(slotDocLoaded()));
    connect(m_doc, SIGNAL(fixtureAdded(quint32)), this, SLOT(slotFixtureAdded(quint32)));
    connect(m_doc, SIGNAL(fixtureGroupAdded(quint32)), this, SLOT(slotFixtureGroupAdded(quint32)));
}

FixtureManager::~FixtureManager()
{
    m_view->rootContext()->setContextProperty("fixtureManager", nullptr);
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
    emit fixturesMapChanged();
}

QString FixtureManager::searchFilter() const
{
    return m_searchFilter;
}

void FixtureManager::setSearchFilter(QString searchFilter)
{
    if (m_searchFilter == searchFilter)
        return;

    int currLen = m_searchFilter.length();

    m_searchFilter = searchFilter;

    if (searchFilter.length() >= SEARCH_MIN_CHARS ||
        (currLen >= SEARCH_MIN_CHARS && searchFilter.length() < SEARCH_MIN_CHARS))
    {
        updateGroupsTree(m_doc, m_fixtureTree, m_searchFilter, m_treeShowFlags);
        emit groupsTreeModelChanged();
    }

    emit searchFilterChanged();
}

quint32 FixtureManager::itemID() const
{
    return m_itemID;
}

void FixtureManager::setItemID(quint32 itemID)
{
    if (m_itemID == itemID)
        return;

    m_itemID = itemID;
    emit itemIDChanged(m_itemID);
}

QVariantList FixtureManager::universeInfo(quint32 id)
{
    m_universeInfo.clear();

    for (quint32 fixtureID : m_monProps->fixtureItemsID())
    {
        for (quint32 subID : m_monProps->fixtureIDList(fixtureID))
        {
            quint16 headIndex = m_monProps->fixtureHeadIndex(subID);
            quint16 linkedIndex = m_monProps->fixtureLinkedIndex(subID);
            quint32 flags = m_monProps->fixtureFlags(fixtureID, headIndex, linkedIndex);
            if (flags & MonitorProperties::HiddenFlag)
                continue;

            Fixture *fixture = m_doc->fixture(fixtureID);
            if (fixture == nullptr || fixture->universe() != id)
                continue;

            QVariantMap fxMap;
            fxMap.insert("classRef", QVariant::fromValue(fixture));
            fxMap.insert("manuf", fixture->fixtureDef() ? fixture->fixtureDef()->manufacturer() : "");
            fxMap.insert("fmodel", fixture->fixtureDef() ? fixture->fixtureDef()->model() : "");
            fxMap.insert("weight", fixture->fixtureMode() ? fixture->fixtureMode()->physical().weight() : 0);
            fxMap.insert("power", fixture->fixtureMode() ? fixture->fixtureMode()->physical().powerConsumption() : 0);
            m_universeInfo.append(fxMap);
        }
    }

    return m_universeInfo;
}

QVariant FixtureManager::fixtureInfo(quint32 itemID)
{
    QVariantMap fxMap;

    quint32 fixtureID = FixtureUtils::itemFixtureID(itemID);
    Fixture *fixture = m_doc->fixture(fixtureID);

    if (fixture == nullptr)
        return fxMap;

    QLCFixtureDef *def = fixture->fixtureDef();
    QLCFixtureMode *mode = fixture->fixtureMode();

    if (def == nullptr || mode == nullptr)
        return fxMap;

    QLCPhysical phy = mode->physical();

    fxMap.insert("classRef", QVariant::fromValue(fixture));
    fxMap.insert("manuf", def->manufacturer());
    fxMap.insert("fmodel", def->model());
    fxMap.insert("mode", mode->name());
    fxMap.insert("author", def->author());

    QVariantList channelList;
    for (QLCChannel *channel : mode->channels())
    {
        QVariantMap chMap;
        chMap.insert("mIcon", channel->getIconNameFromGroup(channel->group(), true));
        chMap.insert("mLabel", channel->name());
        channelList.append(chMap);
    }
    fxMap.insert("channels", QVariant::fromValue(channelList));

    fxMap.insert("width", phy.width());
    fxMap.insert("height", phy.height());
    fxMap.insert("depth", phy.depth());
    fxMap.insert("weight", phy.weight());
    fxMap.insert("power", phy.powerConsumption());
    fxMap.insert("connector", phy.dmxConnector());

    fxMap.insert("bulbType", phy.bulbType());
    fxMap.insert("bulbLumens", phy.bulbLumens());
    fxMap.insert("bulbTemp", phy.bulbColourTemperature());

    fxMap.insert("lensType", phy.lensName());
    fxMap.insert("beamMin", phy.lensDegreesMin());
    fxMap.insert("beamMax", phy.lensDegreesMax());

    fxMap.insert("headType", phy.focusType());
    fxMap.insert("panDegrees", phy.focusPanMax());
    fxMap.insert("tiltDegrees", phy.focusTiltMax());

    return QVariant::fromValue(fxMap);
}

void FixtureManager::slotDocLoaded()
{
    setCapabilityCounter("capIntensity", 0);
    setCapabilityCounter("capColor", 0);
    setCapabilityCounter("capPosition", 0);
    setCapabilityCounter("capColorWheel", 0);
    setCapabilityCounter("capGobos", 0);
    setCapabilityCounter("capShutter", 0);
    setCapabilityCounter("capBeam", 0);

    m_colorCounters.clear();
    m_fixtureList.clear();
    m_fixtureList = m_doc->fixtures();

    emit fixturesCountChanged();
    emit fixturesMapChanged();

    setSearchFilter("");
    updateGroupsTree(m_doc, m_fixtureTree, m_searchFilter);
    emit groupsTreeModelChanged();
}

/*********************************************************************
 * Fixtures
 *********************************************************************/

quint32 FixtureManager::invalidFixture() const
{
    return Fixture::invalidId();
}

quint32 FixtureManager::fixtureForAddress(quint32 universeAddress)
{
    return m_doc->fixtureForAddress(universeAddress);
}

bool FixtureManager::addFixture(QString manuf, QString model, QString mode, QString name,
                                int uniIdx, int address, int channels, int quantity, quint32 gap,
                                qreal xPos, qreal yPos)
{
    qDebug() << "[addFixture]" << manuf << model << name << address << channels << quantity << gap;

    if (model == "Generic RGB Panel")
        return addRGBPanel(name, xPos, yPos);

    QLCFixtureDef *fxiDef = m_doc->fixtureDefCache()->fixtureDef(manuf, model);
    QLCFixtureMode *fxiMode = fxiDef != nullptr ? fxiDef->mode(mode) : nullptr;

    // temporarily disconnect this signal since we want to use the given position
    disconnect(m_doc, SIGNAL(fixtureAdded(quint32)), this, SLOT(slotFixtureAdded(quint32)));
    quint32 fxAddress = address;

    for (int i = 0; i < quantity; i++)
    {
        Fixture *fxi = new Fixture(m_doc);
        //quint32 fxAddress = address + (i * channels) + (i * gap);
        if (fxAddress + channels > UNIVERSE_SIZE)
        {
            uniIdx++;
            if (m_doc->inputOutputMap()->getUniverseID(uniIdx) == m_doc->inputOutputMap()->invalidUniverse())
            {
                m_doc->inputOutputMap()->addUniverse();
                m_doc->inputOutputMap()->startUniverses();
            }
            fxAddress = 0;
        }

        fxi->setAddress(fxAddress);
        fxi->setUniverse(uniIdx);
        if (fxiDef == nullptr && fxiMode == nullptr)
        {
            if (model == "Generic Dimmer")
            {
                fxiDef = fxi->genericDimmerDef(channels);
                fxiMode = fxi->genericDimmerMode(fxiDef, channels);
            }
            else
            {
                qWarning() << "FIXME: Something really bad happened";
                return false;
            }
        }

        fxi->setFixtureDefinition(fxiDef, fxiMode);

        if (m_doc->addFixture(fxi) == true)
        {
            fxi->setName(QString("%1 [%2]").arg(name).arg(fxi->id() + 1));
            Tardis::instance()->enqueueAction(Tardis::FixtureCreate, fxi->id(), QVariant(),
                                              Tardis::instance()->actionToByteArray(Tardis::FixtureCreate, fxi->id()));
            slotFixtureAdded(fxi->id(), QVector3D(xPos, yPos, 0));
        }

        fxAddress += (channels + gap);
    }

    connect(m_doc, SIGNAL(fixtureAdded(quint32)), this, SLOT(slotFixtureAdded(quint32)));

    emit fixturesCountChanged();
    emit fixturesMapChanged();

    return true;
}

bool FixtureManager::moveFixture(quint32 fixtureID, quint32 newAddress)
{
    qDebug() << "[FixtureManager] requested to move fixture with ID" << fixtureID << "to address" << newAddress;
    Fixture *fixture = m_doc->fixture(fixtureID);
    if (fixture == nullptr)
        return false;

    Tardis::instance()->enqueueAction(Tardis::FixtureMove, fixtureID, fixture->address(), newAddress);

    fixture->setAddress(newAddress);

    updateGroupsTree(m_doc, m_fixtureTree, m_searchFilter);
    emit groupsTreeModelChanged();
    emit fixturesMapChanged();
    return true;
}

bool FixtureManager::deleteFixtures(QVariantList IDList)
{
    for (QVariant id : IDList)
    {
        quint32 itemID = id.toUInt();
        quint32 fxID = FixtureUtils::itemFixtureID(itemID);
        quint16 headIndex = FixtureUtils::itemHeadIndex(itemID);
        quint16 linkedIndex = FixtureUtils::itemLinkedIndex(itemID);

        Tardis::instance()->enqueueAction(Tardis::FixtureSetPosition, itemID,
                                          QVariant(m_monProps->fixturePosition(fxID, headIndex, linkedIndex)), QVariant());
        m_monProps->removeFixture(fxID, headIndex, linkedIndex);
        Tardis::instance()->enqueueAction(Tardis::FixtureDelete, itemID,
                                          Tardis::instance()->actionToByteArray(Tardis::FixtureDelete, fxID),
                                          QVariant());
        m_doc->deleteFixture(fxID);
        emit fixtureDeleted(itemID);
    }

    emit fixturesCountChanged();

    updateGroupsTree(m_doc, m_fixtureTree, m_searchFilter);
    emit groupsTreeModelChanged();
    emit fixturesMapChanged();

    return true;
}

bool FixtureManager::deleteFixtureInGroup(quint32 groupID, quint32 itemID, QString path)
{
    FixtureGroup *group = m_doc->fixtureGroup(groupID);
    if (group == nullptr)
        return false;

    quint32 fxID = FixtureUtils::itemFixtureID(itemID);
    //quint16 headIndex = FixtureUtils::itemHeadIndex(itemID);
    //quint16 linkedIndex = FixtureUtils::itemLinkedIndex(itemID);

    //TODO: tardis

    qDebug() << "Removing fixture" << fxID << "from group" << group->name();
    group->resignFixture(fxID);

    m_fixtureTree->removeItem(path);
    emit groupsTreeModelChanged();

    return true;
}

bool FixtureManager::renameFixture(quint32 itemID, QString newName)
{
    quint32 fixtureID = FixtureUtils::itemFixtureID(itemID);
    //quint16 headIndex = FixtureUtils::itemHeadIndex(itemID);
    //quint16 linkedIndex = FixtureUtils::itemLinkedIndex(itemID);

    Fixture *fixture = m_doc->fixture(fixtureID);
    if (fixture == nullptr)
        return false;

    QList<Fixture*> fixtureList = m_doc->fixtures();
    for (Fixture *docFixture : fixtureList)
    {
        if (docFixture->name() == newName)
            return false;
    }

    Tardis::instance()->enqueueAction(Tardis::FixtureSetName, itemID, fixture->name(), newName);

    setItemRoleData(itemID, -1, "label", newName);
    fixture->setName(newName);

    return true;
}

int FixtureManager::fixturesCount()
{
    return m_doc->fixturesCount();
}

QVariant FixtureManager::groupsTreeModel()
{
    if (m_fixtureTree == nullptr)
    {
        m_fixtureTree = new TreeModel(this);
        QQmlEngine::setObjectOwnership(m_fixtureTree, QQmlEngine::CppOwnership);
        setPropertyEditEnabled(false);
    }

    return QVariant::fromValue(m_fixtureTree);
}

bool FixtureManager::propertyEditEnabled()
{
    return m_propertyEditEnabled;
}

void FixtureManager::setPropertyEditEnabled(bool enable)
{
    m_propertyEditEnabled = enable;

    QStringList treeColumns;
    treeColumns << "classRef" << "type" << "id" << "subid" << "chIdx" << "inGroup";

    if (enable)
    {
        m_treeShowFlags = ShowChannels | ShowLinked | ShowFlags | ShowCanFade | ShowPrecedence | ShowModifier;
        treeColumns << "flags" << "canFade" << "precedence" << "modifier";
    }
    else
    {
        m_treeShowFlags = ShowGroups | ShowLinked | ShowHeads;
    }

    emit propertyEditEnabledChanged();

    m_fixtureTree->setColumnNames(treeColumns);
    m_fixtureTree->enableSorting(false);
    updateGroupsTree(m_doc, m_fixtureTree, m_searchFilter, m_treeShowFlags);
    emit groupsTreeModelChanged();
}

void FixtureManager::setItemRoleData(int itemID, int index, QString role, QVariant value)
{
    quint32 fixtureID = FixtureUtils::itemFixtureID(itemID);
    quint16 headIndex = FixtureUtils::itemHeadIndex(itemID);
    quint16 linkedIndex = FixtureUtils::itemLinkedIndex(itemID);

    Fixture *fixture = m_doc->fixture(fixtureID);
    if (fixture == nullptr)
        return;

    const QLCChannel *channel = index == -1 ? nullptr : fixture->channel(index);
    if (index >= 0 && channel == nullptr)
        return;

    qDebug() << "Set fixture data" << fixture->name() << role << value;

    if (role == "flags")
    {
        if (index == -1)
        {
            m_monProps->setFixtureFlags(fixtureID, headIndex, linkedIndex, value.toUInt());
            emit fixtureFlagsChanged(itemID, value.toUInt());
        }
        else
        {
            // TODO: flags per channel ?
        }
    }
    else if (role == "canFade")
    {
        fixture->setChannelCanFade(index, value.toBool());
    }
    else if (role == "precedence")
    {
        QList<int> forcedHTP = fixture->forcedHTPChannels();
        QList<int> forcedLTP = fixture->forcedLTPChannels();

        int newMode = value.toInt();
        switch (newMode)
        {
            case AutoHTP:
            case AutoLTP:
                forcedHTP.removeOne(index);
                forcedLTP.removeOne(index);
            break;
            case ForcedHTP:
                if (channel->group() != QLCChannel::Intensity)
                    forcedHTP.append(index);
            break;
            case ForcedLTP:
                if (channel->group() == QLCChannel::Intensity)
                    forcedLTP.append(index);
            break;
        }

        fixture->setForcedHTPChannels(forcedHTP);
        fixture->setForcedLTPChannels(forcedLTP);
    }

    // now reconstruct the item path and change the role of the tree model
    QString path;
    QString fxName = fixture->name();
    QStringList uniNames = m_doc->inputOutputMap()->universeNames();
    int roleIndex = m_fixtureTree->roleIndex(role);
    if (linkedIndex)
        fxName = m_monProps->fixtureName(fixtureID, headIndex, linkedIndex);

    if (index == -1)
    {
        // change happened on a fixture node
        path = QString("%1%2%3").arg(uniNames.at(fixture->universe()))
                                .arg(TreeModel::separator()).arg(fxName);
    }
    else
    {
        // change happened on a channel node
        path = QString("%1%2%3%2%4").arg(uniNames.at(fixture->universe()))
                                    .arg(TreeModel::separator()).arg(fxName)
                                    .arg(channel->name());
    }

    //qDebug() << "Path" << path << ", role index" << roleIndex;

    m_fixtureTree->setItemRoleData(path, value, roleIndex);
}

void FixtureManager::setItemRoleData(int itemID, QVariant value, int role)
{
    if (m_fixtureTree == nullptr)
        return;

    quint32 fixtureID = FixtureUtils::itemFixtureID(itemID);

    Fixture *fixture = m_doc->fixture(fixtureID);
    if (fixture == nullptr)
        return;

    QString fxName = fixture->name();
    QStringList uniNames = m_doc->inputOutputMap()->universeNames();

    QString path = QString("%1%2%3").arg(uniNames.at(fixture->universe()))
               .arg(TreeModel::separator()).arg(fxName);

    m_fixtureTree->setItemRoleData(path, value, role);
}

bool FixtureManager::compareFixtures(Fixture *left, Fixture *right)
{
    return *left < *right;
}

void FixtureManager::addFixtureNode(Doc *doc, TreeModel *treeModel, Fixture *fixture,
                                    QString basePath, quint32 nodeSubID,
                                    int &matchMask, QString searchFilter,
                                    int showFlags, QList<SceneValue> checkedChannels)
{
    if (doc == nullptr || treeModel == nullptr || fixture == nullptr)
        return;

    MonitorProperties *monProps = doc->monitorProperties();
    bool expandAll = searchFilter.length() >= SEARCH_MIN_CHARS;

    if (searchFilter.length() < SEARCH_MIN_CHARS || fixture->name().toLower().contains(searchFilter))
        matchMask |= FixtureMatch;

    for (quint32 subID : monProps->fixtureIDList(fixture->id()))
    {
        quint16 headIndex = monProps->fixtureHeadIndex(subID);
        quint16 linkedIndex = monProps->fixtureLinkedIndex(subID);
        quint32 itemID = FixtureUtils::fixtureItemID(fixture->id(), headIndex, linkedIndex);
        int flags = monProps->fixtureFlags(fixture->id(), headIndex, linkedIndex);

        // do not show hidden fixtures if not editing
        if (!(showFlags & ShowFlags) && (flags & MonitorProperties::HiddenFlag))
            continue;

        // represent dimmers as a whole fixture + (channels || heads)
        if (fixture->type() == QLCFixtureDef::Dimmer && headIndex > 0)
            continue;

        // do not show linked fixtures if not requested
        if (linkedIndex && !(showFlags & ShowLinked))
            continue;

        // do not display channels for linked fixtures
        if (linkedIndex)
            showFlags &= ~ShowChannels;

        QString fxName = fixture->name();
        if (linkedIndex)
            fxName = monProps->fixtureName(fixture->id(), headIndex, linkedIndex);

        QString fxPath = QString("%1%2%3").arg(basePath).arg(TreeModel::separator()).arg(fxName);

        if (showFlags & ShowHeads)
        {
            if (matchMask && fixture->heads() > 1)
            {
                for (int headIdx = 0; headIdx < fixture->heads(); headIdx++)
                {
                    quint32 iID = FixtureUtils::fixtureItemID(fixture->id(), headIdx, linkedIndex);

                    QVariantList headParams;
                    headParams.append(QVariant::fromValue(fixture)); // classRef
                    headParams.append(App::HeadDragItem); // type
                    headParams.append(iID); // id
                    headParams.append(nodeSubID); // subid
                    headParams.append(headIdx); // chIdx
                    headParams.append(false); // inGroup
                    treeModel->addItem(QString("%1 %2").arg(tr("Head")).arg(headIdx + 1, 3, 10, QChar('0')),
                                       headParams, fxPath);
                }
            }
        }
        else if (showFlags & ShowChannels)
        {
            int chIdx = 0;
            QList<int> forcedHTP = fixture->forcedHTPChannels();
            QList<int> forcedLTP = fixture->forcedLTPChannels();

            QLCFixtureMode *mode = fixture->fixtureMode();
            if (mode == nullptr)
                continue;

            for (QLCChannel *channel : mode->channels()) // C++11
            {
                if (matchMask || searchFilter.length() < SEARCH_MIN_CHARS ||
                    channel->name().toLower().contains(searchFilter))
                {
                    int flags = expandAll ? TreeModel::Expanded : 0;
                    if (checkedChannels.contains(SceneValue(fixture->id(), chIdx)))
                        flags |= TreeModel::Checked;

                    QVariantList chParams;
                    chParams.append(QVariant::fromValue(fixture)); // classRef
                    chParams.append(App::ChannelDragItem); // type
                    chParams.append(itemID); // id
                    chParams.append(nodeSubID); // subid
                    chParams.append(chIdx); // chIdx
                    chParams.append(false); // inGroup

                    if (showFlags & ShowFlags)
                        chParams.append(0); // might be useful in the future

                    if (showFlags & ShowCanFade)
                        chParams.append(fixture->channelCanFade(chIdx)); // canFade

                    if (showFlags & ShowPrecedence)
                    {
                        if (forcedHTP.contains(chIdx))
                            chParams.append(ForcedHTP);
                        else if (forcedLTP.contains(chIdx))
                            chParams.append(ForcedLTP);
                        else
                        {
                            if (channel->group() == QLCChannel::Intensity)
                                chParams.append(AutoHTP);
                            else
                                chParams.append(AutoLTP);
                        }
                    }

                    if (showFlags & ShowModifier)
                    {
                        ChannelModifier *cm = fixture->channelModifier(chIdx);
                        chParams.append(cm != nullptr ? cm->name() : "");
                    }

                    treeModel->addItem(channel->name(), chParams, fxPath, flags);
                    matchMask |= ChannelMatch;
                }
                chIdx++;
            }
        }

        if (matchMask)
        {
            // when all the channel/head 'leaves' have been added, set the parent node data
            QVariantList fxParams;
            fxParams.append(QVariant::fromValue(fixture)); // classRef
            fxParams.append(App::FixtureDragItem); // type
            fxParams.append(itemID); // id
            fxParams.append(nodeSubID); // subid
            fxParams.append(0); // chIdx
            fxParams.append(false); // inGroup

            if (showFlags & ShowFlags)
                fxParams.append(monProps->fixtureFlags(fixture->id(), headIndex, linkedIndex));

            if (showFlags & ShowChannels || fixture->heads() > 1)
                treeModel->setPathData(fxPath, fxParams);
            else
                treeModel->addItem(fxName, fxParams, basePath, expandAll ? TreeModel::Expanded : 0);
        }
    }
}

void FixtureManager::addFixtureGroupTreeNode(Doc *doc, TreeModel *treeModel, FixtureGroup *group,
                                             QString searchFilter, int showFlags,
                                             QList<SceneValue> checkedChannels)
{
    int matchMask = 0;
    QList<quint32> fixtureIDList;

    if (doc == nullptr || treeModel == nullptr || group == nullptr)
        return;

    if (searchFilter.length() < SEARCH_MIN_CHARS || group->name().toLower().contains(searchFilter))
        matchMask |= GroupMatch;

    if (showFlags & ShowChannels)
    {
        for (quint32 fxID : group->fixtureList())
        {
            Fixture *fixture = doc->fixture(fxID);
            if (fixture == nullptr)
                continue;

            int fxMatchMask = 0;

            addFixtureNode(doc, treeModel, fixture, group->name(), group->id(),
                           fxMatchMask, searchFilter, showFlags, checkedChannels);
            if (fxMatchMask)
                matchMask |= FixtureMatch;
        }
    }
    else if (showFlags & ShowHeads)
    {
        for (GroupHead head : group->headList())
        {
            Fixture *fixture = doc->fixture(head.fxi);
            if (fixture == nullptr)
                continue;

            if (searchFilter.length() >= SEARCH_MIN_CHARS && fixture->name().toLower().contains(searchFilter) == false)
                continue;

            QString fxPath = QString("%1%2%3").arg(group->name()).arg(TreeModel::separator()).arg(fixture->name());
            quint32 itemID = FixtureUtils::fixtureItemID(fixture->id(), head.head, 0);

            QVariantList headParams;
            headParams.append(QVariant::fromValue(fixture)); // classRef
            headParams.append(App::HeadDragItem); // type
            headParams.append(itemID); // id
            headParams.append(group->id()); // subid
            headParams.append(head.head); // chIdx
            headParams.append(true); // inGroup
            treeModel->addItem(QString("%1 %2").arg(tr("Head")).arg(head.head + 1, 3, 10, QChar('0')),
                               headParams, fxPath);

            if (fixtureIDList.contains(head.fxi) == false)
            {
                QVariantList fxParams;
                fxParams.append(QVariant::fromValue(fixture)); // classRef
                fxParams.append(App::FixtureDragItem); // type
                fxParams.append(itemID); // id
                fxParams.append(group->id()); // subid
                fxParams.append(0); // chIdx
                fxParams.append(true); // inGroup

                treeModel->setPathData(fxPath, fxParams);
                fixtureIDList.append(head.fxi);
            }
        }
    }

    if (matchMask)
    {
        // add also the fixture group data
        QVariantList grpParams;
        grpParams.append(QVariant::fromValue(group)); // classRef
        grpParams.append(App::FixtureGroupDragItem); // type
        grpParams.append(group->id()); // id
        grpParams.append(0); // subid
        grpParams.append(0); // chIdx
        grpParams.append(true); // inGroup

        treeModel->setPathData(group->name(), grpParams);
    }
}

void FixtureManager::updateGroupsTree(Doc *doc, TreeModel *treeModel, QString searchFilter,
                                      int showFlags, QList<SceneValue> checkedChannels)
{
    if (doc == nullptr || treeModel == nullptr)
        return;

    QStringList uniNames = doc->inputOutputMap()->universeNames();
    QList<Fixture*> fixtureList = doc->fixtures();

    treeModel->clear();

    if (showFlags & ShowCheckBoxes)
        treeModel->setCheckable(true);
    else
        treeModel->setCheckable(false);

    if (showFlags & ShowGroups)
    {
        // add Fixture Groups first
        for (FixtureGroup *grp : doc->fixtureGroups()) // C++11
            addFixtureGroupTreeNode(doc, treeModel, grp, searchFilter, showFlags, checkedChannels);
    }

    // sort the fixture list by address and not by ID
    std::sort(fixtureList.begin(), fixtureList.end(), compareFixtures);

    // add the current universes as groups
    for (Fixture *fixture : fixtureList) // C++11
    {
        if (fixture->universe() >= (quint32)uniNames.count())
            continue;

        QString universeName = uniNames.at(fixture->universe());
        int matchMask = 0;

        addFixtureNode(doc, treeModel, fixture, universeName, fixture->universe(),
                       matchMask, searchFilter, showFlags, checkedChannels);
    }

    for (Universe *universe : doc->inputOutputMap()->universes())
    {
        // add also the Universe node data
        QVariantList uniParams;
        uniParams.append(QVariant::fromValue(universe)); // classRef
        uniParams.append(App::UniverseDragItem); // type
        uniParams.append(universe->id()); // id
        uniParams.append(0); // subid
        uniParams.append(0); // chIdx
        uniParams.append(false); // inGroup

        treeModel->setPathData(universe->name(), uniParams);
    }

    //treeModel->printTree(); // enable for debug purposes
}

QString FixtureManager::fixtureIcon(quint32 fixtureID)
{
    Fixture *fixture = m_doc->fixture(fixtureID);
    if (fixture == nullptr)
        return QString();

    return fixture->iconResource(true);
}

QStringList FixtureManager::fixtureModes(quint32 itemID)
{
    QStringList modes;
    quint32 fixtureID = FixtureUtils::itemFixtureID(itemID);
    Fixture *fixture = m_doc->fixture(fixtureID);
    if (fixture == nullptr)
        return modes;

    for (QLCFixtureMode *mode : fixture->fixtureDef()->modes())
        modes.append(mode->name());

    return modes;
}

int FixtureManager::fixtureModeIndex(quint32 itemID)
{
    quint32 fixtureID = FixtureUtils::itemFixtureID(itemID);
    Fixture *fixture = m_doc->fixture(fixtureID);
    if (fixture == nullptr)
        return -1;

    QLCFixtureMode *currMode = fixture->fixtureMode();
    QList<QLCFixtureMode *> modes = fixture->fixtureDef()->modes();

    return modes.indexOf(currMode);
}

bool FixtureManager::setFixtureModeIndex(quint32 itemID, int index)
{
    quint32 fixtureID = FixtureUtils::itemFixtureID(itemID);
    Fixture *fixture = m_doc->fixture(fixtureID);
    if (fixture == nullptr)
        return false;

    QList<QLCFixtureMode *> modes = fixture->fixtureDef()->modes();
    if (index < 0 || index >= modes.count())
        return false;

    QLCFixtureMode *newMode = modes.at(index);

    // check if new channels are available
    int chNum = newMode->channels().count();

    for (quint32 i = fixture->universeAddress(); i < fixture->universeAddress() + chNum; i++)
    {
        quint32 id = m_doc->fixtureForAddress(i);
        if (id != fixture->id() && id != Fixture::invalidId())
            return false;
    }

    fixture->setFixtureDefinition(fixture->fixtureDef(), newMode);

    emit fixturesMapChanged();

    return true;
}

int FixtureManager::fixtureIDfromItemID(quint32 itemID)
{
    return FixtureUtils::itemFixtureID(itemID);
}

int FixtureManager::fixtureLinkedIndex(quint32 itemID)
{
    return FixtureUtils::itemLinkedIndex(itemID);
}

void FixtureManager::updateLinkedFixtureNode(quint32 itemID, bool add)
{
    quint32 fixtureID = FixtureUtils::itemFixtureID(itemID);
    quint16 headIndex = FixtureUtils::itemHeadIndex(itemID);
    quint16 linkedIndex = FixtureUtils::itemLinkedIndex(itemID);
    MonitorProperties *monProps = m_doc->monitorProperties();
    QStringList uniNames = m_doc->inputOutputMap()->universeNames();

    Fixture *fixture = m_doc->fixture(fixtureID);
    if (fixture == nullptr)
        return;

    if (fixture->universe() >= (quint32)uniNames.count())
        return;

    QString universeName = uniNames.at(fixture->universe());
    QString fixtureName = monProps->fixtureName(fixtureID, headIndex, linkedIndex);

    if (add)
    {
        QVariantList fxParams;
        fxParams.append(QVariant::fromValue(fixture)); // classRef
        fxParams.append(App::FixtureDragItem); // type
        fxParams.append(itemID); // id
        fxParams.append(fixture->universe()); // subid
        fxParams.append(0); // chIdx
        fxParams.append(false); // inGroup
        fxParams.append(monProps->fixtureFlags(fixture->id(), headIndex, linkedIndex));

        m_fixtureTree->addItem(fixtureName, fxParams, universeName, 0);
    }
    else
    {
        QString path = QString("%1%2%3").arg(universeName).arg(TreeModel::separator()).arg(fixtureName);
        m_fixtureTree->removeItem(path);
    }
}

QString FixtureManager::channelIcon(quint32 fxID, quint32 chIdx)
{
    //qDebug() << "Channel icon for fixture" << fxID << "channel" << chIdx;
    Fixture *fixture = m_doc->fixture(fxID);
    if (fixture == nullptr)
        return QString();

    const QLCChannel *channel = fixture->channel(chIdx);
    if (channel == nullptr)
        return QString();

    return channel->getIconNameFromGroup(channel->group(), true);
}

QString FixtureManager::channelName(quint32 fxID, quint32 chIdx)
{
    Fixture *fixture = m_doc->fixture(fxID);
    if (fixture == nullptr)
        return QString();

    const QLCChannel *channel = fixture->channel(chIdx);
    if (channel == nullptr)
        return QString();

    return channel->name();
}

int FixtureManager::channelType(quint32 fxID, quint32 chIdx)
{
    Fixture *fixture = m_doc->fixture(fxID);
    if (fixture == nullptr)
        return QLCChannel::NoGroup;

    const QLCChannel *channel = fixture->channel(chIdx);
    if (channel == nullptr)
        return QLCChannel::NoGroup;

    if (channel->group() == QLCChannel::Intensity)
        return channel->colour();
    else
        return channel->group();
}

qreal FixtureManager::channelDegrees(quint32 fxID, quint32 chIdx)
{
    Fixture *fixture = m_doc->fixture(fxID);
    if (fixture == nullptr)
        return 0;

    const QLCChannel *channel = fixture->channel(chIdx);
    if (channel == nullptr)
        return 0;

    if (channel->group() != QLCChannel::Pan &&
        channel->group() != QLCChannel::Tilt)
        return 0;

    QRectF rect = fixture->degreesRange(0);

    if (channel->group() == QLCChannel::Pan)
    {
        return (channel->controlByte() == QLCChannel::MSB) ? rect.width() : rect.width() / 255.0;
    }
    else
    {
        return (channel->controlByte() == QLCChannel::MSB) ? rect.height() : rect.height() / 255.0;
    }
}

void FixtureManager::slotFixtureAdded(quint32 id, QVector3D pos)
{
    if (m_doc->loadStatus() == Doc::Loading)
        return;

    // emit the creation signal so that ContextManager will
    // create the MonitorProperties
    emit newFixtureCreated(id, pos.x(), pos.y(), pos.z());

    if (m_fixtureTree == nullptr)
    {
        updateGroupsTree(m_doc, m_fixtureTree);
    }
    else
    {
        Fixture *fixture = m_doc->fixture(id);
        if (fixture == nullptr)
            return;

        QStringList uniNames = m_doc->inputOutputMap()->universeNames();
        QString universeName = uniNames.at(fixture->universe());
        int matchMask = 0;

        addFixtureNode(m_doc, m_fixtureTree, fixture, universeName, -1, matchMask);

        emit groupsTreeModelChanged();
    }
}

/*********************************************************************
 * Fixture groups
 *********************************************************************/

void FixtureManager::addFixturesToNewGroup(QList<quint32> fxList)
{
    FixtureGroup *group = new FixtureGroup(m_doc);
    m_doc->addFixtureGroup(group);
    group->setName(tr("New group %1").arg(group->id() + 1));

    // here we should build a "smart" grid based
    // on the 2D position of the fixtures and their heads number.
    // For now we use the "old" QLC+ mechanism of calculating an
    // equilateral grid size
    int headsCount = 0;
    for (quint32 id : fxList)
    {
        Fixture *fxi = m_doc->fixture(id);
        if (fxi != nullptr)
            headsCount += fxi->heads();
    }

    qreal side = qSqrt(headsCount);
    if (side != qFloor(side))
        side += 1; // Fixture number doesn't provide a full square

    group->setSize(QSize(side, side));
    for (quint32 id : fxList)
        group->assignFixture(id);

    Tardis::instance()->enqueueAction(Tardis::FixtureGroupCreate, group->id(), QVariant(),
                                      Tardis::instance()->actionToByteArray(Tardis::FixtureGroupCreate, group->id()));

    addFixtureGroupTreeNode(m_doc, m_fixtureTree, group, m_searchFilter);
}

void FixtureManager::updateFixtureGroup(quint32 groupID, quint32 itemID, int headIdx)
{
    FixtureGroup *group = m_doc->fixtureGroup(groupID);
    if (group == nullptr)
        return;

    quint32 fixtureID = FixtureUtils::itemFixtureID(itemID);
    Fixture *fixture = m_doc->fixture(fixtureID);
    if (fixture == nullptr)
        return;

    QList<int> headsList;
    if (headIdx != -1)
    {
        headsList << headIdx;
    }
    else
    {
        for (int i = 0; i < fixture->heads(); i++)
            headsList << i;
    }

    QString fxPath = QString("%1%2%3").arg(group->name()).arg(TreeModel::separator()).arg(fixture->name());
    TreeModelItem *fxItem = m_fixtureTree->itemAtPath(fxPath);

    if (fxItem == nullptr)
    {
        QVariantList fxParams;
        fxParams.append(QVariant::fromValue(fixture)); // classRef
        fxParams.append(App::FixtureDragItem); // type
        fxParams.append(itemID); // id
        fxParams.append(group->id()); // subid
        fxParams.append(0); // chIdx
        fxParams.append(true); // inGroup

        m_fixtureTree->addItem(fixture->name(), fxParams, group->name(), TreeModel::EmptyNode);
    }

    for (int hIdx : headsList)
    {
        QVariantList headParams;
        headParams.append(QVariant::fromValue(fixture)); // classRef
        headParams.append(App::HeadDragItem); // type
        headParams.append(itemID); // id
        headParams.append(group->id()); // subid
        headParams.append(hIdx); // chIdx
        headParams.append(true); // inGroup
        m_fixtureTree->addItem(QString("%1 %2").arg(tr("Head")).arg(hIdx + 1, 3, 10, QChar('0')), headParams, fxPath);
    }

    //m_fixtureTree->printTree(); // enable for debug purposes
}

bool FixtureManager::renameFixtureGroup(quint32 groupID, QString newName)
{
    FixtureGroup *group = m_doc->fixtureGroup(groupID);
    if (group == nullptr)
        return false;

    // check for same name among existing groups
    QList<FixtureGroup*> groupList = m_doc->fixtureGroups();
    for (FixtureGroup *docGroup : groupList)
        if (docGroup->name() == newName)
            return false;

    // check also among universe names since they are represented as groups
    for (QString &uniName : m_doc->inputOutputMap()->universeNames())
        if (uniName == newName)
            return false;

    group->setName(newName);

    updateGroupsTree(m_doc, m_fixtureTree, m_searchFilter);
    emit groupsTreeModelChanged();
    return true;
}

bool FixtureManager::deleteFixtureGroups(QVariantList IDList)
{
    for (QVariant id : IDList)
    {
        quint32 groupID = id.toUInt();
        Tardis::instance()->enqueueAction(Tardis::FixtureGroupDelete, groupID,
                                          Tardis::instance()->actionToByteArray(Tardis::FixtureGroupDelete, groupID),
                                          QVariant());
        m_doc->deleteFixtureGroup(groupID);
    }
    updateGroupsTree(m_doc, m_fixtureTree, m_searchFilter);
    emit groupsTreeModelChanged();

    return true;
}

void FixtureManager::slotFixtureGroupAdded(quint32 id)
{
    if (m_doc->loadStatus() == Doc::Loading)
        return;

    FixtureGroup *group = m_doc->fixtureGroup(id);
    if (group != nullptr)
        addFixtureGroupTreeNode(m_doc, m_fixtureTree, group, m_searchFilter);
}

/*********************************************************************
 * RGB Panel creation
 *********************************************************************/

bool FixtureManager::addRGBPanel(QString name, qreal xPos, qreal yPos)
{
    QQuickItem *propItem = qobject_cast<QQuickItem*>(m_view->rootObject()->findChild<QObject *>("RGBPanelProps"));
    if (propItem == nullptr)
        return false;

    int address = propItem->property("address").toInt();
    int uniIndex = propItem->property("universeIndex").toInt();

    int rows = propItem->property("rows").toInt();
    int columns = propItem->property("columns").toInt();
    int phyWidth = propItem->property("physicalWidth").toInt();
    qreal phyHeight = propItem->property("physicalHeight").toReal() / (qreal)rows;

    Fixture::Components components = Fixture::Components(propItem->property("components").toInt());
    PanelDirection direction = PanelDirection(propItem->property("direction").toInt());
    PanelOrientation orientation = PanelOrientation(propItem->property("startCorner").toInt());
    PanelType displacement = PanelType(propItem->property("displacement").toInt());

    FixtureGroup *grp = new FixtureGroup(m_doc);
    Q_ASSERT(grp != nullptr);

    QSize panelSize(columns, rows);
    grp->setSize(panelSize);
    m_doc->addFixtureGroup(grp);

    // make sure the name is unique
    QList<FixtureGroup*> groupList = m_doc->fixtureGroups();
    for (FixtureGroup *docGroup : groupList)
    {
        if (docGroup->name() == name)
        {
            name = QString ("%1 [%2]").arg(name).arg(grp->id());
            break;
        }
    }

    grp->setName(name);

    int transpose = 0;
    if (direction == Vertical)
    {
        int tmp = columns;
        columns = rows;
        rows = tmp;
        transpose = 1;
    }

    QLCFixtureDef *rowDef = nullptr;
    QLCFixtureMode *rowMode = nullptr;
    int currRow = 0;
    int rowInc = 1;
    int xPosStart = 0;
    int xPosEnd = columns - 1;
    int xPosInc = 1;

    if (transpose)
    {
        if (orientation == TopRight || orientation == BottomRight)
        {
            currRow = rows -1;
            rowInc = -1;
        }
        if (orientation == BottomRight || orientation == BottomLeft)
        {
            xPosStart = columns - 1;
            xPosEnd = 0;
            xPosInc = -1;
        }
    }
    else
    {
        if (orientation == BottomLeft || orientation == BottomRight)
        {
            currRow = rows -1;
            rowInc = -1;
        }
        if (orientation == TopRight || orientation == BottomRight)
        {
            xPosStart = columns - 1;
            xPosEnd = 0;
            xPosInc = -1;
        }
    }

    // temporarily disconnect this signal since we want to use the given position
    disconnect(m_doc, SIGNAL(fixtureAdded(quint32)), this, SLOT(slotFixtureAdded(quint32)));

    for (int i = 0; i < rows; i++)
    {
        Fixture *fxi = new Fixture(m_doc);
        Q_ASSERT(fxi != nullptr);
        fxi->setName(tr("%1 - Row %2").arg(name).arg(i + 1));
        if (rowDef == nullptr)
            rowDef = fxi->genericRGBPanelDef(columns, components, false);
        if (rowMode == nullptr)
            rowMode = fxi->genericRGBPanelMode(rowDef, components, false, phyWidth, phyHeight);
        fxi->setFixtureDefinition(rowDef, rowMode);

        // Check universe span
        if (address + fxi->channels() > 512)
        {
            uniIndex++;
            if (m_doc->inputOutputMap()->getUniverseID(uniIndex) == m_doc->inputOutputMap()->invalidUniverse())
            {
                m_doc->inputOutputMap()->addUniverse();
                m_doc->inputOutputMap()->startUniverses();
            }
            address = 1;
        }

        fxi->setUniverse(m_doc->inputOutputMap()->getUniverseID(uniIndex));
        fxi->setAddress(address - 1);
        address += fxi->channels();
        m_doc->addFixture(fxi);

        if (displacement == ZigZag)
        {
            int xPos = xPosStart;
            for (int h = 0; h < fxi->heads(); h++)
            {
                if (transpose)
                    grp->assignHead(QLCPoint(currRow, xPos), GroupHead(fxi->id(), h));
                else
                    grp->assignHead(QLCPoint(xPos, currRow), GroupHead(fxi->id(), h));
                xPos += xPosInc;
            }
        }
        else if (displacement == Snake)
        {
            if (i%2 == 0)
            {
                int xPos = xPosStart;
                for (int h = 0; h < fxi->heads(); h++)
                {
                    if (transpose)
                        grp->assignHead(QLCPoint(currRow, xPos), GroupHead(fxi->id(), h));
                    else
                        grp->assignHead(QLCPoint(xPos, currRow), GroupHead(fxi->id(), h));
                    xPos += xPosInc;
                }
            }
            else
            {
                int xPos = xPosEnd;
                for (int h = 0; h < fxi->heads(); h++)
                {
                    if (transpose)
                        grp->assignHead(QLCPoint(currRow, xPos), GroupHead(fxi->id(), h));
                    else
                        grp->assignHead(QLCPoint(xPos, currRow), GroupHead(fxi->id(), h));
                    xPos += (-xPosInc);
                }
            }
        }

        QVector3D pos;
        QVector3D rot = QVector3D(0, 0, 0);
        float gridUnits = m_monProps->gridUnits() == MonitorProperties::Meters ? 1000.0 : 304.8;

        switch (m_monProps->pointOfView())
        {
            case MonitorProperties::TopView:
                pos = QVector3D(xPos, 1000, yPos);
                if (displacement == Snake && i % 2)
                    rot.setY(180);
            break;
            case MonitorProperties::LeftSideView:
                pos = QVector3D(0, yPos, xPos);
                if (displacement == Snake && i % 2)
                    rot.setY(-90);
                else
                    rot.setY(90);
                rot.setZ(-90);
            break;
            case MonitorProperties::RightSideView:
                pos = QVector3D(0, yPos, (m_monProps->gridSize().z() * gridUnits) - xPos);
                if (displacement == Snake && i % 2)
                    rot.setY(90);
                else
                    rot.setY(-90);
                rot.setZ(90);
            break;
            default:
                pos = QVector3D(xPos, (m_monProps->gridSize().y() * gridUnits) - yPos, 0);
                if (displacement == Snake && i % 2)
                    rot.setZ(180);
                rot.setX(-90);
            break;
        }
        m_monProps->setFixturePosition(fxi->id(), 0, 0, pos);
        m_monProps->setFixtureRotation(fxi->id(), 0, 0, rot);
        slotFixtureAdded(fxi->id(), QVector3D(pos.x(), pos.y(), pos.z()));
        yPos += (qreal)phyHeight;
        currRow += rowInc;
    }

    connect(m_doc, SIGNAL(fixtureAdded(quint32)), this, SLOT(slotFixtureAdded(quint32)));

    m_fixtureList.clear();
    m_fixtureList = m_doc->fixtures();
    updateGroupsTree(m_doc, m_fixtureTree, m_searchFilter);
    emit fixturesCountChanged();
    emit fixturesMapChanged();

    return true;
}

/*********************************************************************
 * Universe Grid Editing
 *********************************************************************/

QVariantList FixtureManager::fixtureNamesMap()
{
    return m_fixtureNamesMap;
}

QString FixtureManager::getTooltip(quint32 address)
{
    quint32 uniFilter = m_universeFilter == Universe::invalid() ? 0 : m_universeFilter;

    quint32 fxID = m_doc->fixtureForAddress((uniFilter << 9) | address);
    if (fxID == Fixture::invalidId())
        return "";

    Fixture *fixture = m_doc->fixture(fxID);
    if (fixture == nullptr)
        return "";

    return fixture->name();
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
    m_fixtureNamesMap.clear();

    QList<Fixture*> origList = m_doc->fixtures();
    // sort the fixture list by address and not by ID
    std::sort(origList.begin(), origList.end(), compareFixtures);

    for (Fixture *fx : origList)
    {
        if (fx == nullptr)
            continue;

        if (fx->universe() != uniFilter)
            continue;

        quint32 startAddress = fx->address();
        for (quint32 cn = 0; cn < fx->channels(); cn++)
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
                m_fixturesMap.append(group);
        }
        odd = !odd;

        m_fixtureNamesMap.append(fx->id());
        m_fixtureNamesMap.append(fx->address());
        m_fixtureNamesMap.append(fx->channels());
        m_fixtureNamesMap.append(fx->name());
    }

    emit fixturesMapChanged();
    emit fixtureNamesMapChanged();

    return m_fixturesMap;
}

int FixtureManager::pasteFromClipboard(QVariantList fixtureIDs)
{
    if (fixtureIDs.isEmpty())
        return 0;

    // check destination universe
    Fixture *fixture = m_doc->fixture(fixtureIDs.first().toUInt());
    if (fixture == nullptr)
        return -1;

    if (fixture->universe() == m_universeFilter ||
        m_universeFilter == Universe::invalid())
        return -1;

    quint32 absAddress = (m_universeFilter << 9);
    int fixtureIndex = 0;
    QList<quint32> newAddresses;
    int availableAddress = 0;
    quint32 freeCounter = 0;

    // check available space
    for (int i = 0; i < 512; i++)
    {
        if (m_doc->fixtureForAddress(absAddress + i) != Fixture::invalidId())
        {
            freeCounter = 0;
            availableAddress = i + 1;
        }
        else
        {
            freeCounter++;
            if (freeCounter == fixture->channels())
            {
                // save the new address for this fixture
                newAddresses.append(availableAddress);
                fixtureIndex++;
                freeCounter = 0;
                availableAddress = i + 1;

                // all fixtures processed. Nothing more to do
                if (fixtureIndex == fixtureIDs.count())
                    break;

                fixture = m_doc->fixture(fixtureIDs.at(fixtureIndex).toUInt());
                if (fixture == nullptr)
                    return -1;
            }
        }
    }

    // not enough space to paste all the fixtures
    if (fixtureIndex != fixtureIDs.count())
        return -2;

    for (int f = 0; f < fixtureIDs.count(); f++)
    {
        Fixture *fxi = m_doc->fixture(fixtureIDs.at(f).toUInt());
        fxi->blockSignals(true);
        fxi->setUniverse(m_universeFilter);
        fxi->setAddress(newAddresses.at(f));
        fxi->blockSignals(false);

        // trigger one single changed signal
        fxi->setID(fxi->id());
    }

    updateGroupsTree(m_doc, m_fixtureTree, m_searchFilter);
    emit groupsTreeModelChanged();
    emit fixturesMapChanged();

    return 0;
}

/*********************************************************************
 * Color filters
 *********************************************************************/

QDir FixtureManager::systemColorFiltersDirectory()
{
    return QLCFile::systemDirectory(QString(COLORFILTERSDIR), QString(KExtColorFilters));
}

QDir FixtureManager::userColorFiltersDirectory()
{
    return QLCFile::userDirectory(QString(USERCOLORFILTERSDIR), QString(COLORFILTERSDIR),
                                  QStringList() << QString("*%1").arg(KExtColorFilters));
}

bool FixtureManager::loadColorFilters(const QDir &dir, bool user)
{
    qDebug() << Q_FUNC_INFO << dir.path();

    if (dir.exists() == false || dir.isReadable() == false)
        return false;

    /* Attempt to read all specified files from the given directory */
    QStringListIterator it(dir.entryList());
    while (it.hasNext() == true)
    {
        QString path(dir.absoluteFilePath(it.next()));

        if (path.toLower().endsWith(KExtColorFilters) == true)
        {
            ColorFilters *colFilter = new ColorFilters();
            Q_ASSERT(colFilter != nullptr);

            QFile::FileError error = colFilter->loadXML(path);
            if (error == QFile::NoError)
            {
                colFilter->setIsUser(user);
                m_colorFilters.append(colFilter);
            }
            else
            {
                qWarning() << Q_FUNC_INFO << "Color filters loading from"
                           << path << "failed:" << QLCFile::errorString(error);
                delete colFilter;
                colFilter = nullptr;
            }
        }
        else
            qWarning() << Q_FUNC_INFO << "Unrecognized color filters extension:" << path;
    }

    return true;
}

void FixtureManager::resetColorFilters()
{
    while (!m_colorFilters.isEmpty())
    {
        ColorFilters *cf = m_colorFilters.takeLast();
        delete cf;
    }
}

QStringList FixtureManager::colorFiltersFileList()
{
    QStringList list;

    if (m_colorFilters.isEmpty())
    {
        loadColorFilters(systemColorFiltersDirectory(), false);
        loadColorFilters(userColorFiltersDirectory(), true);
        emit selectedFiltersChanged();
    }

    for (ColorFilters *filters : m_colorFilters)
        list.append(filters->name());

    return list;
}

void FixtureManager::addColorFiltersFile()
{
    int newID = 1;

    /* count the existing user filters */
    for (ColorFilters *filter : m_colorFilters)
        if (filter->isUser())
            newID++;

    ColorFilters *newFilter = new ColorFilters();
    /* set a possibly unique name */
    newFilter->setName(tr("New filters %1").arg(newID));
    newFilter->setIsUser(true);

    QString targetName = QString("%1%2%3%4").arg(userColorFiltersDirectory().absolutePath())
                                            .arg(QDir::separator())
                                            .arg(newFilter->name().replace(' ', '_'))
                                            .arg(KExtColorFilters);

    qDebug() << "Target name is" << targetName;

    newFilter->saveXML(targetName);

    m_colorFilters.append(newFilter);

    emit colorFiltersFileListChanged();
    setColorFilterFileIndex(m_colorFilters.count() - 1);
}

int FixtureManager::colorFilterFileIndex() const
{
    return m_colorFiltersFileIndex;
}

void FixtureManager::setColorFilterFileIndex(int index)
{
    if (m_colorFiltersFileIndex == index)
        return;

    m_colorFiltersFileIndex = index;
    emit colorFilterFileIndexChanged(m_colorFiltersFileIndex);
    emit selectedFiltersChanged();
}

ColorFilters *FixtureManager::selectedFilters()
{
    if (m_colorFiltersFileIndex < 0 || m_colorFiltersFileIndex >= m_colorFilters.count())
        return nullptr;

    return m_colorFilters.at(m_colorFiltersFileIndex);
}

/*********************************************************************
 * Channel capabilities
 *********************************************************************/

void FixtureManager::setChannelValue(quint32 fixtureID, quint32 channelIndex, quint8 value)
{
    emit channelValueChanged(fixtureID, channelIndex, value);
}

void FixtureManager::setPresetValue(quint32 fixtureID, int chIndex, quint8 value)
{
    qDebug() << "[FixtureManager] setPresetValue - fixture:" << fixtureID << ", channel:" << chIndex << "value:" << value;

    Fixture *fixture = m_doc->fixture(fixtureID);
    if (fixture == nullptr || fixture->fixtureMode() == nullptr)
        return;

    const QLCChannel *ch = fixture->fixtureMode()->channel(chIndex);
    emit presetChanged(ch, value);
}

void FixtureManager::updateCapabilityCounter(bool update, QString capName, int delta)
{
    if (update == false)
        return;

    QQuickItem *capItem = qobject_cast<QQuickItem*>(m_view->rootObject()->findChild<QObject *>(capName));
    if (capItem != nullptr)
    {
        int count = capItem->property("counter").toInt() + delta;
        capItem->setProperty("counter", count);

        if (count == 0)
            return;

        if (capName == "capPosition")
        {
            capItem->setProperty("panDegrees", m_maxPanDegrees);
            capItem->setProperty("tiltDegrees", m_maxTiltDegrees);
        }
        else if (capName == "capBeam")
        {
            QMetaObject::invokeMethod(capItem, "setZoomRange",
                    Q_ARG(QVariant, m_minBeamDegrees),
                    Q_ARG(QVariant, m_maxBeamDegrees),
                    Q_ARG(QVariant, m_invertedZoom));
        }
    }
}

void FixtureManager::setCapabilityCounter(QString capName, int value)
{
    QQuickItem *capItem = qobject_cast<QQuickItem*>(m_view->rootObject()->findChild<QObject *>(capName));
    if (capItem)
        capItem->setProperty("counter", value);
}

QMultiHash<int, SceneValue> FixtureManager::getFixtureCapabilities(quint32 itemID, int headIndex, bool enable)
{
    int capDelta = enable ? 1 : -1;
    bool hasDimmer = false, hasColor = false, hasPosition = false;
    bool hasShutter = false, hasColorWheel = false, hasGobos = false;
    bool hasBeam = false;
    int origColorsMask = m_colorsMask;
    quint32 origCapabilityMask = m_capabilityMask;
    QLCPhysical phy;

    QList<quint32> channelIndices;
    QMultiHash<int, SceneValue> channelsMap;
    quint32 fixtureID = FixtureUtils::itemFixtureID(itemID);

    Fixture *fixture = m_doc->fixture(fixtureID);
    if (fixture == nullptr)
        return channelsMap;

    // build a list of channel indices depending on
    // if a head has to be considered or not
    if (headIndex == -1)
    {
        for (quint32 i = 0; i < fixture->channels(); i++)
            channelIndices.append(i);
    }
    else
    {
        QLCFixtureHead head = fixture->head(headIndex);
        channelIndices = head.channels();
    }

    if (fixture->fixtureMode() != nullptr)
        phy = fixture->fixtureMode()->physical();

    for (quint32 ch : channelIndices)
    {
        const QLCChannel *channel(fixture->channel(ch));
        if (channel == nullptr)
            continue;

        int chType = channel->group();

        if (chType == QLCChannel::Intensity)
        {
            if (channel->colour() == QLCChannel::NoColour)
                m_capabilityMask |= App::DimmerType;
            else
                m_capabilityMask |= App::ColorType;
        }
        else
        {
            m_capabilityMask |= (1 << chType);
        }

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
                        channelsMap.insert(chType, SceneValue(fixtureID, ch));
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
                        channelsMap.insert(chType, SceneValue(fixtureID, ch));
                    break;
                    default: break;
                }
            }
            break;
            case QLCChannel::Pan:
            case QLCChannel::Tilt:
            {
                hasPosition = true;
                if (fixture->fixtureMode() != nullptr)
                {
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
                channelsMap.insert(chType, SceneValue(fixtureID, ch));
            }
            break;
            case QLCChannel::Shutter:
            {
                hasShutter = true;
                if (enable)
                {
                    if (m_presetsCache.contains(channel) == false)
                    {
                        m_presetsCache[channel] = fixtureID;
                        emit shutterChannelsChanged();
                    }
                }
                else
                {
                    m_presetsCache.remove(channel);
                    emit shutterChannelsChanged();
                }
                channelsMap.insert(chType, SceneValue(fixtureID, ch));
            }
            break;
            case QLCChannel::Colour:
            {
                hasColorWheel = true;
                if (enable)
                {
                    if (m_presetsCache.contains(channel) == false)
                    {
                        m_presetsCache[channel] = fixtureID;
                        emit colorWheelChannelsChanged();
                    }
                }
                else
                {
                    m_presetsCache.remove(channel);
                    emit colorWheelChannelsChanged();
                }
                channelsMap.insert(chType, SceneValue(fixtureID, ch));
            }
            break;
            case QLCChannel::Gobo:
            {
                hasGobos = true;
                if (enable)
                {
                    if (m_presetsCache.contains(channel) == false)
                    {
                        m_presetsCache[channel] = fixtureID;
                        emit goboChannelsChanged();
                    }
                }
                else
                {
                    m_presetsCache.remove(channel);
                    emit goboChannelsChanged();
                }
                channelsMap.insert(chType, SceneValue(fixtureID, ch));
            }
            break;
            case QLCChannel::Beam:
            {
                if (channel->preset() != QLCChannel::BeamZoomBigSmall &&
                    channel->preset() != QLCChannel::BeamZoomSmallBig &&
                    channel->preset() != QLCChannel::BeamZoomFine)
                    break;

                hasBeam = true;
                if (fixture->fixtureMode() != nullptr)
                {
                    m_minBeamDegrees = phy.lensDegreesMin();

                    if (phy.lensDegreesMax() > m_maxBeamDegrees)
                        m_maxBeamDegrees = phy.lensDegreesMax();

                    if (m_maxBeamDegrees == 0)
                        m_maxBeamDegrees = 30.0;

                    // this considers only the last selected fixture
                    if (channel->preset() == QLCChannel::BeamZoomBigSmall)
                        m_invertedZoom = true;
                    else
                        m_invertedZoom = false;
                }
                channelsMap.insert(chType, SceneValue(fixtureID, ch));
            }
            break;
            default:
            break;
        }
    }

    if (origColorsMask != m_colorsMask)
        emit colorsMaskChanged(m_colorsMask);

    if (origCapabilityMask != m_capabilityMask)
        emit capabilityMaskChanged();

    updateCapabilityCounter(hasDimmer, "capIntensity", capDelta);
    updateCapabilityCounter(hasColor, "capColor", capDelta);
    updateCapabilityCounter(hasPosition, "capPosition", capDelta);
    updateCapabilityCounter(hasShutter, "capShutter", capDelta);
    updateCapabilityCounter(hasColorWheel, "capColorWheel", capDelta);
    updateCapabilityCounter(hasGobos, "capGobos", capDelta);
    updateCapabilityCounter(hasBeam, "capBeam", capDelta);

    return channelsMap;
}

void FixtureManager::resetCapabilities()
{
    m_maxPanDegrees = 0;
    m_maxTiltDegrees = 0;
    m_minBeamDegrees = 15.0;
    m_maxBeamDegrees = 0;
    m_colorsMask = 0;
    m_capabilityMask = 0;

    emit capabilityMaskChanged();

    setCapabilityCounter("capIntensity", 0);
    setCapabilityCounter("capColor", 0);
    setCapabilityCounter("capPosition", 0);
    setCapabilityCounter("capColorWheel", 0);
    setCapabilityCounter("capGobos", 0);
    setCapabilityCounter("capShutter", 0);
    setCapabilityCounter("capBeam", 0);
}

QList<SceneValue> FixtureManager::getFixturePosition(quint32 fxID, int type, int degrees)
{
    Fixture *fixture = m_doc->fixture(fxID);
    if (fixture == nullptr || fixture->fixtureMode() == nullptr)
        return QList<SceneValue>();

    return fixture->positionToValues(type, degrees);
}

QList<SceneValue> FixtureManager::getFixtureZoom(quint32 fxID, float degrees)
{
    Fixture *fixture = m_doc->fixture(fxID);
    if (fixture == nullptr || fixture->fixtureMode() == nullptr)
        return QList<SceneValue>();

    return fixture->zoomToValues(degrees, false);
}

QVariantList FixtureManager::presetsChannels(QLCChannel::Group group)
{
    QVariantList prList;

    for (const QLCChannel *ch : m_presetsCache.keys())
    {
        if (ch->group() != group)
            continue;

        quint32 fxID = m_presetsCache[ch];
        Fixture *fixture = m_doc->fixture(fxID);
        if (fixture == nullptr)
            continue;

        const QLCFixtureDef *def = fixture->fixtureDef();
        if (def != nullptr)
        {
            QLCChannel *nonConstCh = const_cast<QLCChannel *>(ch);
            quint32 idx = fixture->fixtureMode()->channelNumber(nonConstCh);
            QVariantMap prMap;
            prMap.insert("name", QString("%1 - %2")
                                .arg(def->model())
                                .arg(ch->name()));
            prMap.insert("fixtureID", fxID);
            prMap.insert("channelIdx", idx);
            prList.append(prMap);
        }
    }

    return prList;
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

QVariantList FixtureManager::shutterChannels()
{
    return presetsChannels(QLCChannel::Shutter);
}

QVariantList FixtureManager::presetCapabilities(quint32 fixtureID, int chIndex)
{
    QVariantList var;

    Fixture *fixture = m_doc->fixture(fixtureID);
    if (fixture == nullptr || fixture->fixtureMode() == nullptr)
        return var;

    qDebug() << "[FixtureManager] Requesting presets for fixture" << fixtureID << ", channel:" << chIndex;

    const QLCChannel *ch = fixture->fixtureMode()->channel(chIndex);

    for (QLCCapability *cap : ch->capabilities())
        var.append(QVariant::fromValue(cap));

    return var;
}

QVariantList FixtureManager::presetChannel(quint32 fixtureID, int chIndex)
{
    QVariantList prList;

    Fixture *fixture = m_doc->fixture(fixtureID);
    if (fixture == nullptr)
        return prList;

    const QLCFixtureDef *def = fixture->fixtureDef();
    const QLCFixtureMode *mode = fixture->fixtureMode();

    if (def == nullptr || mode == nullptr)
        return prList;

    const QLCChannel *ch = fixture->channel(chIndex);

    if (ch == nullptr)
        return prList;

    QVariantMap prMap;
    prMap.insert("name", ch->name());
    prMap.insert("fixtureID", fixtureID);
    prMap.insert("channelIdx", chIndex);
    prList.append(prMap);

    return prList;
}

quint32 FixtureManager::capabilityMask() const
{
    return m_capabilityMask;
}

int FixtureManager::colorsMask() const
{
    return m_colorsMask;
}

/*********************************************************************
 * Channel modifiers
 *********************************************************************/

QStringList FixtureManager::channelModifiersList() const
{
    QList<QString> names = m_doc->modifiersCache()->templateNames();
    names.sort();
    names.prepend("None");

    return names;
}

void FixtureManager::selectChannelModifier(QString name)
{
    m_selectedChannelModifier = m_doc->modifiersCache()->modifier(name);

    emit channelModifierValuesChanged();
}

void FixtureManager::setChannelModifier(quint32 itemID, quint32 channelIndex)
{
    quint32 fixtureID = FixtureUtils::itemFixtureID(itemID);
    Fixture *fixture = m_doc->fixture(fixtureID);
    if (fixture == nullptr)
        return;

    fixture->setChannelModifier(channelIndex, m_selectedChannelModifier);

    // update UI tree
    setItemRoleData(itemID, channelIndex, "modifier", m_selectedChannelModifier == nullptr ?
                    "None" : m_selectedChannelModifier->name());
}

void FixtureManager::showModifierEditor(quint32 itemID, quint32 channelIndex)
{
    quint32 fixtureID = FixtureUtils::itemFixtureID(itemID);
    Fixture *fixture = m_doc->fixture(fixtureID);
    if (fixture == nullptr)
        return;

    ChannelModifier *cm = fixture->channelModifier(channelIndex);

    QQuickItem *fgmItem = qobject_cast<QQuickItem*>(m_view->rootObject()->findChild<QObject *>("fixtureGroupManager"));
    if (fgmItem == nullptr)
        return;

    selectChannelModifier(cm == nullptr ? "" : cm->name());

    QMetaObject::invokeMethod(fgmItem, "showChannelModifierEditor",
            Q_ARG(QVariant, itemID),
            Q_ARG(QVariant, channelIndex),
            Q_ARG(QVariant, cm == nullptr ? "" : cm->name()));
}

QVariantList FixtureManager::channelModifierValues() const
{
    QVariantList values;

    if (m_selectedChannelModifier != nullptr)
    {
        QList< QPair<uchar, uchar> > map = m_selectedChannelModifier->modifierMap();
        for (int i = 0; i < map.count(); i++)
        {
            QPair<uchar, uchar> dmxPair = map.at(i);
            values.append(dmxPair.first);
            values.append(dmxPair.second);
        }
    }

    return values;
}


