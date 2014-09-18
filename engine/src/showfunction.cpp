/*
  Q Light Controller Plus
  showfunction.cpp

  Copyright (c) Massimo Callegari

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

#include <QDomDocument>
#include <QDomElement>
#include <QDebug>

#include "showfunction.h"
#include "function.h"

#define KXMLShowFunctionID "ID"
#define KXMLShowFunctionStartTime "StartTime"
#define KXMLShowFunctionDuration "Duration"
#define KXMLShowFunctionColor "Color"
#define KXMLShowFunctionLocked "Locked"

ShowFunction::ShowFunction()
{
    m_id = Function::invalidId();
    m_startTime = UINT_MAX;
    m_duration = 0;
    m_color = QColor();
    m_locked = false;
}

void ShowFunction::setFunctionID(quint32 id)
{
    m_id = id;
}

quint32 ShowFunction::functionID() const
{
    return m_id;
}

void ShowFunction::setStartTime(quint32 time)
{
    m_startTime = time;
}

quint32 ShowFunction::startTime() const
{
    return m_startTime;
}

void ShowFunction::setDuration(quint32 duration)
{
    m_duration = duration;
}

quint32 ShowFunction::duration() const
{
    return m_duration;
}

void ShowFunction::setColor(QColor color)
{
    m_color = color;
}

QColor ShowFunction::color() const
{
    return m_color;
}

QColor ShowFunction::defaultColor(Function::Type type)
{
    switch (type)
    {
        case Function::Chaser:
            return QColor(85, 107, 128);
        break;
        case Function::Audio:
            return QColor(96, 128, 83);
        break;
        case Function::RGBMatrix:
            return QColor(101, 155, 155);
        break;
        case Function::EFX:
            return QColor(128, 60, 60);
        break;
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
        case Function::Video:
            return QColor(147, 140, 20);
        break;
#endif
        default:
            return QColor(100, 100, 100);
        break;
    }
}

void ShowFunction::setLocked(bool locked)
{
    m_locked = locked;
}

bool ShowFunction::isLocked() const
{
    return m_locked;
}

/************************************************************************
 * Load & Save
 ***********************************************************************/

bool ShowFunction::loadXML(const QDomElement &root)
{
    if (root.tagName() != KXMLShowFunction)
    {
        qWarning() << Q_FUNC_INFO << "ShowFunction node not found";
        return false;
    }

    if (root.hasAttribute(KXMLShowFunctionID))
        setFunctionID(root.attribute(KXMLShowFunctionID).toUInt());
    if (root.hasAttribute(KXMLShowFunctionStartTime))
        setStartTime(root.attribute(KXMLShowFunctionStartTime).toUInt());
    if (root.hasAttribute(KXMLShowFunctionDuration))
        setDuration(root.attribute(KXMLShowFunctionDuration).toUInt());
    if (root.hasAttribute(KXMLShowFunctionColor))
        setColor(QColor(root.attribute(KXMLShowFunctionColor)));
    if (root.hasAttribute(KXMLShowFunctionLocked))
        setLocked(true);

    return true;
}

bool ShowFunction::saveXML(QDomDocument *doc, QDomElement *root) const
{
    QDomElement tag;

    if (doc == NULL || root == NULL)
        return false;

    /* Main tag */
    tag = doc->createElement(KXMLShowFunction);
    root->appendChild(tag);

    /* Attributes */
    tag.setAttribute(KXMLShowFunctionID, functionID());
    tag.setAttribute(KXMLShowFunctionStartTime, startTime());
    tag.setAttribute(KXMLShowFunctionDuration, duration());
    if (color().isValid())
        tag.setAttribute(KXMLShowFunctionColor, color().name());
    if (isLocked())
        tag.setAttribute(KXMLShowFunctionLocked, m_locked);

    return true;
}
