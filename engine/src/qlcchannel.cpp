/*
  Q Light Controller
  qlcchannel.cpp

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

#include <QStringList>
#include <QPainter>
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
#define KXMLQLCChannelColourAmber      QString("Amber")
#define KXMLQLCChannelColourWhite      QString("White")
#define KXMLQLCChannelColourUV         QString("UV")

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

QPixmap QLCChannel::drawIntensity(QColor color, QString str) const
{
    QPixmap pm(32, 32);
    QPainter painter(&pm);
    painter.setRenderHint(QPainter::Antialiasing);

    /*QFont tfont = QApplication::font();
    tfont.setBold(true);
    tfont.setPixelSize(14);
    painter.setFont(tfont);*/

    pm.fill(color);
    if (str == "B")
        painter.setPen(Qt::white);
    painter.drawText(0, 0, 32, 32, Qt::AlignHCenter|Qt::AlignVCenter, str);

    return pm;
}

QIcon QLCChannel::getIntensityIcon() const
{
    QPixmap pm(32, 32);

    if (m_colour == QLCChannel::Red)
        pm = drawIntensity(Qt::red, "R");
    else if (m_colour == QLCChannel::Green)
        pm = drawIntensity(Qt::green, "G");
    else if (m_colour == QLCChannel::Blue) 
        pm = drawIntensity(Qt::blue, "B");
    else if (m_colour == QLCChannel::Cyan)
        pm = drawIntensity(Qt::cyan, "C");
    else if (m_colour == QLCChannel::Magenta)
        pm = drawIntensity(Qt::magenta, "M");
    else if (m_colour == QLCChannel::Yellow)
        pm = drawIntensity(Qt::yellow, "Y");
    else if (m_colour == QLCChannel::Amber)
        pm = drawIntensity(QColor(0xFFFF7E00), "A");
    else if (m_colour == QLCChannel::White)
        pm = drawIntensity(Qt::white, "W");
    else if (m_colour == QLCChannel::UV)
        pm = drawIntensity(QColor(0xFF9400D3), "UV");
    else
    {
        // None of the primary colours matched and since this is an
        // intensity channel, it must be controlling a plain dimmer OSLT.
        return QIcon(":/intensity.png");
    }

    return QIcon(pm);
}

QString QLCChannel::getIntensityColorCode() const
{
    if (m_colour == QLCChannel::Red)
        return QString("#FF0000");
    else if (m_colour == QLCChannel::Green)
        return QString("#00FF00");
    else if (m_colour == QLCChannel::Blue)
        return QString("#0000FF");
    else if (m_colour == QLCChannel::Cyan)
        return QString("#00FFFF");
    else if (m_colour == QLCChannel::Magenta)
        return QString("#FF00FF");
    else if (m_colour == QLCChannel::Yellow)
        return QString("#FFFF00");
    else if (m_colour == QLCChannel::Amber)
        return QString("#FF7E00");
    else if (m_colour == QLCChannel::White)
        return QString("#FFFFFF");
    else if (m_colour == QLCChannel::UV)
        return QString("#9400D3");
    else
    {
        // None of the primary colours matched and since this is an
        // intensity channel, it must be controlling a plain dimmer OSLT.
        return QString(":/intensity.png");
    }

    return QString(":/intensity.png");
}

QIcon QLCChannel::getIcon() const
{
    if (group() == Intensity)
        return getIntensityIcon();
    else
        return QIcon(getIconNameFromGroup(group()));
}

QString QLCChannel::getIconNameFromGroup(QLCChannel::Group grp) const
{
    switch(grp)
    {
        case Pan: return QString(":/pan.png"); break;
        case Tilt: return QString(":/tilt.png"); break;
        case Colour: return QString(":/colorwheel.png"); break;
        case Effect: return QString(":/star.png"); break;
        case Gobo: return QString(":/gobo.png"); break;
        case Shutter: return QString(":/shutter.png"); break;
        case Speed: return QString(":/speed.png"); break;
        case Prism: return QString(":/prism.png"); break;
        case Maintenance: return QString(":/configure.png"); break;
        case Intensity: return getIntensityColorCode(); break;
        case Beam: return QString(":/beam.png"); break;
        default:
        break;
    }
    return QString(":/intensity.png");
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
    list << KXMLQLCChannelColourAmber;
    list << KXMLQLCChannelColourWhite;
    list << KXMLQLCChannelColourUV;
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
    case Amber:
        return KXMLQLCChannelColourAmber;
    case White:
        return KXMLQLCChannelColourWhite;
    case UV:
        return KXMLQLCChannelColourUV;
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
    else if (str == KXMLQLCChannelColourAmber)
        return Amber;
    else if (str == KXMLQLCChannelColourWhite)
        return White;
    else if (str == KXMLQLCChannelColourUV)
        return UV;
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
