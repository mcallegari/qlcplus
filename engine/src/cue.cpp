/*
  Q Light Controller
  cue.cpp

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

#include <QDomDocument>
#include <QDomElement>
#include <QDebug>

#include "cue.h"

Cue::Cue(const QString& name)
    : m_name(name)
    , m_fadeInSpeed(0)
    , m_fadeOutSpeed(0)
    , m_duration(0)
{
}

Cue::Cue(const QHash <uint,uchar> values)
    : m_name(QString())
    , m_values(values)
    , m_fadeInSpeed(0)
    , m_fadeOutSpeed(0)
    , m_duration(0)
{
}

Cue::Cue(const Cue& cue)
    : m_name(cue.name())
    , m_values(cue.values())
    , m_fadeInSpeed(cue.fadeInSpeed())
    , m_fadeOutSpeed(cue.fadeOutSpeed())
    , m_duration(cue.duration())
{
}

Cue::~Cue()
{
}

/****************************************************************************
 * Name
 ****************************************************************************/

void Cue::setName(const QString& str)
{
    m_name = str;
}

QString Cue::name() const
{
    return m_name;
}

/****************************************************************************
 * Values
 ****************************************************************************/

void Cue::setValue(uint channel, uchar value)
{
    m_values[channel] = value;
}

void Cue::unsetValue(uint channel)
{
    if (m_values.contains(channel) == true)
        m_values.remove(channel);
}

uchar Cue::value(uint channel) const
{
    if (m_values.contains(channel) == true)
        return m_values[channel];
    else
        return 0;
}

QHash <uint,uchar> Cue::values() const
{
    return m_values;
}

/****************************************************************************
 * Speed
 ****************************************************************************/

void Cue::setFadeInSpeed(uint ms)
{
    m_fadeInSpeed = ms;
}

uint Cue::fadeInSpeed() const
{
    return m_fadeInSpeed;
}

void Cue::setFadeOutSpeed(uint ms)
{
    m_fadeOutSpeed = ms;
}

uint Cue::fadeOutSpeed() const
{
    return m_fadeOutSpeed;
}

void Cue::setDuration(uint ms)
{
    m_duration = ms;
}

uint Cue::duration() const
{
    return m_duration;
}

/****************************************************************************
 * Load & Save
 ****************************************************************************/

bool Cue::loadXML(const QDomElement& root)
{
    qDebug() << Q_FUNC_INFO;

    if (root.tagName() != KXMLQLCCue)
    {
        qWarning() << Q_FUNC_INFO << "Cue node not found";
        return false;
    }

    setName(root.attribute(KXMLQLCCueName));

    QDomNode node = root.firstChild();
    while (node.isNull() == false)
    {
        QDomElement tag = node.toElement();
        if (tag.tagName() == KXMLQLCCueValue)
        {
            QString ch = tag.attribute(KXMLQLCCueValueChannel);
            QString val = tag.text();
            if (ch.isEmpty() == false && val.isEmpty() == false)
                setValue(ch.toUInt(), uchar(val.toUInt()));
        }
        else if (tag.tagName() == KXMLQLCCueSpeed)
        {
            loadXMLSpeed(tag);
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unrecognized Cue tag:" << tag.tagName();
        }

        node = node.nextSibling();
    }

    return true;
}

bool Cue::saveXML(QDomDocument* doc, QDomElement* stack_root) const
{
    qDebug() << Q_FUNC_INFO;
    Q_ASSERT(doc != NULL);
    Q_ASSERT(stack_root != NULL);

    QDomElement root = doc->createElement(KXMLQLCCue);
    root.setAttribute(KXMLQLCCueName, name());
    stack_root->appendChild(root);

    QHashIterator <uint,uchar> it(values());
    while (it.hasNext() == true)
    {
        it.next();
        QDomElement e = doc->createElement(KXMLQLCCueValue);
        e.setAttribute(KXMLQLCCueValueChannel, it.key());
        QDomText t = doc->createTextNode(QString::number(it.value()));
        e.appendChild(t);
        root.appendChild(e);
    }

    saveXMLSpeed(doc, &root);

    return true;
}

bool Cue::loadXMLSpeed(const QDomElement& speedRoot)
{
    if (speedRoot.tagName() != KXMLQLCCueSpeed)
        return false;

    m_fadeInSpeed = speedRoot.attribute(KXMLQLCCueSpeedFadeIn).toUInt();
    m_fadeOutSpeed = speedRoot.attribute(KXMLQLCCueSpeedFadeOut).toUInt();
    m_duration = speedRoot.attribute(KXMLQLCCueSpeedDuration).toUInt();

    return true;
}

bool Cue::saveXMLSpeed(QDomDocument* doc, QDomElement* root) const
{
    QDomElement tag;

    tag = doc->createElement(KXMLQLCCueSpeed);
    tag.setAttribute(KXMLQLCCueSpeedFadeIn, QString::number(fadeInSpeed()));
    tag.setAttribute(KXMLQLCCueSpeedFadeOut, QString::number(fadeOutSpeed()));
    tag.setAttribute(KXMLQLCCueSpeedDuration, QString::number(duration()));
    root->appendChild(tag);

    return true;
}
