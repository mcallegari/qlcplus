/*
  Q Light Controller
  qlcinputchannel.h

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

#ifndef QLCINPUTCHANNEL_H
#define QLCINPUTCHANNEL_H


class QLCInputProfile;
class QDomDocument;
class QDomElement;
class QString;

#define KXMLQLCInputChannel "Channel"
#define KXMLQLCInputChannelName "Name"
#define KXMLQLCInputChannelType "Type"
#define KXMLQLCInputChannelNumber "Number"
#define KXMLQLCInputChannelSlider "Slider"
#define KXMLQLCInputChannelKnob "Knob"
#define KXMLQLCInputChannelButton "Button"
#define KXMLQLCInputChannelPageUp "Next Page"
#define KXMLQLCInputChannelPageDown "Previous Page"
#define KXMLQLCInputChannelPageSet "Page Set"
#define KXMLQLCInputChannelNone "None"

class QLCInputChannel
{
    /********************************************************************
     * Initialization
     ********************************************************************/
public:
    /** Standard constructor */
    QLCInputChannel();

    /** Copy constructor */
    QLCInputChannel(const QLCInputChannel& channel);

    /** Destructor */
    virtual ~QLCInputChannel();

    /********************************************************************
     * Type
     ********************************************************************/
public:
    enum Type
    {
        Slider,
        Knob,
        Button,
        NextPage,
        PrevPage,
        PageSet,
        NoType
    };

    /** Set the type of this channel (see enum Type) */
    void setType(Type type);

    /** Get the type of this channel */
    Type type() const;

    /** Convert the given QLCInputChannel::Type to a QString */
    static QString typeToString(Type type);

    /** Convert the given QString to a QLCInputChannel::Type */
    static Type stringToType(const QString& type);

    /** Get a list of available channel types */
    static QStringList types();

protected:
    Type m_type;

    /********************************************************************
     * Name
     ********************************************************************/
public:
    /** Set the name of this this channel */
    void setName(const QString& name);

    /** Get the name of this channel */
    QString name() const;

protected:
    QString m_name;

    /********************************************************************
     * Load & Save
     ********************************************************************/
public:
    /**
     * Load this channel's contents from the given XML document
     *
     * @param root An input channel tag
     * @return true if successful, otherwise false
     */
    bool loadXML(const QDomElement& root);

    /**
     * Save this channel's contents to the given XML document, setting the
     * given channel number as the channel's number.
     *
     * @param doc The master XML document to save to
     * @param root The input profile root to save under
     * @param channelNumber The channel's number in the channel map
     * @return true if successful, otherwise false
     */
    bool saveXML(QDomDocument* doc, QDomElement* root,
                 quint32 channelNumber) const;
};

#endif
