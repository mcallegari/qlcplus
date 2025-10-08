/*
  Q Light Controller Plus
  fixturebrowser.cpp

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

#include <QQmlEngine>
#include <QQuickItem>
#include <QQmlContext>
#include <QDebug>

#include "fixturebrowser.h"
#include "qlcfixturemode.h"
#include "qlcfixturedef.h"
#include "treemodelitem.h"
#include "treemodel.h"
#include "doc.h"

FixtureBrowser::FixtureBrowser(QQuickView *view, Doc *doc, QObject *parent)
    : QObject(parent)
    , m_doc(doc)
    , m_view(view)
    , m_manufacturerIndex(0)
    , m_selectedManufacturer(QString())
    , m_selectedModel(QString())
    , m_fixtureName(QString())
    , m_selectedMode(QString())
    , m_modeChannelsCount(1)
    , m_mode(nullptr)
    , m_searchFilter(QString())
{
    Q_ASSERT(m_doc != nullptr);
    Q_ASSERT(m_view != nullptr);

    m_view->rootContext()->setContextProperty("fixtureBrowser", this);

    m_searchTree = new TreeModel(this);
    QQmlEngine::setObjectOwnership(m_searchTree, QQmlEngine::CppOwnership);
    m_searchTree->enableSorting(true);
}

FixtureBrowser::~FixtureBrowser()
{
    m_view->rootContext()->setContextProperty("fixtureBrowser", nullptr);
}

QStringList FixtureBrowser::manufacturers()
{
    if (m_defCache.isEmpty())
        m_defCache = m_doc->fixtureDefCache()->fixtureCache();

    QStringList mfList = m_defCache.keys();
    mfList.sort(Qt::CaseInsensitive);
    m_manufacturerIndex = mfList.indexOf("Generic");
    emit manufacturerIndexChanged(m_manufacturerIndex);
    return mfList;
}

QString FixtureBrowser::selectedManufacturer() const
{
    return m_selectedManufacturer;
}

void FixtureBrowser::setSelectedManufacturer(QString selectedManufacturer)
{
    if (m_selectedManufacturer == selectedManufacturer)
        return;

    m_selectedManufacturer = selectedManufacturer;
    emit selectedManufacturerChanged(selectedManufacturer);
    emit modelsListChanged();
}

QStringList FixtureBrowser::modelsList()
{
    if (m_selectedManufacturer.isEmpty())
        return QStringList();

    if (m_defCache.isEmpty())
        m_defCache = m_doc->fixtureDefCache()->fixtureCache();

    qDebug() << "[FixtureBrowser] Models list for" << m_selectedManufacturer;
    QStringList fxList = m_defCache[m_selectedManufacturer].keys();
    if (m_selectedManufacturer == "Generic")
    {
        fxList << "Generic Dimmer";
        fxList << "Generic RGB Panel";
    }

    fxList.sort(Qt::CaseInsensitive);
    return fxList;
}

bool FixtureBrowser::isUserDefinition(QString manufacturer, QString model)
{
    return m_defCache[manufacturer].value(model, false);
}

QString FixtureBrowser::selectedModel() const
{
    return m_selectedModel;
}

void FixtureBrowser::setSelectedModel(QString selectedModel)
{
    if (m_selectedModel == selectedModel)
        return;

    m_selectedModel = selectedModel;
    setFixtureName(m_selectedModel);
    m_selectedMode = QString();
    m_modeChannelsCount = 1;
    emit selectedModelChanged(selectedModel);
    emit modesListChanged();
    emit modeChannelsCountChanged();
    emit modeChannelListChanged();
}

QString FixtureBrowser::fixtureName() const
{
    return m_fixtureName;
}

void FixtureBrowser::setFixtureName(QString fixtureName)
{
    if (m_fixtureName == fixtureName)
        return;

    m_fixtureName = fixtureName;
    emit fixtureNameChanged(fixtureName);
}

QStringList FixtureBrowser::modesList()
{
    qDebug() << "[FixtureBrowser] Modes list for" << m_selectedManufacturer << m_selectedModel;

    QStringList modesList;

    QLCFixtureDef *definition = fixtureDefinition();

    if (definition != nullptr)
    {
        QList<QLCFixtureMode *> fxModesList = definition->modes();
        foreach (QLCFixtureMode *mode, fxModesList)
        {
            modesList.append(mode->name());
            if (m_selectedMode.isEmpty())
            {
                m_selectedMode = mode->name();
                m_modeChannelsCount = mode->channels().count();
            }
        }
    }
    return modesList;
}

QString FixtureBrowser::selectedMode() const
{
    return m_selectedMode;
}

void FixtureBrowser::setSelectedMode(QString selectedMode)
{
    qDebug() << "[FixtureBrowser] Select mode for" << m_selectedManufacturer << m_selectedModel << selectedMode;

    if (m_selectedMode == selectedMode)
        return;

    m_selectedMode = selectedMode;
    QLCFixtureDef *definition = fixtureDefinition();

    if (definition != nullptr)
    {
        m_mode = definition->mode(m_selectedMode);
        if (m_mode)
            m_modeChannelsCount = m_mode->channels().count();
    }
    emit selectedModeChanged(selectedMode);
    emit modeChannelsCountChanged();
    emit modeChannelListChanged();
}

int FixtureBrowser::modeChannelsCount()
{
    QLCFixtureDef *definition = fixtureDefinition();

    if (definition != nullptr)
    {
        m_mode = definition->mode(m_selectedMode);

        if (m_mode != nullptr)
            return m_mode->channels().count();
    }
    return m_modeChannelsCount;
}

void FixtureBrowser::setModeChannelsCount(int modeChannelsCount)
{
    if (m_modeChannelsCount == modeChannelsCount)
        return;

    m_modeChannelsCount = modeChannelsCount;
    emit modeChannelsCountChanged();
}

QVariant FixtureBrowser::modeChannelList() const
{
    QVariantList channelList;

    if (m_mode != nullptr)
    {
        int i = 1;
        for (QLCChannel *channel : m_mode->channels()) // C++11
        {
            QVariantMap chMap;
            chMap.insert("mIcon", channel->getIconNameFromGroup(channel->group(), true));
            chMap.insert("mLabel", QString("%1: %2").arg(i++).arg(channel->name()));
            channelList.append(chMap);
        }
    }

    return QVariant::fromValue(channelList);
}

int FixtureBrowser::manufacturerIndex() const
{
    return m_manufacturerIndex;
}

void FixtureBrowser::setManufacturerIndex(int index)
{
    if (m_manufacturerIndex == index)
        return;

    m_manufacturerIndex = index;
    emit manufacturerIndexChanged(index);
}

int FixtureBrowser::availableChannel(quint32 uniIdx, int channels, int quantity, int gap, int requested)
{
    qDebug() << "[FixtureBrowser] uniIdx:" << uniIdx << ", channels:" << channels << ", requested:" << requested;
    bool isAvailable = true;
    quint32 uniFilter = uniIdx == Universe::invalid() ? 0 : uniIdx;
    quint32 absAddress = (requested & 0x01FF) | (uniFilter << 9);
    for (int n = 0; n < quantity; n++)
    {
        for (int i = 0; i < channels; i++)
        {
            if (m_doc->fixtureForAddress(absAddress + i) != Fixture::invalidId())
            {
                isAvailable = false;
                break;
            }
        }
        absAddress += channels + gap;
    }
    if (isAvailable == true)
    {
        qDebug() << "[FixtureBrowser] Requested channel is available:" << requested;
        return requested;
    }
    else
    {
        qDebug() << "[FixtureBrowser] Requested channel" << requested << "not available in universe" << uniFilter;
        int validAddr = 0;
        int freeCounter = 0;
        absAddress = uniFilter << 9;
        for (int i = 0; i < 512; i++)
        {
            if (m_doc->fixtureForAddress(absAddress + i) != Fixture::invalidId())
            {
                freeCounter = 0;
                validAddr = i + 1;
            }
            else
                freeCounter++;

            if (freeCounter == (channels * quantity) + (gap * quantity))
            {
                qDebug() << "[FixtureBrowser] Returning available address:" << validAddr;
                return validAddr;
            }
        }
    }

    return -1;
}

int FixtureBrowser::availableChannel(quint32 fixtureID, int requested)
{
    qDebug() << "[FixtureBrowser] fxID:" << fixtureID << ", requested:" << requested;
    bool isAvailable = true;

    Fixture *fixture = m_doc->fixture(fixtureID);
    if (fixture == nullptr)
        return -1;

    quint32 channels = fixture->channels();
    quint32 absAddress = (requested & 0x01FF) | (fixture->universe() << 9);

    for (quint32 i = 0; i < channels; i++)
    {
        quint32 fxIDOnAddr = m_doc->fixtureForAddress(absAddress + i);
        if (fxIDOnAddr != Fixture::invalidId() && fxIDOnAddr != fixtureID)
        {
            isAvailable = false;
            break;
        }
    }

    if (isAvailable == true)
    {
        qDebug() << "[FixtureBrowser] Requested channel is available:" << requested;
        return requested;
    }

    return -1;
}

QString FixtureBrowser::searchFilter() const
{
    return m_searchFilter;
}

void FixtureBrowser::setSearchFilter(QString searchFilter)
{
    if (m_searchFilter == searchFilter)
        return;

    if (m_searchFilter.length() >= SEARCH_MIN_CHARS && searchFilter.length() < SEARCH_MIN_CHARS)
    {
        m_selectedManufacturer = "";
        emit selectedManufacturerChanged(m_selectedManufacturer);
    }

    m_searchFilter = searchFilter;

    if (searchFilter.length() >= SEARCH_MIN_CHARS)
        updateSearchTree();
    else
    {
        m_searchTree->clear();
        emit searchListChanged();
    }

    emit searchFilterChanged(searchFilter);
}

QVariant FixtureBrowser::searchTreeModel() const
{
    return QVariant::fromValue(m_searchTree);
}

void FixtureBrowser::updateSearchTree()
{
    m_searchTree->clear();

    QStringList mfList = m_doc->fixtureDefCache()->manufacturers();
    mfList.sort();
    QString searchFilter = m_searchFilter.toLower();

    for (QString &manufacturer : mfList) // C++11
    {
        QStringList modelsList = m_doc->fixtureDefCache()->models(manufacturer);
        modelsList.sort();

        for (QString &model : modelsList)
        {
            if (manufacturer.toLower().contains(searchFilter) ||
                model.toLower().contains(searchFilter))
            {
                QVariantList params;
                TreeModelItem *item = m_searchTree->addItem(model, params, manufacturer);
                item->setFlag(TreeModel::Expanded, true);
            }
        }
    }
    emit searchListChanged();
}

QLCFixtureDef *FixtureBrowser::fixtureDefinition()
{
    return m_doc->fixtureDefCache()->fixtureDef(m_selectedManufacturer, m_selectedModel);
}

