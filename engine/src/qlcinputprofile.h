/*
  Q Light Controller
  qlcinputprofile.h

  Copyright (c) Heikki Junnila

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

#ifndef QLCINPUTPROFILE_H
#define QLCINPUTPROFILE_H

#include <QStringList>
#include <QVariant>
#include <QObject>
#include <QString>
#include <QHash>
#include <QMap>

class QLCInputChannel;
class QLCInputProfile;
class QXmlStreamReader;

/** @addtogroup engine Engine
 * @{
 */

#define KXMLQLCInputProfile                 QString("InputProfile")
#define KXMLQLCInputProfileManufacturer     QString("Manufacturer")
#define KXMLQLCInputProfileModel            QString("Model")
#define KXMLQLCInputProfileType             QString("Type")
#define KXMLQLCInputProfileMidiSendNoteOff  QString("MIDISendNoteOff")
#define KXMLQLCInputProfileColorTable       QString("ColorTable")
#define KXMLQLCInputProfileColor            QString("Color")
#define KXMLQLCInputProfileMidiChannelTable QString("MidiChannelTable")
#define KXMLQLCInputProfileMidiChannel      QString("Channel")

class QLCInputProfile : public QObject
{
    Q_OBJECT

    /********************************************************************
     * Initialization
     ********************************************************************/
public:
    /** Standard constructor */
    QLCInputProfile();

    /** Destructor */
    virtual ~QLCInputProfile();

    QLCInputProfile *createCopy();

    /** Assignment operator */
    QLCInputProfile& operator=(const QLCInputProfile& profile);

    /********************************************************************
     * Profile information
     ********************************************************************/
public:
    void setManufacturer(const QString& manufacturer);
    QString manufacturer() const;

    void setModel(const QString& model);
    QString model() const;

    /** Get the profile name (manufacturer - model) */
    QString name() const;

    /** Get the path where the profile is stored in. Don't use
        this as a unique ID since this varies between platforms. */
    void setPath(QString path);
    QString path() const;

    enum Type
    {
        MIDI = 0,
        OS2L,
        OSC,
        HID,
        DMX,
        Enttec,
    };
#if QT_VERSION >= 0x050500
    Q_ENUM(Type)
#endif

    void setType(Type type);

    Type type() const;

    static QString typeToString(Type type);

    static Type stringToType(const QString & str);

    static QList<Type> types();

protected:
    QString m_manufacturer;
    QString m_model;
    QString m_path;
    Type m_type;

    /********************************************************************
     * Plugin-specific global settings
     ********************************************************************/
public:
    /** Instruct the MIDI plugin not to send Note Off on
     *  a value equal to 0. Instead, send a Note On with 0 velocity */
    void setMidiSendNoteOff(bool enable);

    /** Retrieve the MIDI Note Off flag */
    bool midiSendNoteOff() const;

    QMap<QString, QVariant> globalSettings() const;

protected:
    bool m_midiSendNoteOff;

    QMap<QString, QVariant> m_globalSettingsMap;

    /********************************************************************
     * Channels
     ********************************************************************/
public:
    /**
     * Insert a new channel to this profile to the given channel number
     * and claim ownership of the channel. If the profile already contains
     * the given channel, the call fails.
     *
     * @param channel The channel number to add to.
     * @param ich The input channel to add.
     * @return true if the channel was inserted, otherwise false.
     */
    bool insertChannel(quint32 channel, QLCInputChannel* ich);

    /**
     * Remove the given channel mapping from this profile. Also deletes the
     * channel instance.
     *
     * @param channel The channel number to remove & delete.
     */
    bool removeChannel(quint32 channel);

    /**
     * Re-map the given channel to a different channel number. If another
     * channel is already at the new channel number or the given input
     * channel object is not a member of the profile, this method fails.
     *
     * @param ich The input channel to re-map.
     * @param number The new channel number to re-map to.
     * @return true if successful, otherwise false.
     */
    bool remapChannel(QLCInputChannel* ich, quint32 number);

    /**
     * Get a channel object by a channel number.
     *
     * @param channel The number of the channel to get.
     * @return A QLCInputChannel* or NULL if not found.
     */
    QLCInputChannel *channel(quint32 channel) const;

    /**
     * Get the channel number for the given input channel.
     *
     * @param channel The channel whose number to get
     * @return Channel number or InputMap::invalidChannel() if not found
     */
    quint32 channelNumber(const QLCInputChannel* channel) const;

    /**
     * Get available channels.
     */
    QMap <quint32,QLCInputChannel*> channels() const;

    /**
     *  Retrieve additional parameters to be passed to plugins
     *  when sending feedback.
     */
    QVariant channelExtraParams(const QLCInputChannel *channel) const;

private:
    /** Delete and remove all channels */
    void destroyChannels();

protected:
    /** Channel objects present in this profile. This is a QMap and not a
        QList because not all channels might be present. */
    QMap <quint32, QLCInputChannel*> m_channels;

    /********************************************************************
     * Color Translation Table
     ********************************************************************/
public:
    bool hasColorTable();
    void addColor(uchar value, QString label, QColor color);
    void removeColor(uchar value);

    QMap<uchar, QPair<QString, QColor>> colorTable();

protected:
    QMap<uchar, QPair<QString, QColor>> m_colorTable;

    /********************************************************************
     * MIDI Channel table
     ********************************************************************/
public:
    bool hasMidiChannelTable();
    void addMidiChannel(uchar channel, QString label);
    void removeMidiChannel(uchar channel);

    QMap<uchar, QString> midiChannelTable();

protected:
    QMap<uchar, QString> m_midiChannelTable;

    /********************************************************************
     * Load & Save
     ********************************************************************/
public:
    /** Load an input profile from the given path */
    static QLCInputProfile* loader(const QString& path);

    /** Save an input profile into a given file name */
    bool saveXML(const QString& fileName);

    /** Load an optional color table for RGB LED feedback */
    bool loadColorTableXML(QXmlStreamReader &tableRoot);

    /** Load an optional MIDI channel table */
    bool loadMidiChannelTableXML(QXmlStreamReader &tableRoot);

    /** Load an input profile from the given document */
    bool loadXML(QXmlStreamReader &doc);
};

/** @} */

#endif
