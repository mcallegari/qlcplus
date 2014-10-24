/*
  Q Light Controller
  qlcfixturemode.cpp

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

#include <iostream>
#include <QString>
#include <QDebug>
#include <QVector>
#include <QtXml>

#include "qlcfixturemode.h"
#include "qlcfixturehead.h"
#include "qlcfixturedef.h"
#include "qlcchannel.h"
#include "qlcphysical.h"

QLCFixtureMode::QLCFixtureMode(QLCFixtureDef* fixtureDef)
    : m_fixtureDef(fixtureDef)
{
    Q_ASSERT(fixtureDef != NULL);
}

QLCFixtureMode::QLCFixtureMode(QLCFixtureDef* fixtureDef, const QLCFixtureMode* mode)
    : m_fixtureDef(fixtureDef)
{
    Q_ASSERT(fixtureDef != NULL);
    Q_ASSERT(mode != NULL);

    if (mode != NULL)
        *this = *mode;
}

QLCFixtureMode::~QLCFixtureMode()
{
}

QLCFixtureMode& QLCFixtureMode::operator=(const QLCFixtureMode& mode)
{
    if (this != &mode)
    {
        m_name = mode.m_name;
        m_physical = mode.m_physical;
        m_heads = mode.m_heads;

        /* Clear the existing list of channels */
        m_channels.clear();

        Q_ASSERT(m_fixtureDef != NULL);

        quint32 i = 0;
        QVectorIterator <QLCChannel*> it(mode.m_channels);
        while (it.hasNext() == true)
        {
            /* Since m_fixtureDef might not be the same as
               mode.m_fixtureDef, we need to search for a
               channel with the same name from m_fixtureDef and
               not from mode.m_fixtureDef. If the channel in the
               other mode is deleted, the one in this copied mode
               will be invalid and we end up in a crash. */
            QLCChannel* ch = it.next();
            QLCChannel* actual = m_fixtureDef->channel(ch->name());
            if (actual != NULL)
                insertChannel(actual, i++);
            else
                qWarning() << Q_FUNC_INFO << "Unable to find channel"
                           << ch->name() << "for mode"
                           << m_name << "from its fixture definition";
        }
    }

    return *this;
}

/****************************************************************************
 * Name
 ****************************************************************************/

void QLCFixtureMode::setName(const QString &name)
{
    m_name = name;
}

QString QLCFixtureMode::name() const
{
    return m_name;
}

/*****************************************************************************
 * Fixture definition
 *****************************************************************************/

QLCFixtureDef* QLCFixtureMode::fixtureDef() const
{
    return m_fixtureDef;
}

/****************************************************************************
 * Channels
 ****************************************************************************/

bool QLCFixtureMode::insertChannel(QLCChannel* channel, quint32 index)
{
    if (channel == NULL)
    {
        qWarning() << Q_FUNC_INFO << "Will not add a NULL channel to mode"
                   << m_name;
        return false;
    }

    Q_ASSERT(m_fixtureDef != NULL);

    if (m_fixtureDef->channels().contains(channel) == true)
    {
        if (m_channels.contains(channel) == false)
        {
            if (index >= quint32(m_channels.size()))
                m_channels.append(channel);
            else
                m_channels.insert(index, channel);
            return true;
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Channel" << channel->name()
                       << "is already a member of mode" << m_name;
            return false;
        }
    }
    else
    {
        qWarning() << Q_FUNC_INFO << "Will not add channel" << channel->name()
                   << "to mode" << m_name
                   << "because the channel does not belong to mode's"
                   << "parent fixture definition.";
        return false;
    }
}

bool QLCFixtureMode::removeChannel(const QLCChannel* channel)
{
    QMutableVectorIterator <QLCChannel*> it(m_channels);
    while (it.hasNext() == true)
    {
        if (it.next() == channel)
        {
            /* Don't delete the channel since QLCFixtureModes
               don't own them. QLCFixtureDefs do. */
            it.remove();
            return true;
        }
    }

    return false;
}

void QLCFixtureMode::removeAllChannels()
{
    m_channels.clear();
}

QLCChannel* QLCFixtureMode::channel(const QString& name) const
{
    QVectorIterator <QLCChannel*> it(m_channels);
    while (it.hasNext() == true)
    {
        QLCChannel* ch = it.next();
        Q_ASSERT(ch != NULL);
        if (ch->name() == name)
            return ch;
    }

    return NULL;
}

QLCChannel* QLCFixtureMode::channel(quint32 ch) const
{
    return m_channels.value(ch, NULL);
}

QVector <QLCChannel*> QLCFixtureMode::channels() const
{
    return m_channels;
}

quint32 QLCFixtureMode::channelNumber(QLCChannel* channel) const
{
    if (channel == NULL)
        return QLCChannel::invalid();

    for (int i = 0; i < m_channels.size(); i++)
    {
        if (m_channels.at(i) == channel)
            return i;
    }

    return QLCChannel::invalid();
}

/*****************************************************************************
 * Heads
 *****************************************************************************/

void QLCFixtureMode::insertHead(int index, const QLCFixtureHead& head)
{
    if (index < 0 || index >= m_heads.size())
        m_heads.append(head);
    else
        m_heads.insert(index, head);
}

void QLCFixtureMode::removeHead(int index)
{
    if (index >= 0 && index < m_heads.size())
        m_heads.remove(index);
}

void QLCFixtureMode::replaceHead(int index, const QLCFixtureHead& head)
{
    if (index >= 0 && index < m_heads.size())
        m_heads[index] = head;
}

QVector <QLCFixtureHead> const& QLCFixtureMode::heads() const
{
    return m_heads;
}

int QLCFixtureMode::headForChannel(quint32 chnum) const
{
    for (int i = 0; i < m_heads.size(); i++)
    {
        if (m_heads[i].channels().contains(chnum) == true)
            return i;
    }

    return -1;
}

void QLCFixtureMode::cacheHeads()
{
    for (int i = 0; i < m_heads.size(); i++)
    {
        QLCFixtureHead& head(m_heads[i]);
        head.cacheChannels(this);
    }
}

/****************************************************************************
 * Physical
 ****************************************************************************/

void QLCFixtureMode::setPhysical(const QLCPhysical& physical)
{
    m_physical = physical;
}

QLCPhysical QLCFixtureMode::physical() const
{
    return m_physical;
}

/****************************************************************************
 * Load & Save
 ****************************************************************************/

bool QLCFixtureMode::loadXML(const QDomElement& root)
{
    if (root.tagName() != KXMLQLCFixtureMode)
    {
        qWarning() << Q_FUNC_INFO << "Mode tag not found";
        return false;
    }

    /* Mode name */
    QString str = root.attribute(KXMLQLCFixtureModeName);
    if (str.isEmpty() == true)
    {
        qWarning() << Q_FUNC_INFO << "Mode has no name";
        return false;
    }
    else
    {
        setName(str);
    }

    /* Subtags */
    QDomNode node = root.firstChild();
    while (node.isNull() == false)
    {
        QDomElement tag = node.toElement();
        if (tag.tagName() == KXMLQLCFixtureModeChannel)
        {
            /* Channel */
            Q_ASSERT(m_fixtureDef != NULL);
            str = tag.attribute(KXMLQLCFixtureModeChannelNumber);
            insertChannel(m_fixtureDef->channel(tag.text()),
                          str.toInt());
        }
        else if (tag.tagName() == KXMLQLCFixtureHead)
        {
            /* Head */
            QLCFixtureHead head;
            if (head.loadXML(tag) == true)
                insertHead(-1, head);
        }
        else if (tag.tagName() == KXMLQLCPhysical)
        {
            /* Physical */
            QLCPhysical physical;
            physical.loadXML(tag);
            setPhysical(physical);
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown Fixture Mode tag:" << tag.tagName();
        }

        node = node.nextSibling();
    }

    // Cache all head channels
    cacheHeads();

    return true;
}

bool QLCFixtureMode::saveXML(QDomDocument* doc, QDomElement* root)
{
    QDomElement tag;
    QDomElement chtag;
    QDomText text;
    QString str;
    int i = 0;

    Q_ASSERT(doc != NULL);
    Q_ASSERT(root != NULL);

    /* Mode entry */
    tag = doc->createElement(KXMLQLCFixtureMode);
    tag.setAttribute(KXMLQLCFixtureModeName, m_name);
    root->appendChild(tag);

    m_physical.saveXML(doc, &tag);

    /* Channels */
    QVectorIterator <QLCChannel*> it(m_channels);
    while (it.hasNext() == true)
    {
        chtag = doc->createElement(KXMLQLCFixtureModeChannel);
        tag.appendChild(chtag);

        str.setNum(i++);
        chtag.setAttribute(KXMLQLCFixtureModeChannelNumber, str);

        text = doc->createTextNode(it.next()->name());
        chtag.appendChild(text);
    }

    /* Heads */
    QVectorIterator <QLCFixtureHead> hit(m_heads);
    while (hit.hasNext() == true)
        hit.next().saveXML(doc, &tag);

    return true;
}
