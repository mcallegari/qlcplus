/*
  Q Light Controller
  fixture.h

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

#ifndef FIXTURE_H
#define FIXTURE_H

#include <QObject>
#include <QList>
#include <QIcon>
#include <QHash>

#include "qlcchannel.h"

class QDomDocument;
class QDomElement;
class QString;

class QLCFixtureDefCache;
class ChannelModifier;
class QLCFixtureMode;
class QLCFixtureHead;
class FixtureConsole;
class QLCFixtureDef;
class Doc;

/** @addtogroup engine Engine
 * @{
 */

#define KXMLFixture "Fixture"
#define KXMLFixtureName "Name"
#define KXMLFixtureUniverse "Universe"
#define KXMLFixtureAddress "Address"
#define KXMLFixtureID "ID"
#define KXMLFixtureGeneric "Generic"
#define KXMLFixtureRGBPanel "RGBPanel"
#define KXMLFixtureChannels "Channels"
#define KXMLFixtureDimmer "Dimmer"
#define KXMLFixtureExcludeFade "ExcludeFade"
#define KXMLFixtureForcedHTP "ForcedHTP"
#define KXMLFixtureForcedLTP "ForcedLTP"

#define KXMLFixtureChannelModifier "Modifier"
#define KXMLFixtureChannelIndex "Channel"
#define KXMLFixtureModifierName "Name"

class Fixture : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(Fixture)

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    /** Create a new fixture instance with the given QObject parent. */
    Fixture(QObject* parent);

    /** Destructor */
    ~Fixture();

    /** Less-than operator for qSort() */
    bool operator<(const Fixture& fxi);

signals:
    /** Emitted whenever a Fixture property is changed */
    void changed(quint32);

    /*********************************************************************
     * Fixture ID
     *********************************************************************/
public:
    /**
     * Change the fixture instance's fixture ID. This is generally VERY
     * dangerous, since all functions (using this fixture) will cease to
     * work unless their fixture references are changed as well. Also, the
     * fixture will have to be moved within Doc's fixture array to reflect
     * the new ID, which is essentially the fixture's position in the array.
     *
     * @param id The new fixture id
     */
    void setID(quint32 id);

    /**
     * Get the fixture instance's fixture ID.
     *
     * @return The instance's fixture ID
     */
    quint32 id() const;

    /**
     * Get the invalid fixture ID (for comparison etc...)
     */
    static quint32 invalidId();

protected:
    /** Fixture ID */
    quint32 m_id;

    /*********************************************************************
     * Name
     *********************************************************************/
public:
    /**
     * Change the fixture instance's friendly name.
     *
     * @param name The new name to assign for the instance
     */
    void setName(const QString& name);

    /**
     * Get the fixture instance's friendly name.
     *
     * @return The instance's friendly name
     */
    QString name() const;

protected:
    /** Friendly name */
    QString m_name;

    /*********************************************************************
     * Fixture type
     *********************************************************************/
public:
    /**
     * Get the fixture's type as a string.
     *
     * @return Fixture type
     */
    QString type();

    /**
     * Check, whether the fixture is a dimmer-type fixture (i.e. without
     * a definition).
     *
     * @return true if the fixture is a dimmer, otherwise false
     */
    bool isDimmer() const;

    /*********************************************************************
     * Universe
     *********************************************************************/
public:
    /**
     * Set the fixture instance's DMX universe
     *
     * @param universe A zero-based DMX universe (i.e. 0-7; not 1-8)
     */
    void setUniverse(quint32 universe);

    /**
     * Get the fixture instance's DMX universe
     *
     * @return A zero-based DMX address (i.e. 0-511; not 1-512)
     */
    quint32 universe() const;

    /*********************************************************************
     * Address
     *********************************************************************/
public:
    /**
     * Set the fixture instance's DMX address
     *
     * @param address A zero-based DMX address (i.e. 0-511; not 1-512)
     */
    void setAddress(quint32 address);

    /**
     * Get the fixture instance's DMX address
     *
     * @return A zero-based DMX address (i.e. 0-511; not 1-512)
     */
    quint32 address() const;

public:
    /**
     * Get the fixture instance's DMX address & universe as one
     *
     * @return The fixture's address & universe
     */
    quint32 universeAddress() const;

    /*********************************************************************
     * Channels
     *********************************************************************/
public:
    /**
     * Change the number of channels. Valid only for generic dimmers, whose
     * $fixtureDef == NULL && $fixtureMode == NULL.
     *
     * @param channels The new number of channels
     */
    void setChannels(quint32 channels);

    /**
     * Get the number of channels occupied by this fixture instance.
     * This takes also the selected mode into account, as different modes
     * can have different channel sets.
     *
     * @return Number of channels
     */
    quint32 channels() const;

    /**
     * Get a specific channel object by the channel's number. For generic
     * dimmers, the returned QLCChannel is the same for all channel numbers.
     *
     * @param channel The channel number to get
     * @return A QLCChannel* instance that should not be modified
     */
    const QLCChannel* channel(quint32 channel) const;

    /**
     * Get a fixture's channel's DMX address.
     *
     */
    quint32 channelAddress(quint32 channel) const;

    /**
     * Get a channel from the given group of channels and by its primary color
     *
     * @param group Group name of the channel
     * @param colour Primary color to search for
     * @return The first matching channel number
     */
    quint32 channel(QLCChannel::Group group,
        QLCChannel::PrimaryColour color = QLCChannel::NoColour) const;

    /**
     * Get a set of channels from the given group of channels and by their primary color
     *
     * @param group Group name of the channel
     * @param color Primary color to search for
     * @return A QSet containing the matching channel numbers
     */
    QSet <quint32> channels(
                    QLCChannel::Group group,
                    QLCChannel::PrimaryColour color = QLCChannel::NoColour) const;

    /** @see QLCFixtureHead */
    quint32 panMsbChannel(int head = 0) const;

    /** @see QLCFixtureHead */
    quint32 tiltMsbChannel(int head = 0) const;

    /** @see QLCFixtureHead */
    quint32 panLsbChannel(int head = 0) const;

    /** @see QLCFixtureHead */
    quint32 tiltLsbChannel(int head = 0) const;

    /** @see QLCFixtureHead */
    quint32 masterIntensityChannel(int head = 0) const;

    /** @see QLCFixtureHead */
    QVector <quint32> rgbChannels(int head = 0) const;

    /** @see QLCFixtureHead */
    QVector <quint32> cmyChannels(int head = 0) const;

    /** Set a list of channel indices to exclude from fade transitions */
    void setExcludeFadeChannels(QList<int> indices);

    /** Get the list of channel indices to exclude from fade transitions */
    QList<int> excludeFadeChannels();

    /** Add a channel index to exclude from fade transitions */
    void setChannelCanFade(int idx, bool canFade);

    /** Check if a channel can be faded or not */
    bool channelCanFade(int index);

    /** Set a list of channel indices that are forced to be HTP */
    void setForcedHTPChannels(QList<int> indices);

    /** Get a list of channel indices that are forced to be HTP */
    QList<int> forcedHTPChannels();

    /** Set a list of channel indices that are forced to be LTP */
    void setForcedLTPChannels(QList<int> indices);

    /** Get a list of channel indices that are forced to be LTP */
    QList<int> forcedLTPChannels();

    /** Set a ChannelModifier to the channel with the given $idx */
    void setChannelModifier(quint32 idx, ChannelModifier *mod);

    /** Get the ChannelModifier for the channel with the given $idx.
     *  Returns NULL if no modifier has been assigned */
    ChannelModifier *channelModifier(quint32 idx);

protected:
    /** Create a generic intensity channel */
    void createGenericChannel();

    /** Find and store channel numbers (pan, tilt, intensity) */
    void findChannels();

protected:
    /** DMX address & universe */
    quint32 m_address;

    /** Number of channels (ONLY for dimmer fixtures!) */
    quint32 m_channels;

    /** Generic intensity channel for dimmer fixtures */
    QLCChannel* m_genericChannel;

    /** List holding the channels indices to exlude from fade transitions */
    QList<int> m_excludeFadeIndices;

    /** List holding the LTP channels indices that are forced to be HTP */
    QList<int> m_forcedHTPIndices;

    /** List holding the HTP channels indices that are forced to be LTP */
    QList<int> m_forcedLTPIndices;

    /** Hash holding the pair <channel index, modifier pointer>
     *  This is basically the place to store them to be saved/loaded
     *  on the project XML file */
    QHash<quint32, ChannelModifier*> m_channelModifiers;

    /*********************************************************************
     * Fixture definition
     *********************************************************************/
public:
    /**
     * Change the fixture's definition & mode. Use this when changing an
     * existing fixture to use another def.
     *
     * @param fixtureDef The new fixture definition
     * @param fixtureMode The new fixture mode (member of $fixtureDef)
     */
    void setFixtureDefinition(QLCFixtureDef *fixtureDef,
                              QLCFixtureMode *fixtureMode);

    /**
     * Get the fixture definition that this fixture instance is based on.
     *
     * @return A QLCFixture definition
     */
    QLCFixtureDef *fixtureDef() const;

    /**
     * Get the fixture mode that this fixture instance is based on.
     *
     * @return A QLCFixtureMode definition
     */
    QLCFixtureMode* fixtureMode() const;

    /**
     * Return the number of heads used by the fixture. If the fixture is a
     * generic dimmer, this returns the number of channels (assuming each one
     * controls one lamp == head). Otherwise returns the number of heads defined
     * in fixtureMode().
     *
     * @return Number of heads
     */
    int heads() const;

    /**
     * Get the fixture head at the given index. If $index is invalid, returns NULL.
     * Each fixture has at least one head. Dimmer fixtures have no heads since each
     * channel can be treated as a head.
     *
     * @param index The index of the head to return
     * @return The head at the given index or NULL
     */
    QLCFixtureHead head(int index) const;

    QIcon getIconFromType(QString type) const;

    QRectF degreesRange(int head) const;

protected:
    /** The fixture definition that this instance is based on */
    QLCFixtureDef* m_fixtureDef;

    /** The mode within the fixture definition that this instance uses */
    QLCFixtureMode* m_fixtureMode;

    /*********************************************************************
     * Generic RGB panel
     *********************************************************************/
public:
    /** Creates and returns a definition for a generic RGB panel row */
    QLCFixtureDef *genericRGBPanelDef(int columns);

    /** Creates and returns a fixture mode for a generic RGB panel row */
    QLCFixtureMode *genericRGBPanelMode(QLCFixtureDef *def, quint32 width, quint32 height);

    /*********************************************************************
     * Load & Save
     *********************************************************************/
public:
    /**
     * Load a fixture from the given XML node and attempt to add it to
     * the given QLC Doc instance.
     *
     * @param root The Fixture node to load from
     * @param doc The doc that owns all fixtures
     */
    static bool loader(const QDomElement& root, Doc* doc);

    /**
     * Load a fixture's contents from the given XML node.
     *
     * @param root An XML subtree containing a single fixture instance
     * @return true if the fixture was loaded successfully, otherwise false
     */
    bool loadXML(const QDomElement& root, Doc* doc,
                 const QLCFixtureDefCache* fixtureDefCache);

    /**
     * Save the fixture instance into an XML document, under the given
     * XML element (tag).
     *
     * @param doc The master XML document to save to.
     * @param wksp_root The workspace root element
     */
    bool saveXML(QDomDocument* doc, QDomElement* wksp_root) const;

    /*********************************************************************
     * Status
     *********************************************************************/
public:
    /**
     * Get the fixture instance's status info for Fixture Manager
     *
     * @return A sort-of HTML-RTF-gibberish for Fixture Manager
     */
    QString status() const;
};

/** @} */

#endif

