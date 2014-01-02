/*
  Q Light Controller
  qlcfixturemode.h

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

#ifndef QLCFIXTUREMODE_H
#define QLCFIXTUREMODE_H

#include <QString>
#include <QList>

#include "qlcfixturehead.h"
#include "qlcfixturedef.h"
#include "qlcphysical.h"
#include "qlcchannel.h"

class QDomDocument;
class QDomElement;
class QLCFixtureHead;
class QLCFixtureMode;
class QLCFixtureDef;
class QLCPhysical;
class QLCChannel;

/** @addtogroup engine Engine
 * @{
 */

#define KXMLQLCFixtureMode              "Mode"
#define KXMLQLCFixtureModeName          "Name"
#define KXMLQLCFixtureModeChannel       "Channel"
#define KXMLQLCFixtureModeChannelNumber "Number"

/**
 * QLCFixtureMode is essentially a collection of QLCChannels, arranged in such
 * order that they represent the channel configuration of an actual fixture.
 * Damn that sentence was hard to formulate... In many simple cases, fixtures
 * have only one mode and therefore a separate QLCFixtureMode is rather useless.
 * However, since many fixtures DO use different modes (16bit & 8bit movement
 * modes have a different number of channels etc..) this class is very much
 * needed to prevent the need for having to create a separate definition files
 * for each different mode. To ease the user in selecting the proper mode,
 * each mode can also have a name.
 *
 * Since fixture modes represent different settings for a fixture, it is only
 * natural to assume that also the physical properties of a fixture can be
 * changed. Therefore, each QLCFixtureMode contains also a QLCPhysical object
 * that defines the physical properties of a fixture in a certain mode.
 *
 * QLCFixtureDef owns the channel instances and deletes them when it is deleted
 * itself. QLCFixtureModes do not delete their channels because they might be
 * shared between multiple modes.
 */
class QLCFixtureMode
{
public:
    /**
     * Create a new QLCFixtureMode for the given QLCFixtureDef. Added
     * channels must belong to the fixture definition.
     *
     * @param fixtureDef The parent fixture definition
     */
    QLCFixtureMode(QLCFixtureDef* fixtureDef);

    /**
     * Create a copy of the given mode, taking channels from the given
     * parent fixture definition.
     *
     * @param fixtureDef The parent fixture definition, who owns channels
     *                   that belong to this mode.
     * @param mode The mode to copy
     */
    QLCFixtureMode(QLCFixtureDef* fixtureDef, const QLCFixtureMode* mode);

    /** Destructor */
    virtual ~QLCFixtureMode();

    /** Assignment operator */
    QLCFixtureMode& operator=(const QLCFixtureMode& mode);

    /*********************************************************************
     * Name
     *********************************************************************/
public:
    /** Set the name of the fixture mode */
    void setName(const QString &name);

    /** Get the name of the fixture mode */
    QString name() const;

protected:
    QString m_name;

    /*********************************************************************
     * Fixture definition
     *********************************************************************/
public:
    /** Get the fixture that this mode is associated to */
    QLCFixtureDef* fixtureDef() const;

protected:
    QLCFixtureDef* m_fixtureDef;

    /*********************************************************************
     * Channels
     *********************************************************************/
public:
    /**
     * Insert a channel at the given position. The channel must belong
     * to m_fixtureDef or it won't get added to the mode. Each channel can
     * occupy exactly one index in a mode.
     *
     * @param channel The channel to add
     * @param index The position to insert the channel at
     * @return true, if successful, otherwise false
     */
    bool insertChannel(QLCChannel* channel, quint32 index);

    /**
     * Remove a channel from this mode. The channel is only removed from
     * m_channels list, but it's not deleted, since it might be used by
     * other modes, and in any case, the fixtureDef owns the channel.
     *
     * @param channel The channel to remove
     * @return true if the channel was found and removed. Otherwise false.
     */
    bool removeChannel(const QLCChannel* channel);

    /**
     * Remove all channels from this mode. The channels are only removed from
     * m_channels list, but they are not deleted, since they might be used by
     * other modes, and in any case, the fixtureDef owns the channel.
     */
    void removeAllChannels();

    /**
     * Get a channel by its name. If there are more than one channels with
     * the same name in a mode, only the first one is returned (although
     * channel names should be unique and this should never happen).
     *
     * @param name The name of the channel to get
     * @return The channel or NULL if not found
     */
    QLCChannel* channel(const QString& name) const;

    /**
     * Get a channel by its index (channel number). One DMX channel is
     * represented by exactly one QLCChannel.
     *
     * @param ch The number of the channel to get
     * @return The channel or NULL if ch >= size.
     */
    QLCChannel* channel(quint32 ch) const;

    /**
     * Get an ordered list of channels in a mode. Returns a copy of the list;
     * Any modifications to the list won't end up in the mode, but
     * modifications to channels are possible (discouraged).
     *
     * @return A list of channels in the mode.
     */
    QList <QLCChannel*> channels() const;

    /**
     * Get a channel's index (i.e. the DMX channel number) within a mode.
     * If the channel is not part of the mode, QLCChannel::invalid() is returned.
     *
     * @param channel The channel, whose number to get
     * @return Channel number or QLCChannel::invalid()
     */
    quint32 channelNumber(QLCChannel* channel) const;

protected:
    /** List of channels (pointers are not owned) */
    QList <QLCChannel*> m_channels;

    /*********************************************************************
     * Heads
     *********************************************************************/
public:
    /**
     * Insert a head at the given position within the fixture mode.
     *
     * @param index The index to insert the head at (if invalid, append occurs)
     * @param head The head to insert
     */
    void insertHead(int index, const QLCFixtureHead& head);

    /**
     * Replace a head at the given position with the given head.
     *
     * @param index The index to replace the head at (must be valid)
     * @param head The head to replace
     */
    void replaceHead(int index, const QLCFixtureHead& head);

    /**
     * Remove a head at the given index.
     *
     * @param index The index of the head to remove
     */
    void removeHead(int index);

    /**
     * Get a list of available fixture heads within the fixture mode
     */
    QList <QLCFixtureHead> heads() const;

    /**
     * Find a head number for the given channel number
     *
     * @param chnum The number of the channel whose head to find
     * @return The head number of -1 if the channel doesn't belong to any head
     */
    int headForChannel(quint32 chnum) const;

    /**
     * Cache all heads' channels
     */
    void cacheHeads();

private:
    QList <QLCFixtureHead> m_heads;

    /*********************************************************************
     * Physical
     *********************************************************************/
public:
    /**
     * Set a mode's physical properties.
     *
     * @param physical Properties to set
     */
    void setPhysical(const QLCPhysical& physical);

    /**
     * Get physical properties for a mode. The returned properties is a
     * copy of the original, so any modifications won't end up in the mode.
     *
     * @return Mode's physical properties
     */
    QLCPhysical physical() const;

protected:
    QLCPhysical m_physical;

    /*********************************************************************
     * Physical
     *********************************************************************/
public:
    /** Load a mode's properties from an XML tag */
    bool loadXML(const QDomElement& root);

    /** Save a mode to an XML document */
    bool saveXML(QDomDocument* doc, QDomElement* root);
};

/** @} */

#endif
