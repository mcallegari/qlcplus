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
#include <QObject>

class QXmlStreamWriter;
class QXmlStreamReader;
class QLCInputProfile;
class QString;

/** @addtogroup engine Engine
 * @{
 */

#define KXMLQLCInputChannel             QString("Channel")
#define KXMLQLCInputChannelName         QString("Name")
#define KXMLQLCInputChannelType         QString("Type")
#define KXMLQLCInputChannelNumber       QString("Number")
#define KXMLQLCInputChannelSlider       QString("Slider")
#define KXMLQLCInputChannelKnob         QString("Knob")
#define KXMLQLCInputChannelEncoder      QString("Encoder")
#define KXMLQLCInputChannelButton       QString("Button")
#define KXMLQLCInputChannelPageUp       QString("Next Page")
#define KXMLQLCInputChannelPageDown     QString("Previous Page")
#define KXMLQLCInputChannelPageSet      QString("Page Set")
#define KXMLQLCInputChannelNone         QString("None")
#define KXMLQLCInputChannelMovement     QString("Movement")
#define KXMLQLCInputChannelRelative     QString("Relative")
#define KXMLQLCInputChannelSensitivity  QString("Sensitivity")
#define KXMLQLCInputChannelExtraPress   QString("ExtraPress")
#define KXMLQLCInputChannelFeedback     QString("Feedback")
#define KXMLQLCInputChannelLowerValue   QString("LowerValue")
#define KXMLQLCInputChannelUpperValue   QString("UpperValue")
#define KXMLQLCInputChannelMidiChannel  QString("MidiChannel")

class QLCInputChannel : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(QLCInputChannel)

    Q_PROPERTY(Type type READ type WRITE setType NOTIFY typeChanged FINAL)
    Q_PROPERTY(QString typeString READ typeString CONSTANT)
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged FINAL)

    Q_PROPERTY(bool sendExtraPress READ sendExtraPress WRITE setSendExtraPress NOTIFY sendExtraPressChanged FINAL)
    Q_PROPERTY(MovementType movementType READ movementType WRITE setMovementType NOTIFY movementTypeChanged FINAL)
    Q_PROPERTY(int movementSensitivity READ movementSensitivity WRITE setMovementSensitivity NOTIFY movementSensitivityChanged FINAL)
    Q_PROPERTY(uchar lowerValue READ lowerValue WRITE setLowerValue NOTIFY lowerValueChanged FINAL)
    Q_PROPERTY(uchar upperValue READ upperValue WRITE setUpperValue NOTIFY upperValueChanged FINAL)

    /********************************************************************
     * Initialization
     ********************************************************************/
public:
    /** Standard constructor */
    QLCInputChannel();

    /** Copy constructor */
    //QLCInputChannel(const QLCInputChannel& channel);
    QLCInputChannel *createCopy();

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
#if QT_VERSION >= 0x050500
    Q_ENUM(Type)
#endif

    /** Set the type of this channel (see enum Type) */
    void setType(Type type);

    /** Get the type of this channel */
    Type type() const;

    /** Convert the given QLCInputChannel::Type to a QString */
    static QString typeToString(Type type);
    QString typeString();

    /** Convert the given QString to a QLCInputChannel::Type */
    static Type stringToType(const QString& type);

    /** Get a list of available channel types */
    static QStringList types();

    /** Get icon for a type */
    static QIcon typeToIcon(Type type);

    /** Get icon for a type */
    static QIcon stringToIcon(const QString& str);

    Q_INVOKABLE static QString iconResource(QLCInputChannel::Type type, bool svg = false);

    QIcon icon() const;

signals:
    void typeChanged();

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

signals:
    void nameChanged();

protected:
    QString m_name;

    /*********************************************************************
     * Slider/Knob movement behaviour specific methods
     *********************************************************************/
public:
    /** Movement behaviour */
    enum MovementType
    {
        Absolute = 0,
        Relative = 1
    };
#if QT_VERSION >= 0x050500
    Q_ENUM(MovementType)
#endif

    MovementType movementType() const;
    void setMovementType(MovementType type);

    int movementSensitivity() const;
    void setMovementSensitivity(int value);

signals:
    void movementTypeChanged();
    void movementSensitivityChanged();

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
    void setLowerValue(const uchar value);

    uchar upperValue() const;
    void setUpperValue(const uchar value);

    int lowerChannel() const;
    void setLowerChannel(const int channel);

    int upperChannel() const;
    void setUpperChannel(const int channel);

signals:
    void sendExtraPressChanged();
    void lowerValueChanged();
    void upperValueChanged();
    void midiChannelChanged();

protected:
    bool m_sendExtraPress;
    uchar m_lowerValue, m_upperValue;
    int m_lowerChannel, m_upperChannel;

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
