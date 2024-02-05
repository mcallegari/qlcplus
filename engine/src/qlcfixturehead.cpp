/*
  Q Light Controller Plus
  qlcfixturehead.cpp

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
#include <QDebug>

#include "qlcfixturehead.h"
#include "qlcfixturemode.h"
#include "qlcchannel.h"

QLCFixtureHead::QLCFixtureHead()
    : m_channelsCached(false)
{
}

QLCFixtureHead::QLCFixtureHead(const QLCFixtureHead& head)
    : m_channels(head.m_channels)
    , m_channelsCached(head.m_channelsCached)
    , m_channelsMap(head.m_channelsMap)
    , m_colorWheels(head.m_colorWheels)
    , m_shutterChannels(head.m_shutterChannels)
{
}

QLCFixtureHead::~QLCFixtureHead()
{
}

QLCFixtureHead &QLCFixtureHead::operator=(const QLCFixtureHead &head)
{
    if (this != &head)
    {
        m_channels = head.channels();
        m_channelsMap = head.channelsMap();
        m_colorWheels = head.colorWheels();
        m_shutterChannels = head.shutterChannels();
        m_channelsCached = true;
    }

    return *this;
}

/****************************************************************************
 * Channels
 ****************************************************************************/

void QLCFixtureHead::addChannel(quint32 channel)
{
    if (m_channels.contains(channel) == false)
        m_channels.append(channel);
}

void QLCFixtureHead::removeChannel(quint32 channel)
{
    m_channels.removeAll(channel);
}

QList<quint32> QLCFixtureHead::channels() const
{
    return m_channels;
}

/****************************************************************************
 * Cached channels
 ****************************************************************************/

quint32 QLCFixtureHead::channelNumber(int type, int controlByte) const
{
    quint32 val = m_channelsMap.value(type, 0xFFFFFFFF);

    if (val == 0xFFFFFFFF)
        return QLCChannel::invalid();

    if (controlByte == QLCChannel::MSB)
        val = val >> 16;
    else
        val &= 0x0000FFFF;

    if (val == 0x0000FFFF)
        return QLCChannel::invalid();

    return val;
}

QVector <quint32> QLCFixtureHead::rgbChannels() const
{
    QVector <quint32> vector;
    quint32 r = channelNumber(QLCChannel::Red, QLCChannel::MSB);
    quint32 g = channelNumber(QLCChannel::Green, QLCChannel::MSB);
    quint32 b = channelNumber(QLCChannel::Blue, QLCChannel::MSB);

    if (r != QLCChannel::invalid() && g != QLCChannel::invalid() && b != QLCChannel::invalid())
        vector << r << g << b;

    return vector;
}

QMap<int, quint32> QLCFixtureHead::channelsMap() const
{
    return m_channelsMap;
}

QVector <quint32> QLCFixtureHead::cmyChannels() const
{
    QVector <quint32> vector;
    quint32 c = channelNumber(QLCChannel::Cyan, QLCChannel::MSB);
    quint32 m = channelNumber(QLCChannel::Magenta, QLCChannel::MSB);
    quint32 y = channelNumber(QLCChannel::Yellow, QLCChannel::MSB);

    if (c != QLCChannel::invalid() && m != QLCChannel::invalid() && y != QLCChannel::invalid())
        vector << c << m << y;

    return vector;
}

QVector <quint32> QLCFixtureHead::colorWheels() const
{
    return m_colorWheels;
}

QVector <quint32> QLCFixtureHead::shutterChannels() const
{
    return m_shutterChannels;
}

void QLCFixtureHead::setMapIndex(int chType, int controlByte, quint32 index)
{
    if (index == QLCChannel::invalid())
        return;

    quint32 val = m_channelsMap.value(chType, 0xFFFFFFFF);

    if (controlByte == QLCChannel::MSB)
    {
        val &= 0x0000FFFF;
        val |= ((index << 16) & 0xFFFF0000);
    }
    else if (controlByte == QLCChannel::LSB)
    {
        val &= 0xFFFF0000;
        val |= index;
    }
    m_channelsMap[chType] = val;

    //qDebug() << this << "chtype:" << chType << "control" << controlByte << "index" << index << "val" << QString::number(val, 16);
}

void QLCFixtureHead::cacheChannels(const QLCFixtureMode* mode)
{
    Q_ASSERT(mode != NULL);

    // Allow only one caching round per fixture mode instance
    if (m_channelsCached == true)
        return;

    m_colorWheels.clear();
    m_shutterChannels.clear();
    m_channelsMap.clear();

    foreach (quint32 i, m_channels)
    {
        if ((int)i >= mode->channels().size())
        {
            qDebug() << "Head contains undefined channel" << i;
            continue;
        }

        const QLCChannel* ch = mode->channels().at(i);
        Q_ASSERT(ch != NULL);

        if (ch->group() == QLCChannel::Pan)
        {
            setMapIndex(QLCChannel::Pan, ch->controlByte(), i);
        }
        else if (ch->group() == QLCChannel::Tilt)
        {
            setMapIndex(QLCChannel::Tilt, ch->controlByte(), i);
        }
        else if (ch->group() == QLCChannel::Intensity)
        {
            if (ch->colour() == QLCChannel::NoColour)
            {
                 setMapIndex(QLCChannel::Intensity, ch->controlByte(), i);
            }
            else // all the other colors
            {
                setMapIndex(ch->colour(), ch->controlByte(), i);
            }
        }
        else if (ch->group() == QLCChannel::Colour && ch->controlByte() == QLCChannel::MSB)
        {
            m_colorWheels << i;
        }
        else if (ch->group() == QLCChannel::Shutter && ch->controlByte() == QLCChannel::MSB)
        {
            m_shutterChannels << i;
        }
    }

    // if this head doesn't include any Pan/Tilt channel
    // try to retrieve them from the fixture Mode
    if (channelNumber(QLCChannel::Pan, QLCChannel::MSB) == QLCChannel::invalid())
        setMapIndex(QLCChannel::Pan, QLCChannel::MSB, mode->channelNumber(QLCChannel::Pan, QLCChannel::MSB));
    if (channelNumber(QLCChannel::Pan, QLCChannel::LSB) == QLCChannel::invalid())
        setMapIndex(QLCChannel::Pan, QLCChannel::LSB, mode->channelNumber(QLCChannel::Pan, QLCChannel::LSB));
    if (channelNumber(QLCChannel::Tilt, QLCChannel::MSB) == QLCChannel::invalid())
        setMapIndex(QLCChannel::Tilt, QLCChannel::MSB, mode->channelNumber(QLCChannel::Tilt, QLCChannel::MSB));
    if (channelNumber(QLCChannel::Tilt, QLCChannel::LSB) == QLCChannel::invalid())
        setMapIndex(QLCChannel::Tilt, QLCChannel::LSB, mode->channelNumber(QLCChannel::Tilt, QLCChannel::LSB));

    std::sort(m_colorWheels.begin(), m_colorWheels.end());
    std::sort(m_shutterChannels.begin(), m_shutterChannels.end());

    // Allow only one caching round per head
    m_channelsCached = true;
}

/****************************************************************************
 * Load & Save
 ****************************************************************************/

bool QLCFixtureHead::loadXML(QXmlStreamReader &doc)
{
    if (doc.name() != KXMLQLCFixtureHead)
    {
        qWarning() << Q_FUNC_INFO << "Fixture Head node not found!";
        return false;
    }

    while (doc.readNextStartElement())
    {
        if (doc.name() == KXMLQLCFixtureHeadChannel)
            addChannel(doc.readElementText().toUInt());
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown Head tag:" << doc.name();
            doc.skipCurrentElement();
        }
    }

    return true;
}

bool QLCFixtureHead::saveXML(QXmlStreamWriter *doc) const
{
    Q_ASSERT(doc != NULL);

    doc->writeStartElement(KXMLQLCFixtureHead);

    foreach (quint32 index, m_channels)
        doc->writeTextElement(KXMLQLCFixtureHeadChannel, QString::number(index));

    doc->writeEndElement();

    return true;
}


