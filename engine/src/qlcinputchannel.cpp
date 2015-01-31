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

#include <QString>
#include <QtXml>
#include <QIcon>

#include "qlcinputchannel.h"
#include "qlcinputprofile.h"

/****************************************************************************
 * Initialization
 ****************************************************************************/

QLCInputChannel::QLCInputChannel()
{
    m_type = Button;
    m_movementType = Absolute;
    m_movementSensitivity = 20;
}

QLCInputChannel::QLCInputChannel(const QLCInputChannel& channel)
{
    m_name = channel.m_name;
    m_type = channel.m_type;
    m_movementType = channel.m_movementType;
    m_movementSensitivity = channel.m_movementSensitivity;
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
    case Button:
        return QIcon(":/button.png");
    case Knob:
        return QIcon(":/knob.png");
    case Slider:
        return QIcon(":/slider.png");
    case PrevPage:
        return QIcon(":/forward.png");
    case NextPage:
        return QIcon(":/back.png");
    case PageSet:
       return QIcon(":/star.png");
    default:
       return QIcon();
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
 * Slider movement behaviour specific methods
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

/****************************************************************************
 * Load & Save
 ****************************************************************************/

bool QLCInputChannel::loadXML(const QDomElement& root)
{
    /* Verify that the tag contains an input channel */
    if (root.tagName() != KXMLQLCInputChannel)
    {
        qWarning() << Q_FUNC_INFO << "Channel node not found";
        return false;
    }

    /* Go thru all sub tags */
    QDomNode node = root.firstChild();
    while (node.isNull() == false)
    {
        QDomElement tag = node.toElement();
        if (tag.tagName() == KXMLQLCInputChannelName)
        {
            setName(tag.text());
        }
        else if (tag.tagName() == KXMLQLCInputChannelType)
        {
            setType(stringToType(tag.text()));
        }
        else if (tag.tagName() == KXMLQLCInputChannelMovement)
        {
            if (tag.hasAttribute(KXMLQLCInputChannelSensitivity))
                setMovementSensitivity(tag.attribute(KXMLQLCInputChannelSensitivity).toInt());
            if (tag.text() == KXMLQLCInputChannelRelative)
                setMovementType(Relative);
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown input channel tag" << tag.tagName();
        }

        node = node.nextSibling();
    }

    return true;
}

bool QLCInputChannel::saveXML(QDomDocument* doc, QDomElement* root,
                              quint32 channelNumber) const
{
    QDomElement subtag;
    QDomElement tag;
    QDomText text;
    QString str;

    Q_ASSERT(doc != NULL);
    Q_ASSERT(root != NULL);

    /* The channel tag */
    tag = doc->createElement(KXMLQLCInputChannel);
    root->appendChild(tag);

    /* Channel number attribute */
    tag.setAttribute(KXMLQLCInputChannelNumber,
                     QString("%1").arg(channelNumber));

    /* Name */
    subtag = doc->createElement(KXMLQLCInputChannelName);
    tag.appendChild(subtag);
    text = doc->createTextNode(m_name);
    subtag.appendChild(text);

    /* Type */
    subtag = doc->createElement(KXMLQLCInputChannelType);
    tag.appendChild(subtag);
    text = doc->createTextNode(typeToString(m_type));
    subtag.appendChild(text);

    /* Save only slider's relative movement */
    if (type() == Slider && movementType() == Relative)
    {
        subtag = doc->createElement(KXMLQLCInputChannelMovement);
        subtag.setAttribute(KXMLQLCInputChannelSensitivity, movementSensitivity());
        tag.appendChild(subtag);
        text = doc->createTextNode(KXMLQLCInputChannelRelative);
        subtag.appendChild(text);
    }

    return true;
}
