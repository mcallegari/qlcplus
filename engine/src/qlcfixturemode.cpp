/*
  Q Light Controller Plus
  qlcfixturemode.cpp

  Copyright (C) Heikki Junnila
                Massimo Callegari

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

#include <QXmlStreamReader>
#include <iostream>
#include <QString>
#include <QDebug>
#include <QVector>

#include "qlcfixturemode.h"
#include "qlcfixturehead.h"
#include "qlcfixturedef.h"
#include "qlcchannel.h"
#include "qlcphysical.h"

QLCFixtureMode::QLCFixtureMode(QLCFixtureDef* fixtureDef)
    : m_fixtureDef(fixtureDef)
    , m_masterIntensityChannel(QLCChannel::invalid())
    , m_useGlobalPhysical(true)
{
    Q_ASSERT(fixtureDef != NULL);
}

QLCFixtureMode::QLCFixtureMode(QLCFixtureDef* fixtureDef, const QLCFixtureMode* mode)
    : m_fixtureDef(fixtureDef)
    , m_actsOnChannelsList(mode->actsOnChannelsList())
    , m_masterIntensityChannel(QLCChannel::invalid())
    , m_useGlobalPhysical(true)
{
    Q_ASSERT(fixtureDef != NULL);
    Q_ASSERT(mode != NULL);

    if (mode != NULL)
        *this = *mode;
}

QLCFixtureMode::~QLCFixtureMode()
{
}

QLCFixtureMode& QLCFixtureMode::operator=(const QLCFixtureMode& mode)
{
    if (this != &mode)
    {
        m_name = mode.m_name;
        m_useGlobalPhysical = mode.m_useGlobalPhysical;
        m_physical = mode.m_physical;
        m_heads = mode.m_heads;
        m_masterIntensityChannel = QLCChannel::invalid();
        m_actsOnChannelsList = mode.actsOnChannelsList();

        /* Clear the existing list of channels */
        m_channels.clear();

        Q_ASSERT(m_fixtureDef != NULL);

        quint32 i = 0;
        QVectorIterator <QLCChannel*> it(mode.m_channels);
        while (it.hasNext() == true)
        {
            /* Since m_fixtureDef might not be the same as
               mode.m_fixtureDef, we need to search for a
               channel with the same name from m_fixtureDef and
               not from mode.m_fixtureDef. If the channel in the
               other mode is deleted, the one in this copied mode
               will be invalid and we end up in a crash. */
            QLCChannel* ch = it.next();
            QLCChannel* actual = m_fixtureDef->channel(ch->name());
            if (actual != NULL)
                insertChannel(actual, i++);
            else
                qWarning() << Q_FUNC_INFO << "Unable to find channel"
                           << ch->name() << "for mode"
                           << m_name << "from its fixture definition";
        }
    }

    return *this;
}

/****************************************************************************
 * Name
 ****************************************************************************/

void QLCFixtureMode::setName(const QString &name)
{
    m_name = name;
}

QString QLCFixtureMode::name() const
{
    return m_name;
}

/*****************************************************************************
 * Fixture definition
 *****************************************************************************/

QLCFixtureDef* QLCFixtureMode::fixtureDef() const
{
    return m_fixtureDef;
}

/****************************************************************************
 * Channels
 ****************************************************************************/

bool QLCFixtureMode::insertChannel(QLCChannel* channel, quint32 index)
{
    if (channel == NULL)
    {
        qWarning() << Q_FUNC_INFO << "Will not add a NULL channel to mode"
                   << m_name;
        return false;
    }

    Q_ASSERT(m_fixtureDef != NULL);

    if (m_fixtureDef->channels().contains(channel) == true)
    {
        if (m_channels.contains(channel) == false)
        {
            if (index >= quint32(m_channels.size()))
                m_channels.append(channel);
            else
                m_channels.insert(index, channel);
            return true;
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Channel" << channel->name()
                       << "is already a member of mode" << m_name;
            return false;
        }
    }
    else
    {
        qWarning() << Q_FUNC_INFO << "Will not add channel" << channel->name()
                   << "to mode" << m_name
                   << "because the channel does not belong to mode's"
                   << "parent fixture definition.";
        return false;
    }
}

bool QLCFixtureMode::removeChannel(const QLCChannel* channel)
{
    QMutableVectorIterator <QLCChannel*> it(m_channels);
    while (it.hasNext() == true)
    {
        if (it.next() == channel)
        {
            /* Don't delete the channel since QLCFixtureModes
               don't own them. QLCFixtureDefs do. */
            it.remove();
            return true;
        }
    }

    return false;
}

bool QLCFixtureMode::replaceChannel(QLCChannel *currChannel, QLCChannel *newChannel)
{
    if (currChannel == NULL || newChannel == NULL)
        return false;

    int chIndex = m_channels.indexOf(currChannel);
    if (chIndex == -1)
        return false;

    m_channels.replace(chIndex, newChannel);

    return true;
}

void QLCFixtureMode::removeAllChannels()
{
    m_channels.clear();
}

QLCChannel* QLCFixtureMode::channel(const QString& name) const
{
    QVectorIterator <QLCChannel*> it(m_channels);
    while (it.hasNext() == true)
    {
        QLCChannel* ch = it.next();
        Q_ASSERT(ch != NULL);
        if (ch->name() == name)
            return ch;
    }

    return NULL;
}

QLCChannel* QLCFixtureMode::channel(quint32 ch) const
{
    return m_channels.value(ch, NULL);
}

QVector <QLCChannel*> QLCFixtureMode::channels() const
{
    return m_channels;
}

quint32 QLCFixtureMode::channelNumber(QLCChannel* channel) const
{
    if (channel == NULL)
        return QLCChannel::invalid();

    int idx = m_channels.indexOf(channel);
    return idx == -1 ? QLCChannel::invalid() : idx;
}

quint32 QLCFixtureMode::channelNumber(QLCChannel::Group group, QLCChannel::ControlByte cByte) const
{
    for (int i = 0; i < m_channels.size(); i++)
    {
        if (m_channels.at(i)->group() == group &&
            m_channels.at(i)->controlByte() == cByte)
            return i;
    }

    return QLCChannel::invalid();
}

quint32 QLCFixtureMode::masterIntensityChannel() const
{
    return m_masterIntensityChannel;
}

void QLCFixtureMode::updateActsOnChannel(QLCChannel *mainChannel, QLCChannel *actsOnChannel)
{
    m_actsOnChannelsList.insert(mainChannel, actsOnChannel);
}

QHash<QLCChannel *, QLCChannel *> QLCFixtureMode::actsOnChannelsList() const
{
    return m_actsOnChannelsList;
}

/*****************************************************************************
 * Heads
 *****************************************************************************/

void QLCFixtureMode::insertHead(int index, const QLCFixtureHead& head)
{
    if (index < 0 || index >= m_heads.size())
        m_heads.append(head);
    else
        m_heads.insert(index, head);
}

void QLCFixtureMode::removeHead(int index)
{
    if (index >= 0 && index < m_heads.size())
        m_heads.remove(index);
}

void QLCFixtureMode::replaceHead(int index, const QLCFixtureHead& head)
{
    if (index >= 0 && index < m_heads.size())
        m_heads[index] = head;
}

QVector <QLCFixtureHead> const& QLCFixtureMode::heads() const
{
    return m_heads;
}

int QLCFixtureMode::headForChannel(quint32 chnum) const
{
    for (int i = 0; i < m_heads.size(); i++)
    {
        if (m_heads[i].channels().contains(chnum) == true)
            return i;
    }

    return -1;
}

void QLCFixtureMode::cacheHeads()
{
    for (int i = 0; i < m_heads.size(); i++)
    {
        QLCFixtureHead& head(m_heads[i]);
        head.cacheChannels(this);
    }

    for (int i = 0; i < m_channels.size(); i++)
    {
        if (m_channels.at(i)->group() == QLCChannel::Intensity &&
            m_channels.at(i)->controlByte() == QLCChannel::MSB &&
            m_channels.at(i)->colour() == QLCChannel::NoColour &&
            headForChannel(i) == -1)
        {
            m_masterIntensityChannel = i;
            break;
        }
    }
}

/****************************************************************************
 * Physical
 ****************************************************************************/

void QLCFixtureMode::setPhysical(const QLCPhysical& physical)
{
    m_useGlobalPhysical = false;
    m_physical = physical;
}

void QLCFixtureMode::resetPhysical()
{
    m_useGlobalPhysical = true;
}

bool QLCFixtureMode::useGlobalPhysical()
{
    return m_useGlobalPhysical;
}

QLCPhysical QLCFixtureMode::physical() const
{
    if (m_useGlobalPhysical)
        return fixtureDef()->physical();

    return m_physical;
}

/****************************************************************************
 * Load & Save
 ****************************************************************************/

bool QLCFixtureMode::loadXML(QXmlStreamReader &doc)
{
    if (doc.name() != KXMLQLCFixtureMode)
    {
        qWarning() << Q_FUNC_INFO << "Mode tag not found";
        return false;
    }

    /* Mode name */
    QString str = doc.attributes().value(KXMLQLCFixtureModeName).toString();
    if (str.isEmpty() == true)
    {
        qWarning() << Q_FUNC_INFO << "Mode has no name";
        return false;
    }
    else
    {
        setName(str);
    }

    /* Temporary list with mode's channels pointer and acts on indexes. */
    QList<ChannelActsOnData> listChannelsWithActsOnIndex;

    /* Subtags */
    while (doc.readNextStartElement())
    {
        if (doc.name() == KXMLQLCFixtureModeChannel)
        {
            /* Channel */
            Q_ASSERT(m_fixtureDef != NULL);
            str = doc.attributes().value(KXMLQLCFixtureModeChannelNumber).toString();

            int actsOnChannelIndex = -1;

            if (doc.attributes().hasAttribute(KXMLQLCFixtureModeChannelActsOn))
            {
                actsOnChannelIndex = doc.attributes().value(KXMLQLCFixtureModeChannelActsOn).toInt();
            }

            QLCChannel *currentChannel = m_fixtureDef->channel(doc.readElementText());

            ChannelActsOnData channelActsData(currentChannel, actsOnChannelIndex);

            listChannelsWithActsOnIndex.append(channelActsData);

            insertChannel(currentChannel,
                          str.toInt());
        }
        else if (doc.name() == KXMLQLCFixtureHead)
        {
            /* Head */
            QLCFixtureHead head;
            if (head.loadXML(doc) == true)
                insertHead(-1, head);
        }
        else if (doc.name() == KXMLQLCPhysical)
        {
            /* Physical */
            QLCPhysical physical;
            physical.loadXML(doc);
            setPhysical(physical);
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown Fixture Mode tag:" << doc.name();
            doc.skipCurrentElement();
        }
    }

    // Set acts on channels

    foreach (ChannelActsOnData channelSctsOnData, listChannelsWithActsOnIndex)
    {
        if(m_channels.contains(channelSctsOnData.channel) &&
                channelSctsOnData.actsOnIndex >= 0 &&
                m_channels.size() > channelSctsOnData.actsOnIndex)
        {
            m_actsOnChannelsList.insert(channelSctsOnData.channel,
                                        m_channels.at(channelSctsOnData.actsOnIndex));
        }
    }

    // Cache all head channels
    cacheHeads();

    return true;
}

bool QLCFixtureMode::saveXML(QXmlStreamWriter *doc)
{
    int i = 0;

    Q_ASSERT(doc != NULL);

    /* Mode entry */
    doc->writeStartElement(KXMLQLCFixtureMode);
    doc->writeAttribute(KXMLQLCFixtureModeName, m_name);

    if (m_useGlobalPhysical == false)
        m_physical.saveXML(doc);

    /* Channels */
    QVectorIterator <QLCChannel*> it(m_channels);
    while (it.hasNext() == true)
    {
        QLCChannel* channel = it.next();

        doc->writeStartElement(KXMLQLCFixtureModeChannel);
        doc->writeAttribute(KXMLQLCFixtureModeChannelNumber, QString::number(i++));

        if (m_actsOnChannelsList.contains(channel))
        {
            QLCChannel *ChannelActsOn = m_actsOnChannelsList.value(channel);
            if(ChannelActsOn != NULL){
                doc->writeAttribute(KXMLQLCFixtureModeChannelActsOn, QString::number(m_channels.indexOf(ChannelActsOn)));
            }
        }

        doc->writeCharacters(channel->name());
        doc->writeEndElement();
    }

    /* Heads */
    QVectorIterator <QLCFixtureHead> hit(m_heads);
    while (hit.hasNext() == true)
        hit.next().saveXML(doc);

    doc->writeEndElement();

    return true;
}

QLCFixtureMode::ChannelActsOnData::ChannelActsOnData(QLCChannel *newChannel, int newAcsOnIndex) :
    channel(newChannel),
    actsOnIndex(newAcsOnIndex)
{}
