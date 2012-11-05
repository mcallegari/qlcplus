/*
  Q Light Controller
  qlcinputprofile.h

  Copyright (c) Heikki Junnila

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

#ifndef QLCINPUTPROFILE_H
#define QLCINPUTPROFILE_H

#include <QStringList>
#include <QString>
#include <QHash>
#include <QMap>


class QLCInputChannel;
class QLCInputProfile;
class QDomDocument;
class QDomElement;

#define KXMLQLCInputProfile "InputProfile"
#define KXMLQLCInputProfileManufacturer "Manufacturer"
#define KXMLQLCInputProfileModel "Model"

class QLCInputProfile
{
    /********************************************************************
     * Initialization
     ********************************************************************/
public:
    /** Standard constructor */
    QLCInputProfile();

    /** Copy constructor */
    QLCInputProfile(const QLCInputProfile& profile);

    /** Destructor */
    virtual ~QLCInputProfile();

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
    QString path() const;

protected:
    QString m_manufacturer;
    QString m_model;
    QString m_path;

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
    QLCInputChannel* channel(quint32 channel) const;

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

private:
    /** Delete and remove all channels */
    void destroyChannels();

protected:
    /** Channel objects present in this profile. This is a QMap and not a
        QList because not all channels might be present. */
    QMap <quint32,QLCInputChannel*> m_channels;

    /********************************************************************
     * Load & Save
     ********************************************************************/
public:
    /** Load an input profile from the given path */
    static QLCInputProfile* loader(const QString& path);

    /** Save an input profile into a given file name */
    bool saveXML(const QString& fileName);

    /** Load an input profile from the given document */
    bool loadXML(const QDomDocument& doc);
};

#endif
