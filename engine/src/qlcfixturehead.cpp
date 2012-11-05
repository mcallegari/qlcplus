/*
  Q Light Controller
  qlcfixturehead.cpp

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

#include <QDomDocument>
#include <QDomElement>
#include <QDebug>

#include "qlcfixturehead.h"
#include "qlcfixturemode.h"
#include "qlcchannel.h"

QLCFixtureHead::QLCFixtureHead()
    : m_channelsCached(false)
    , m_panMsbChannel(QLCChannel::invalid())
    , m_tiltMsbChannel(QLCChannel::invalid())
    , m_panLsbChannel(QLCChannel::invalid())
    , m_tiltLsbChannel(QLCChannel::invalid())
    , m_masterIntensityChannel(QLCChannel::invalid())
{
}

QLCFixtureHead::QLCFixtureHead(const QLCFixtureHead& head)
    : m_channels(head.m_channels)
    , m_channelsCached(head.m_channelsCached)
    , m_panMsbChannel(head.m_panMsbChannel)
    , m_tiltMsbChannel(head.m_tiltMsbChannel)
    , m_panLsbChannel(head.m_panLsbChannel)
    , m_tiltLsbChannel(head.m_tiltLsbChannel)
    , m_masterIntensityChannel(head.m_masterIntensityChannel)
    , m_rgbChannels(head.m_rgbChannels)
    , m_cmyChannels(head.m_cmyChannels)
{
}

QLCFixtureHead::~QLCFixtureHead()
{
}

/****************************************************************************
 * Channels
 ****************************************************************************/

void QLCFixtureHead::addChannel(quint32 channel)
{
    if (m_channels.contains(channel) == false)
        m_channels.insert(channel);
}

void QLCFixtureHead::removeChannel(quint32 channel)
{
    m_channels.remove(channel);
}

QSet <quint32> QLCFixtureHead::channels() const
{
    return m_channels;
}

/****************************************************************************
 * Cached channels
 ****************************************************************************/

quint32 QLCFixtureHead::panMsbChannel() const
{
    return m_panMsbChannel;
}

quint32 QLCFixtureHead::tiltMsbChannel() const
{
    return m_tiltMsbChannel;
}

quint32 QLCFixtureHead::panLsbChannel() const
{
    return m_panLsbChannel;
}

quint32 QLCFixtureHead::tiltLsbChannel() const
{
    return m_tiltLsbChannel;
}

quint32 QLCFixtureHead::masterIntensityChannel() const
{
    return m_masterIntensityChannel;
}

QList <quint32> QLCFixtureHead::rgbChannels() const
{
    return m_rgbChannels;
}

QList <quint32> QLCFixtureHead::cmyChannels() const
{
    return m_cmyChannels;
}

void QLCFixtureHead::cacheChannels(const QLCFixtureMode* mode)
{
    Q_ASSERT(mode != NULL);

    // Allow only one caching round per fixture mode instance
    if (m_channelsCached == true)
        return;

    quint32 r, g, b, c, m, y;
    r = g = b = c = m = y = QLCChannel::invalid();

    QSetIterator <quint32> it(m_channels);
    while (it.hasNext() == true)
    {
        quint32 i(it.next());
        const QLCChannel* ch = mode->channels().at(i);
        Q_ASSERT(ch != NULL);

        if (ch->group() == QLCChannel::Pan)
        {
            if (ch->controlByte() == QLCChannel::MSB)
                m_panMsbChannel = i;
            else if (ch->controlByte() == QLCChannel::LSB)
                m_panLsbChannel = i;
        }
        else if (ch->group() == QLCChannel::Tilt)
        {
            if (ch->controlByte() == QLCChannel::MSB)
                m_tiltMsbChannel = i;
            else if (ch->controlByte() == QLCChannel::LSB)
                m_tiltLsbChannel = i;
        }
        else if (ch->group() == QLCChannel::Intensity &&
                 ch->colour() == QLCChannel::NoColour)
        {
            m_masterIntensityChannel = i;
        }
        else if (ch->group() == QLCChannel::Intensity &&
                 ch->colour() == QLCChannel::Red)
        {
            r = i;
        }
        else if (ch->group() == QLCChannel::Intensity &&
                 ch->colour() == QLCChannel::Green)
        {
            g = i;
        }
        else if (ch->group() == QLCChannel::Intensity &&
                 ch->colour() == QLCChannel::Blue)
        {
            b = i;
        }
        else if (ch->group() == QLCChannel::Intensity &&
                 ch->colour() == QLCChannel::Cyan)
        {
            c = i;
        }
        else if (ch->group() == QLCChannel::Intensity &&
                 ch->colour() == QLCChannel::Magenta)
        {
            m = i;
        }
        else if (ch->group() == QLCChannel::Intensity &&
                 ch->colour() == QLCChannel::Yellow)
        {
            y = i;
        }
    }

    if (r != QLCChannel::invalid() && g != QLCChannel::invalid() && b != QLCChannel::invalid())
        m_rgbChannels << r << g << b;
    if (c != QLCChannel::invalid() && m != QLCChannel::invalid() && y != QLCChannel::invalid())
        m_cmyChannels << c << m << y;

    // Allow only one caching round per head
    m_channelsCached = true;
}

/****************************************************************************
 * Load & Save
 ****************************************************************************/

bool QLCFixtureHead::loadXML(const QDomElement& root)
{
    if (root.tagName() != KXMLQLCFixtureHead)
    {
        qWarning() << Q_FUNC_INFO << "Fixture Head node not found!";
        return false;
    }

    QDomNode node = root.firstChild();
    while (node.isNull() == false)
    {
        QDomElement tag = node.toElement();
        if (tag.tagName() == KXMLQLCFixtureHeadChannel)
            addChannel(tag.text().toUInt());
        else
            qWarning() << Q_FUNC_INFO << "Unknown Head tag:" << tag.tagName();
        node = node.nextSibling();
    }

    return true;
}

bool QLCFixtureHead::saveXML(QDomDocument* doc, QDomElement* mode_root) const
{
    Q_ASSERT(doc != NULL);
    Q_ASSERT(mode_root != NULL);

    QDomElement root = doc->createElement(KXMLQLCFixtureHead);
    mode_root->appendChild(root);

    QSetIterator <quint32> it(m_channels);
    while (it.hasNext() == true)
    {
        QDomElement tag = doc->createElement(KXMLQLCFixtureHeadChannel);
        QDomText text = doc->createTextNode(QString::number(it.next()));
        tag.appendChild(text);
        root.appendChild(tag);
    }

    return true;
}

/****************************************************************************
 * QLCDimmerHead
 ****************************************************************************/

QLCDimmerHead::QLCDimmerHead(int head)
    : QLCFixtureHead()
{
    m_masterIntensityChannel = head;
    m_channelsCached = true;
}

