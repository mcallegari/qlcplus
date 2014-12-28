/*
  Q Light Controller
  qlcchannel.h

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

#ifndef QLC_CHANNEL_H
#define QLC_CHANNEL_H

#include <climits>
#include <QString>
#include <QList>
#include <QIcon>

class QFile;
class QString;
class QDomDocument;
class QDomElement;
class QStringList;
class QLCCapability;
class QLCChannel;

/** @addtogroup engine Engine
 * @{
 */

#define KXMLQLCChannel          QString("Channel")
#define KXMLQLCChannelNumber    QString("Number")
#define KXMLQLCChannelName      QString("Name")
#define KXMLQLCChannelGroup     QString("Group")
#define KXMLQLCChannelGroupByte QString("Byte")
#define KXMLQLCChannelColour    QString("Colour")

/* Compound strings used by PaletteGenerator to identify
 * special fixture modes
 */
#define KQLCChannelMovement  QString("Movement")
#define KQLCChannelRGB       QString("RGB")
#define KQLCChannelCMY       QString("CMY")
#define KQLCChannelWhite     QString("White")

/**
 * QLCChannel represents one DMX channel with one or more DMX value ranges,
 * represented by QLCCapability instances. For example, the DMX channel used to
 * set gobos to a fixture is represented by a QLCChannel that contains one
 * capability for each gobo value range. QLCChannels can also have a name for
 * making it easier for users to understand their purpose, a group to provide
 * additional (and more formal) data about the channel's use (for example pan
 * and tilt for QLC's EFX function) as well as a control byte to signify the
 * bit depth of the channel (16bit pan & tilt, mostly).
 *
 * QLCChannels themselves don't have channel numbers assigned to them, because
 * the same channel might be present at different locations in different
 * fixture modes. Instead, a QLCFixtureMode defines the actual channel number
 * for each of its QLCChannels.
 */
class QLCChannel
{
public:
    /** Standard constructor */
    QLCChannel();

    /** Copy constructor */
    QLCChannel(const QLCChannel* channel);

    /** Destructor */
    ~QLCChannel();

    /** Assignment operator */
    QLCChannel& operator=(const QLCChannel& lc);

    /**
     * The invalid channel number (for comparison etc...)
     */
    static quint32 invalid();

    /*********************************************************************
     * Groups
     *********************************************************************/
public:
    enum Group
    {
        Intensity = 0,
        Colour,
        Gobo,
        Speed,
        Pan,
        Tilt,
        Shutter,
        Prism,
        Beam,
        Effect,
        Maintenance,
        NoGroup = INT_MAX
    };

    /** Get a list of possible channel groups */
    static QStringList groupList();

    /** Convert a Group to a string */
    static QString groupToString(Group grp);

    /** Convert a string to a Group */
    static Group stringToGroup(const QString& str);

    /** Set the channel's group with the Group enum */
    void setGroup(Group grp);

    /** Get the channel's group as an enum */
    Group group() const;

    /** Get the channel's representation icon */
    QIcon getIcon() const;

    /** Get the channel's icon resource name */
    QString getIconNameFromGroup(QLCChannel::Group grp) const;

private:
    QPixmap drawIntensity(QColor color, QString str) const;

    /** Create a colored icon for a specific intensity channel */
    QIcon getIntensityIcon() const;

    /** Get the intensity channel color name */
    QString getIntensityColorCode() const;

protected:
    Group m_group;

    /*********************************************************************
     * Properties
     *********************************************************************/
public:
    enum ControlByte
    {
        MSB = 0,
        LSB = 1
    };

    /** Get the channel's name */
    QString name() const;

    /** Set the channel's name */
    void setName(const QString& name);

    /** Set the channel's control byte */
    void setControlByte(ControlByte byte);

    /** Get the channel's control byte */
    ControlByte controlByte() const;

protected:
    QString m_name;
    ControlByte m_controlByte;

    /*************************************************************************
     * Colours
     *************************************************************************/
public:
    enum PrimaryColour
    {
        NoColour    = 0,
        Red         = 0xFF0000,
        Green       = 0x00FF00,
        Blue        = 0x0000FF,
        Cyan        = 0x00FFFF,
        Magenta     = 0xFF00FF,
        Yellow      = 0xFFFF00,
        Amber       = 0xFF7E00,
        White       = 0xFFFFFF,
        UV          = 0x9400D3
    };

    /** Get a list of possible channel groups */
    static QStringList colourList();

    /** Convert a Group to a string */
    static QString colourToString(PrimaryColour colour);

    /** Convert a string to a Group */
    static PrimaryColour stringToColour(const QString& str);

    /** Set the colour that is controlled by this channel */
    void setColour(PrimaryColour colour);

    /** Get the colour that is controlled by this channel */
    PrimaryColour colour() const;

private:
    PrimaryColour m_colour;

    /*********************************************************************
     * Capabilities
     *********************************************************************/
public:
    /** Get a list of channel's capabilities */
    const QList <QLCCapability*> capabilities() const;

    /** Search for a particular capability by its channel value */
    QLCCapability* searchCapability(uchar value) const;

    /**
     * Search for a particular capability by its name. If exactMatch = true,
     * the first exact match is returned. If exactMatch = false, the
     * first capability whose name _contains_ the given string is
     * returned.
     *
     * @param name The name to search for
     * @param exactMatch if true, only exact matches are returned,
     *                   otherwise a "contains" comparison is made.
     * @return QLCCapability or NULL
     */
    QLCCapability* searchCapability(const QString& name,
                                    bool exactMatch = true) const;

    /** Add a new capability to the channel */
    bool addCapability(QLCCapability* cap);

    /** Remove a capability from the channel */
    bool removeCapability(QLCCapability* cap);

    /** Sort capabilities to ascending order by their values */
    void sortCapabilities();

protected:
    /** List of channel's capabilities */
    QList <QLCCapability*> m_capabilities;

    /*********************************************************************
     * File operations
     *********************************************************************/
public:
    /** Save the channel to a QDomDocument, under the given element */
    bool saveXML(QDomDocument* doc, QDomElement* root) const;

    /** Load channel contents from an XML element */
    bool loadXML(const QDomElement& tag);
};

/** @} */

#endif
