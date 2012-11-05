/*
  Q Light Controller
  qlcchannel.cpp

  Copyright (C) Heikki Junnila

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

#include <QStringList>
#include <iostream>
#include <QString>
#include <QFile>
#include <QtXml>

#include "qlcchannel.h"
#include "qlccapability.h"

#define KXMLQLCChannelGroupIntensity   QString("Intensity")
#define KXMLQLCChannelGroupColour      QString("Colour")
#define KXMLQLCChannelGroupGobo        QString("Gobo")
#define KXMLQLCChannelGroupPrism       QString("Prism")
#define KXMLQLCChannelGroupShutter     QString("Shutter")
#define KXMLQLCChannelGroupBeam        QString("Beam")
#define KXMLQLCChannelGroupSpeed       QString("Speed")
#define KXMLQLCChannelGroupEffect      QString("Effect")
#define KXMLQLCChannelGroupPan         QString("Pan")
#define KXMLQLCChannelGroupTilt        QString("Tilt")
#define KXMLQLCChannelGroupMaintenance QString("Maintenance")
#define KXMLQLCChannelGroupNothing     QString("Nothing")

#define KXMLQLCChannelColourGeneric    QString("Generic")
#define KXMLQLCChannelColourRed        QString("Red")
#define KXMLQLCChannelColourGreen      QString("Green")
#define KXMLQLCChannelColourBlue       QString("Blue")
#define KXMLQLCChannelColourCyan       QString("Cyan")
#define KXMLQLCChannelColourMagenta    QString("Magenta")
#define KXMLQLCChannelColourYellow     QString("Yellow")

QLCChannel::QLCChannel()
{
    m_group = Intensity;
    m_controlByte = MSB;
    m_colour = NoColour;
}

QLCChannel::QLCChannel(const QLCChannel* channel)
{
    m_group = Intensity;
    m_controlByte = MSB;
    m_colour = NoColour;

    if (channel != NULL)
        *this = *channel;
}

QLCChannel::~QLCChannel()
{
    while (m_capabilities.isEmpty() == false)
        delete m_capabilities.takeFirst();
}

QLCChannel& QLCChannel::operator=(const QLCChannel& channel)
{
    if (this != &channel)
    {
        QListIterator<QLCCapability*> it(channel.m_capabilities);

        m_name = channel.m_name;
        m_group = channel.m_group;
        m_controlByte = channel.m_controlByte;
        m_colour = channel.m_colour;

        /* Clear old capabilities */
        while (m_capabilities.isEmpty() == false)
            delete m_capabilities.takeFirst();

        /* Copy new capabilities from the other channel */
        while (it.hasNext() == true)
            m_capabilities.append(new QLCCapability(it.next()));
    }

    return *this;
}

quint32 QLCChannel::invalid()
{
    return UINT_MAX;
}

/*****************************************************************************
 * Groups
 *****************************************************************************/

QStringList QLCChannel::groupList()
{
    QStringList list;

    // Keep this list in alphabetical order because it's used only in UI
    list.append(KXMLQLCChannelGroupBeam);
    list.append(KXMLQLCChannelGroupColour);
    list.append(KXMLQLCChannelGroupEffect);
    list.append(KXMLQLCChannelGroupGobo);
    list.append(KXMLQLCChannelGroupIntensity);
    list.append(KXMLQLCChannelGroupMaintenance);
    list.append(KXMLQLCChannelGroupNothing);
    list.append(KXMLQLCChannelGroupPan);
    list.append(KXMLQLCChannelGroupPrism);
    list.append(KXMLQLCChannelGroupShutter);
    list.append(KXMLQLCChannelGroupSpeed);
    list.append(KXMLQLCChannelGroupTilt);

    return list;
}

QString QLCChannel::groupToString(Group grp)
{
    switch (grp)
    {
    case Intensity:
        return KXMLQLCChannelGroupIntensity;
    case Colour:
        return KXMLQLCChannelGroupColour;
    case Gobo:
        return KXMLQLCChannelGroupGobo;
    case Prism:
        return KXMLQLCChannelGroupPrism;
    case Shutter:
        return KXMLQLCChannelGroupShutter;
    case Beam:
        return KXMLQLCChannelGroupBeam;
    case Speed:
        return KXMLQLCChannelGroupSpeed;
    case Effect:
        return KXMLQLCChannelGroupEffect;
    case Pan:
        return KXMLQLCChannelGroupPan;
    case Tilt:
        return KXMLQLCChannelGroupTilt;
    case Maintenance:
        return KXMLQLCChannelGroupMaintenance;
    default:
        return KXMLQLCChannelGroupNothing;
    }
}

QLCChannel::Group QLCChannel::stringToGroup(const QString& str)
{
    if (str == KXMLQLCChannelGroupIntensity)
        return Intensity;
    else if (str == KXMLQLCChannelGroupColour)
        return Colour;
    else if (str == KXMLQLCChannelGroupGobo)
        return Gobo;
    else if (str == KXMLQLCChannelGroupPrism)
        return Prism;
    else if (str == KXMLQLCChannelGroupShutter)
        return Shutter;
    else if (str == KXMLQLCChannelGroupBeam)
        return Beam;
    else if (str == KXMLQLCChannelGroupSpeed)
        return Speed;
    else if (str == KXMLQLCChannelGroupEffect)
        return Effect;
    else if (str == KXMLQLCChannelGroupPan)
        return Pan;
    else if (str == KXMLQLCChannelGroupTilt)
        return Tilt;
    else if (str == KXMLQLCChannelGroupMaintenance)
        return Maintenance;
    else
        return NoGroup;
}

void QLCChannel::setGroup(Group grp)
{
    m_group = grp;
}

QLCChannel::Group QLCChannel::group() const
{
    return m_group;
}

/*****************************************************************************
 * Properties
 *****************************************************************************/

QString QLCChannel::name() const
{
    return m_name;
}

void QLCChannel::setName(const QString &name)
{
    m_name = name;
}

void QLCChannel::setControlByte(ControlByte byte)
{
    m_controlByte = byte;
}

QLCChannel::ControlByte QLCChannel::controlByte() const
{
    return m_controlByte;
}

/*****************************************************************************
 * Colours
 *****************************************************************************/

QStringList QLCChannel::colourList()
{
    QStringList list;
    list << KXMLQLCChannelColourGeneric;
    list << KXMLQLCChannelColourRed;
    list << KXMLQLCChannelColourGreen;
    list << KXMLQLCChannelColourBlue;
    list << KXMLQLCChannelColourCyan;
    list << KXMLQLCChannelColourMagenta;
    list << KXMLQLCChannelColourYellow;
    return list;
}

QString QLCChannel::colourToString(PrimaryColour colour)
{
    switch (colour)
    {
    case Red:
        return KXMLQLCChannelColourRed;
    case Green:
        return KXMLQLCChannelColourGreen;
    case Blue:
        return KXMLQLCChannelColourBlue;
    case Cyan:
        return KXMLQLCChannelColourCyan;
    case Magenta:
        return KXMLQLCChannelColourMagenta;
    case Yellow:
        return KXMLQLCChannelColourYellow;
    case NoColour:
    default:
        return KXMLQLCChannelColourGeneric;
    }
}

QLCChannel::PrimaryColour QLCChannel::stringToColour(const QString& str)
{
    if (str == KXMLQLCChannelColourRed)
        return Red;
    else if (str == KXMLQLCChannelColourGreen)
        return Green;
    else if (str == KXMLQLCChannelColourBlue)
        return Blue;
    else if (str == KXMLQLCChannelColourCyan)
        return Cyan;
    else if (str == KXMLQLCChannelColourMagenta)
        return Magenta;
    else if (str == KXMLQLCChannelColourYellow)
        return Yellow;
    else
        return NoColour;
}

void QLCChannel::setColour(QLCChannel::PrimaryColour colour)
{
    m_colour = colour;
}

QLCChannel::PrimaryColour QLCChannel::colour() const
{
    return m_colour;
}

/*****************************************************************************
 * Capabilities
 *****************************************************************************/

const QList <QLCCapability*> QLCChannel::capabilities() const
{
    return m_capabilities;
}

QLCCapability* QLCChannel::searchCapability(uchar value) const
{
    QListIterator <QLCCapability*> it(m_capabilities);
    while (it.hasNext() == true)
    {
        QLCCapability* capability = it.next();
        if (capability->min() <= value && capability->max() >= value)
            return capability;
    }

    return NULL;
}

QLCCapability* QLCChannel::searchCapability(const QString& name,
        bool exactMatch) const
{
    QListIterator <QLCCapability*> it(m_capabilities);
    while (it.hasNext() == true)
    {
        QLCCapability* capability = it.next();
        if (exactMatch == true && capability->name() == name)
            return capability;
        else if (exactMatch == false &&
                 capability->name().contains(name) == true)
            return capability;
    }

    return NULL;
}

bool QLCChannel::addCapability(QLCCapability* cap)
{
    Q_ASSERT(cap != NULL);

    /* Check for overlapping values */
    foreach (QLCCapability* another, m_capabilities)
    {
        if (another->overlaps(cap) == true)
            return false;
    }

    m_capabilities.append(cap);
    return true;
}

bool QLCChannel::removeCapability(QLCCapability* cap)
{
    Q_ASSERT(cap != NULL);

    QMutableListIterator <QLCCapability*> it(m_capabilities);
    while (it.hasNext() == true)
    {
        if (it.next() == cap)
        {
            it.remove();
            delete cap;
            return true;
        }
    }

    return false;
}

static bool capsort(const QLCCapability* cap1, const QLCCapability* cap2)
{
    return (*cap1) < (*cap2);
}

void QLCChannel::sortCapabilities()
{
    qSort(m_capabilities.begin(), m_capabilities.end(), capsort);
}

/*****************************************************************************
 * File operations
 *****************************************************************************/

bool QLCChannel::saveXML(QDomDocument* doc, QDomElement* root) const
{
    QDomElement chtag;
    QDomElement tag;
    QDomText text;

    Q_ASSERT(doc != NULL);
    Q_ASSERT(root != NULL);

    /* Channel entry */
    chtag = doc->createElement(KXMLQLCChannel);
    chtag.setAttribute(KXMLQLCChannelName, m_name);
    root->appendChild(chtag);

    /* Group */
    tag = doc->createElement(KXMLQLCChannelGroup);
    text = doc->createTextNode(groupToString(m_group));
    tag.appendChild(text);

    /* Group control byte */
    tag.setAttribute(KXMLQLCChannelGroupByte, QString::number(controlByte()));
    chtag.appendChild(tag);

    /* Colour */
    if (m_colour != NoColour)
    {
        tag = doc->createElement(KXMLQLCChannelColour);
        text = doc->createTextNode(QLCChannel::colourToString(colour()));
        tag.appendChild(text);
        chtag.appendChild(tag);
    }

    /* Capabilities */
    QListIterator <QLCCapability*> it(m_capabilities);
    while (it.hasNext() == true)
        it.next()->saveXML(doc, &chtag);

    return true;
}

bool QLCChannel::loadXML(const QDomElement& root)
{
    if (root.tagName() != KXMLQLCChannel)
    {
        qWarning() << "Channel node not found.";
        return false;
    }

    /* Get channel name */
    QString str = root.attribute(KXMLQLCChannelName);
    if (str.isEmpty() == true)
        return false;
    setName(str);

    /* Subtags */
    QDomNode node = root.firstChild();
    while (node.isNull() == false)
    {
        QDomElement tag = node.toElement();
        if (tag.tagName() == KXMLQLCCapability)
        {
            /* Create a new capability and attempt to load it */
            QLCCapability* cap = new QLCCapability();
            if (cap->loadXML(tag) == true)
            {
                /* Loading succeeded */
                if (addCapability(cap) == false)
                {
                    /* Value overlaps with existing value */
                    delete cap;
                }
            }
            else
            {
                /* Loading failed */
                delete cap;
            }
        }
        else if (tag.tagName() == KXMLQLCChannelGroup)
        {
            str = tag.attribute(KXMLQLCChannelGroupByte);
            setControlByte(ControlByte(str.toInt()));
            setGroup(stringToGroup(tag.text()));
        }
        else if (tag.tagName() == KXMLQLCChannelColour)
        {
            setColour(stringToColour(tag.text()));
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown Channel tag: " << tag.tagName();
        }

        node = node.nextSibling();
    }

    return true;
}
