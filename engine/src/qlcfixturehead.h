/*
  Q Light Controller
  qlcfixturehead.h

  Copyright (C) Heikki Junnila

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  Version 2 as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details. The license is
  in the file "COPYING".

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef QLCFIXTUREHEAD_H
#define QLCFIXTUREHEAD_H

#include <QList>
#include <QSet>

#define KXMLQLCFixtureHead          "Head"
#define KXMLQLCFixtureHeadChannel   "Channel"

class QLCFixtureMode;
class QDomDocument;
class QDomElement;

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
    QList <quint32> rgbChannels() const;

    /**
     * Get a list of CMY channels. If the fixture doesn't support CMY mixing,
     * the list is empty. The first item is always cyan, then magenta, then yellow.
     * @return A list of three channels or an empty list
     */
    QList <quint32> cmyChannels() const;

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
    QList <quint32> m_rgbChannels;

    /** The CMY mix intensity channels */
    QList <quint32> m_cmyChannels;

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

#endif
