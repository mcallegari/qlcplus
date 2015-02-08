/*
  Q Light Controller
  qlcfixturehead.h

  Copyright (C) Heikki Junnila

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
#include <QSet>

class QLCFixtureMode;
class QDomDocument;
class QDomElement;

/** @addtogroup engine Engine
 * @{
 */

#define KXMLQLCFixtureHead          "Head"
#define KXMLQLCFixtureHeadChannel   "Channel"

class QLCFixtureHead
{
public:
    QLCFixtureHead();
    QLCFixtureHead(const QLCFixtureHead& head);
    virtual ~QLCFixtureHead();

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
    QSet <quint32> channels() const;

private:
    QSet <quint32> m_channels;

    /************************************************************************
     * Cached channels
     ************************************************************************/
public:
    /**
     * Get the channel number for pan MSB (8bit).
     * @return The coarse pan channel or QLCChannel::invalid() if not applicable.
     */
    quint32 panMsbChannel() const;

    /**
     * Get the channel number for tilt MSB (16bit).
     * @return The coarse tilt channel or QLCChannel::invalid() if not applicable.
     */
    quint32 tiltMsbChannel() const;

    /**
     * Get the channel number for pan LSB (16bit).
     * @return The fine pan channel or QLCChannel::invalid() if not applicable
     */
    quint32 panLsbChannel() const;

    /**
     * Get the channel number for tilt LSB (16bit).
     * @return The fine tilt channel or QLCChannel::invalid() if not applicable.
     */
    quint32 tiltLsbChannel() const;

    /**
     * Get the master intensity channel. For dimmers this is invalid.
     * @return The master intensity channel or QLCChannel::invalid() if not applicable.
     */
    quint32 masterIntensityChannel() const;

    /**
     * Get a list of RGB channels. If the fixture doesn't support RGB mixing,
     * the list is empty. The first item is always red, then green, then blue.
     * @return A list of three channels or an empty list
     */
    QVector <quint32> rgbChannels() const;

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

protected:
    /** Indicates, whether cacheChannels() has already been called */
    bool m_channelsCached;

    /** The coarse pan channel */
    quint32 m_panMsbChannel;

    /** The coarse tilt channel */
    quint32 m_tiltMsbChannel;

    /** The fine pan channel */
    quint32 m_panLsbChannel;

    /** The fine tilt channel */
    quint32 m_tiltLsbChannel;

    /** The master intensity channel */
    quint32 m_masterIntensityChannel;

    /** The RGB mix intensity channels */
    QVector <quint32> m_rgbChannels;

    /** The CMY mix intensity channels */
    QVector <quint32> m_cmyChannels;

    /** The color wheel channels */
    QVector <quint32> m_colorWheels;

    /** The shutter channels */
    QVector <quint32> m_shutterChannels;

    /************************************************************************
     * Load & Save
     ************************************************************************/
public:
    /** Load a Fixture Head from an XML tag */
    bool loadXML(const QDomElement& root);

    /** Save a Fixture Head to an XML $doc, under $mode_root */
    bool saveXML(QDomDocument* doc, QDomElement* mode_root) const;
};

/**
 * A small specialization class from QLCFixtureHead to be used for each
 * Generic Dimmer channel to make them work as separate heads.
 */
class QLCDimmerHead : public QLCFixtureHead
{
public:
    /**
     * Construct a new QLCDimmerHead with the given $head number representing
     * the relative channel number within the dimmer. The $head number is set
     * as the head's masterIntensityChannel(), with all other cached channels
     * left invalid.
     *
     * @param head The channel/head number to represent
     */
    QLCDimmerHead(int head);
};

/** @} */

#endif
