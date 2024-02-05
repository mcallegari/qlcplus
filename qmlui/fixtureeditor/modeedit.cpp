/*
  Q Light Controller Plus
  modeedit.cpp

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

#include "qlcfixturemode.h"
#include "listmodel.h"
#include "modeedit.h"

ModeEdit::ModeEdit(QLCFixtureMode *mode, QObject *parent)
    : QObject(parent)
    , m_mode(mode)
    , m_physical(nullptr)
{
    m_channelList = new ListModel(this);
    QStringList chRoles;
    chRoles << "cRef" << "isSelected";
    m_channelList->setRoleNames(chRoles);

    updateChannelList();

    m_headList = new ListModel(this);
    QStringList headRoles;
    headRoles << "channelList" << "isSelected";
    m_headList->setRoleNames(headRoles);

    updateHeadsList();
}

ModeEdit::~ModeEdit()
{

}

QString ModeEdit::name() const
{
    return m_mode->name();
}

void ModeEdit::setName(QString name)
{
    if (name == m_mode->name())
        return;

    m_mode->setName(name);
    emit nameChanged();
}

/************************************************************************
 * Channels
 ************************************************************************/

QVariant ModeEdit::channels() const
{
    return QVariant::fromValue(m_channelList);
}

void ModeEdit::addChannel(QLCChannel *channel, int insertIndex)
{
    m_mode->insertChannel(channel, insertIndex);
    updateChannelList();
}

void ModeEdit::moveChannel(QLCChannel *channel, int insertIndex)
{
    int index = m_mode->channelNumber(channel);
    if (index < insertIndex)
        insertIndex--;

    m_mode->removeChannel(channel);
    m_mode->insertChannel(channel, insertIndex);
    updateChannelList();
}

QLCChannel *ModeEdit::channelFromIndex(int index) const
{
    return m_mode->channel(index);
}

bool ModeEdit::deleteChannel(QLCChannel *channel)
{
    // TODO: Tardis
    bool res = m_mode->removeChannel(channel);
    updateChannelList();

    return res;
}

QStringList ModeEdit::actsOnChannels()
{
    QStringList list;
    list << "-";

    for (QLCChannel *channel : m_mode->channels())
        list << channel->name();

    return list;
}

int ModeEdit::actsOnChannel(int index)
{
    quint32 actsOnChannelIndex = m_mode->channelActsOn(index);

    if (actsOnChannelIndex != QLCChannel::invalid())
        return actsOnChannelIndex + 1;
    else
        return 0;
}

void ModeEdit::setActsOnChannel(int sourceIndex, int destIndex)
{
    quint32 actsOnChannel = destIndex == 0 ? QLCChannel::invalid() : destIndex - 1;
    m_mode->setChannelActsOn(sourceIndex, actsOnChannel);
}

void ModeEdit::updateChannelList()
{
    m_channelList->clear();

    for (QLCChannel *channel : m_mode->channels())
    {
        QVariantMap chanMap;
        chanMap.insert("cRef", QVariant::fromValue(channel));
        chanMap.insert("isSelected", false);
        m_channelList->addDataMap(chanMap);
    }

    emit channelsChanged();
    emit actsOnChannelsChanged();
}

/************************************************************************
 * Heads
 ************************************************************************/

QVariant ModeEdit::heads() const
{
    return QVariant::fromValue(m_headList);
}

void ModeEdit::addHead(QVariantList chIndexList)
{
    QLCFixtureHead head;

    for (QVariant &idx : chIndexList)
        head.addChannel(idx.toUInt());

    m_mode->insertHead(-1, head);
    updateHeadsList();
}

void ModeEdit::deleteHeads(QVariantList headIndexList)
{
    QVector<int> sortedList;
    for (QVariant &idx : headIndexList)
        sortedList.append(idx.toInt());
    std::sort(sortedList.begin(), sortedList.end(), std::greater<int>());

    for (int index : sortedList)
        m_mode->removeHead(index);

    updateHeadsList();
}

void ModeEdit::updateHeadsList()
{
    m_headList->clear();

    for (QLCFixtureHead head : m_mode->heads())
    {
        QVariantMap headMap;
        headMap.insert("channelList", QVariant::fromValue(head.channels()));
        headMap.insert("isSelected", false);
        m_headList->addDataMap(headMap);
    }

    emit headsChanged();
}

/************************************************************************
 * Physical
 ************************************************************************/

bool ModeEdit::useGlobalPhysical()
{
    return m_mode->useGlobalPhysical();
}

PhysicalEdit *ModeEdit::physical()
{
    if (m_physical == nullptr)
        m_physical = new PhysicalEdit(m_mode->physical());

    return m_physical;
}


