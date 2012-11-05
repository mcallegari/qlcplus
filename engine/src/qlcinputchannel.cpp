/*
  Q Light Controller
  qlcinputchannel.cpp

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

#include <QString>
#include <QtXml>

#include "qlcinputchannel.h"
#include "qlcinputprofile.h"

/****************************************************************************
 * Initialization
 ****************************************************************************/

QLCInputChannel::QLCInputChannel()
{
    m_type = Button;
}

QLCInputChannel::QLCInputChannel(const QLCInputChannel& channel)
{
    m_name = channel.m_name;
    m_type = channel.m_type;
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
    else
        return NoType;
}

QStringList QLCInputChannel::types()
{
    QStringList list;
    list << KXMLQLCInputChannelSlider;
    list << KXMLQLCInputChannelKnob;
    list << KXMLQLCInputChannelButton;
    return list;
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

    return true;
}
