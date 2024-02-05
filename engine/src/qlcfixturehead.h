/*
  Q Light Controller Plus
  qlcfixturehead.h

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

#ifndef QLCFIXTUREHEAD_H
#define QLCFIXTUREHEAD_H

#include <QList>
#include <QMap>

class QLCFixtureMode;
class QXmlStreamReader;
class QXmlStreamWriter;

/** @addtogroup engine Engine
 * @{
 */

#define KXMLQLCFixtureHead          QString("Head")
#define KXMLQLCFixtureHeadChannel   QString("Channel")

class QLCFixtureHead
{
public:
    QLCFixtureHead();
    QLCFixtureHead(const QLCFixtureHead& head);
    virtual ~QLCFixtureHead();

    QLCFixtureHead& operator=(const QLCFixtureHead& head);

    /************************************************************************
     * Channels
     ************************************************************************/
public:
    /**
     * Add a channel to a Fixture Head. The channel must exist in the
     * Fixture Mode that owns the head. A channel number can exist only once
     * per head.
     *
     * @param channel The channel number to add
     */
    void addChannel(quint32 channel);

    /**
     * Remove a channel from a Fixture Head.
     *
     * @param channel The channel number to remove
     */
    void removeChannel(quint32 channel);

    /** Get all channels used by the head */
    QList <quint32> channels() const;

private:
    QList <quint32> m_channels;

    /************************************************************************
     * Cached channels
     ************************************************************************/
public:
    /**
     * Get the channel number for the specified channel $type and $controlByte
     * @return The channel number or QLCChannel::invalid() if not applicable.
     */
    quint32 channelNumber(int type, int controlByte) const;

    /**
     * Get a list of RGB channels. If the fixture doesn't support RGB mixing,
     * the list is empty. The first item is always red, then green, then blue.
     * @return A list of three channels or an empty list
     */
    QVector <quint32> rgbChannels() const;

    /**
     * Return a copy of the cached channel map
     */
    QMap<int, quint32> channelsMap() const;

    /**
     * Get a list of CMY channels. If the fixture doesn't support CMY mixing,
     * the list is empty. The first item is always cyan, then magenta, then yellow.
     * @return A list of three channels or an empty list
     */
    QVector <quint32> cmyChannels() const;

    /**
     * Get a list of color wheel channels. Channels are ordered by their number
     * @return A list of zero or more channels
     */
    QVector <quint32> colorWheels() const;

    /**
     * Get a list of shutter channels. Channels are ordered by their number
     * @return A list of zero or more channels
     */
    QVector <quint32> shutterChannels() const;

    /** Find some interesting channels from $mode and store their indices. */
    void cacheChannels(const QLCFixtureMode* mode);

private:
    void setMapIndex(int chType, int controlByte,  quint32 index);

protected:
    /** Indicates, whether cacheChannels() has already been called */
    bool m_channelsCached;

    /** A map of the cached channel indices, organized as follows:
     *  <int> channel type: @see QLCChannel::Group and QLCChannel::PrimaryColour
     *  <quint32> channel MSB index << 16 | channel LSB index
     */
    QMap<int, quint32> m_channelsMap;

    /** The color wheel channels */
    QVector <quint32> m_colorWheels;

    /** The shutter channels */
    QVector <quint32> m_shutterChannels;

    /************************************************************************
     * Load & Save
     ************************************************************************/
public:
    /** Load a Fixture Head from an XML tag */
    bool loadXML(QXmlStreamReader &doc);

    /** Save a Fixture Head to an XML $doc */
    bool saveXML(QXmlStreamWriter *doc) const;
};

/** @} */

#endif
