/*
  Q Light Controller
  qlcinputchannel.cpp

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

#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QString>
#include <QDebug>
#include <QIcon>

#include "qlcinputchannel.h"
#include "qlcinputprofile.h"

/****************************************************************************
 * Initialization
 ****************************************************************************/

QLCInputChannel::QLCInputChannel()
    : m_type(Button)
    , m_movementType(Absolute)
    , m_movementSensitivity(20)
    , m_sendExtraPress(false)
    , m_lower(0)
    , m_upper(UCHAR_MAX)
{
}

QLCInputChannel::QLCInputChannel(const QLCInputChannel& channel)
{
    m_name = channel.m_name;
    m_type = channel.m_type;
    m_movementType = channel.m_movementType;
    m_movementSensitivity = channel.m_movementSensitivity;
    m_sendExtraPress = channel.m_sendExtraPress;
    m_lower = channel.m_lower;
    m_upper = channel.m_upper;
}

QLCInputChannel::~QLCInputChannel()
{
}

/****************************************************************************
 * Type
 ****************************************************************************/

void QLCInputChannel::setType(Type type)
{
    m_type = type;
    if (type == Encoder)
        m_movementSensitivity = 1;
    else
        m_movementSensitivity = 20;
}

QLCInputChannel::Type QLCInputChannel::type() const
{
    return m_type;
}

QString QLCInputChannel::typeToString(Type type)
{
    QString str;

    switch (type)
    {
        case Button:
            str = QString(KXMLQLCInputChannelButton);
        break;
        case Knob:
            str = QString(KXMLQLCInputChannelKnob);
        break;
        case Encoder:
            str = QString(KXMLQLCInputChannelEncoder);
        break;
        case Slider:
            str = QString(KXMLQLCInputChannelSlider);
        break;
        case NextPage:
            str = QString(KXMLQLCInputChannelPageUp);
        break;
        case PrevPage:
            str = QString(KXMLQLCInputChannelPageDown);
        break;
        case PageSet:
            str = QString(KXMLQLCInputChannelPageSet);
        break;
        default:
            str = QString(KXMLQLCInputChannelNone);
    }

    return str;
}

QLCInputChannel::Type QLCInputChannel::stringToType(const QString& type)
{
    if (type == KXMLQLCInputChannelButton)
        return Button;
    else if (type == KXMLQLCInputChannelKnob)
        return Knob;
    else if (type == KXMLQLCInputChannelEncoder)
        return Encoder;
    else if (type == KXMLQLCInputChannelSlider)
        return Slider;
    else if (type == KXMLQLCInputChannelPageUp)
        return NextPage;
    else if (type == KXMLQLCInputChannelPageDown)
        return PrevPage;
    else if (type == KXMLQLCInputChannelPageSet)
        return PageSet;
    else
        return NoType;
}

QStringList QLCInputChannel::types()
{
    QStringList list;
    list << KXMLQLCInputChannelSlider;
    list << KXMLQLCInputChannelKnob;
    list << KXMLQLCInputChannelEncoder;
    list << KXMLQLCInputChannelButton;
    list << KXMLQLCInputChannelPageUp;
    list << KXMLQLCInputChannelPageDown;
    list << KXMLQLCInputChannelPageSet;
    return list;
}

QIcon QLCInputChannel::typeToIcon(Type type)
{
    switch (type)
    {
        case Button: return QIcon(":/button.png");
        case Knob: return QIcon(":/knob.png");
        case Encoder: return QIcon(":/knob.png");
        case Slider: return QIcon(":/slider.png");
        case PrevPage: return QIcon(":/forward.png");
        case NextPage: return QIcon(":/back.png");
        case PageSet: return QIcon(":/star.png");
        default: return QIcon();
    }
}

QIcon QLCInputChannel::stringToIcon(const QString& str)
{
    return typeToIcon(stringToType(str));
}

QIcon QLCInputChannel::icon() const
{
    return typeToIcon(type());
}

/****************************************************************************
 * Name
 ****************************************************************************/

void QLCInputChannel::setName(const QString& name)
{
    m_name = name;
}

QString QLCInputChannel::name() const
{
    return m_name;
}

/*********************************************************************
 * Slider/Knob movement behaviour specific methods
 *********************************************************************/

QLCInputChannel::MovementType QLCInputChannel::movementType() const
{
    return m_movementType;
}

void QLCInputChannel::setMovementType(QLCInputChannel::MovementType type)
{
    m_movementType = type;
}

int QLCInputChannel::movementSensitivity() const
{
    return m_movementSensitivity;
}

void QLCInputChannel::setMovementSensitivity(int value)
{
    m_movementSensitivity = value;
}

/*********************************************************************
 * Button behaviour specific methods
 *********************************************************************/

void QLCInputChannel::setSendExtraPress(bool enable)
{
    m_sendExtraPress = enable;
}

bool QLCInputChannel::sendExtraPress() const
{
    return m_sendExtraPress;
}

void QLCInputChannel::setRange(uchar lower, uchar upper)
{
    m_lower = lower;
    m_upper = upper;
}

uchar QLCInputChannel::lowerValue() const
{
    return m_lower;
}

uchar QLCInputChannel::upperValue() const
{
    return m_upper;
}

/****************************************************************************
 * Load & Save
 ****************************************************************************/

bool QLCInputChannel::loadXML(QXmlStreamReader &root)
{
    if (root.isStartElement() == false || root.name() != KXMLQLCInputChannel)
    {
        qWarning() << Q_FUNC_INFO << "Channel node not found";
        return false;
    }

    while (root.readNextStartElement())
    {
        if (root.name() == KXMLQLCInputChannelName)
        {
            setName(root.readElementText());
        }
        else if (root.name() == KXMLQLCInputChannelType)
        {
            setType(stringToType(root.readElementText()));
        }
        else if (root.name() == KXMLQLCInputChannelExtraPress)
        {
            root.readElementText();
            setSendExtraPress(true);
        }
        else if (root.name() == KXMLQLCInputChannelMovement)
        {
            if (root.attributes().hasAttribute(KXMLQLCInputChannelSensitivity))
                setMovementSensitivity(root.attributes().value(KXMLQLCInputChannelSensitivity).toString().toInt());

            if (root.readElementText() == KXMLQLCInputChannelRelative)
                setMovementType(Relative);
        }
        else if (root.name() == KXMLQLCInputChannelFeedbacks)
        {
            uchar min = 0, max = UCHAR_MAX;

            if (root.attributes().hasAttribute(KXMLQLCInputChannelLowerValue))
                min = uchar(root.attributes().value(KXMLQLCInputChannelLowerValue).toString().toUInt());
            if (root.attributes().hasAttribute(KXMLQLCInputChannelUpperValue))
                max = uchar(root.attributes().value(KXMLQLCInputChannelUpperValue).toString().toUInt());

            setRange(min, max);
            root.skipCurrentElement();
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown input channel tag" << root.name();
            root.skipCurrentElement();
        }
    }

    return true;
}

bool QLCInputChannel::saveXML(QXmlStreamWriter *doc, quint32 channelNumber) const
{
    if (doc == NULL || doc->device() == NULL)
        return false;

    doc->writeStartElement(KXMLQLCInputChannel);
    doc->writeAttribute(KXMLQLCInputChannelNumber,
                        QString("%1").arg(channelNumber));

    doc->writeTextElement(KXMLQLCInputChannelName, m_name);
    doc->writeTextElement(KXMLQLCInputChannelType, typeToString(m_type));
    if (sendExtraPress() == true)
        doc->writeTextElement(KXMLQLCInputChannelExtraPress, "True");

    /* Save only slider's relative movement */
    if ((type() == Slider || type() == Knob) && movementType() == Relative)
    {
        doc->writeStartElement(KXMLQLCInputChannelMovement);
        doc->writeAttribute(KXMLQLCInputChannelSensitivity, QString::number(movementSensitivity()));
        doc->writeCharacters(KXMLQLCInputChannelRelative);
        doc->writeEndElement();
    }
    else if (type() == Encoder)
    {
        doc->writeStartElement(KXMLQLCInputChannelMovement);
        doc->writeAttribute(KXMLQLCInputChannelSensitivity, QString::number(movementSensitivity()));
        doc->writeEndElement();
    }
    else if (type() == Button && (lowerValue() != 0 || upperValue() != UCHAR_MAX))
    {
        doc->writeStartElement(KXMLQLCInputChannelFeedbacks);
        if (lowerValue() != 0)
            doc->writeAttribute(KXMLQLCInputChannelLowerValue, QString::number(lowerValue()));
        if (upperValue() != UCHAR_MAX)
            doc->writeAttribute(KXMLQLCInputChannelUpperValue, QString::number(upperValue()));
        doc->writeEndElement();
    }

    doc->writeEndElement();
    return true;
}
