/*
  Q Light Controller
  qlcinputchannel.h

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

#ifndef QLCINPUTCHANNEL_H
#define QLCINPUTCHANNEL_H

#include <QIcon>

class QXmlStreamWriter;
class QXmlStreamReader;
class QLCInputProfile;
class QString;

/** @addtogroup engine Engine
 * @{
 */

#define KXMLQLCInputChannel         "Channel"
#define KXMLQLCInputChannelName     "Name"
#define KXMLQLCInputChannelType     "Type"
#define KXMLQLCInputChannelNumber   "Number"
#define KXMLQLCInputChannelSlider   "Slider"
#define KXMLQLCInputChannelKnob     "Knob"
#define KXMLQLCInputChannelEncoder  "Encoder"
#define KXMLQLCInputChannelButton   "Button"
#define KXMLQLCInputChannelPageUp   "Next Page"
#define KXMLQLCInputChannelPageDown "Previous Page"
#define KXMLQLCInputChannelPageSet  "Page Set"
#define KXMLQLCInputChannelNone     "None"
#define KXMLQLCInputChannelMovement "Movement"
#define KXMLQLCInputChannelRelative "Relative"
#define KXMLQLCInputChannelSensitivity "Sensitivity"
#define KXMLQLCInputChannelExtraPress "ExtraPress"
#define KXMLQLCInputChannelFeedbacks "Feedbacks"
#define KXMLQLCInputChannelLowerValue "LowerValue"
#define KXMLQLCInputChannelUpperValue "UpperValue"

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
        Encoder,
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

    /** Get icon for a type */
    static QIcon typeToIcon(Type type);

    /** Get icon for a type */
    static QIcon stringToIcon(const QString& str);

    QIcon icon() const;

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

    /*********************************************************************
     * Slider/Knob movement behaviour specific methods
     *********************************************************************/
public:
    /** Movement behaviour */
    enum MovementType {
        Absolute = 0,
        Relative = 1
    };

    MovementType movementType() const;
    void setMovementType(MovementType type);

    int movementSensitivity() const;
    void setMovementSensitivity(int value);

protected:
    MovementType m_movementType;
    int m_movementSensitivity;

    /*********************************************************************
     * Button behaviour specific methods
     *********************************************************************/
public:
    void setSendExtraPress(bool enable);
    bool sendExtraPress() const;
    void setRange(uchar lower, uchar upper);
    uchar lowerValue() const;
    uchar upperValue() const;

protected:
    bool m_sendExtraPress;
    uchar m_lower, m_upper;

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
    bool loadXML(QXmlStreamReader &root);

    /**
     * Save this channel's contents to the given XML document, setting the
     * given channel number as the channel's number.
     *
     * @param doc The master XML document to save to
     * @param channelNumber The channel's number in the channel map
     * @return true if successful, otherwise false
     */
    bool saveXML(QXmlStreamWriter *doc, quint32 channelNumber) const;
};

/** @} */

#endif
