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

#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QDebug>

#include "showfunction.h"
#include "function.h"

#define KXMLShowFunctionID "ID"
#define KXMLShowFunctionStartTime "StartTime"
#define KXMLShowFunctionDuration "Duration"
#define KXMLShowFunctionColor "Color"
#define KXMLShowFunctionLocked "Locked"

ShowFunction::ShowFunction(QObject *parent)
    : QObject(parent)
{
    m_id = Function::invalidId();
    m_startTime = UINT_MAX;
    m_duration = 0;
    m_color = QColor();
    m_locked = false;
}

void ShowFunction::setFunctionID(quint32 id)
{
    if (id == m_id)
        return;

    m_id = id;
    emit functionIDChanged();
}

quint32 ShowFunction::functionID() const
{
    return m_id;
}

void ShowFunction::setStartTime(quint32 time)
{
    if (time == m_startTime)
        return;

    m_startTime = time;
    emit startTimeChanged();
}

quint32 ShowFunction::startTime() const
{
    return m_startTime;
}

void ShowFunction::setDuration(quint32 duration)
{
    if (duration == m_duration)
        return;

    m_duration = duration;
    emit durationChanged();
}

quint32 ShowFunction::duration() const
{
    return m_duration;
}

void ShowFunction::setColor(QColor color)
{
    if (color == m_color)
        return;

    m_color = color;
    emit colorChanged();
}

QColor ShowFunction::color() const
{
    return m_color;
}

QColor ShowFunction::defaultColor(Function::Type type)
{
    switch (type)
    {
        case Function::ChaserType:
            return QColor(85, 107, 128);
        break;
        case Function::AudioType:
            return QColor(96, 128, 83);
        break;
        case Function::RGBMatrixType:
            return QColor(101, 155, 155);
        break;
        case Function::EFXType:
            return QColor(128, 60, 60);
        break;
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
        case Function::VideoType:
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
    if (locked == m_locked)
        return;

    m_locked = locked;
    emit lockedChanged();
}

bool ShowFunction::isLocked() const
{
    return m_locked;
}

/************************************************************************
 * Load & Save
 ***********************************************************************/

bool ShowFunction::loadXML(QXmlStreamReader &root)
{
    if (root.name() != KXMLShowFunction)
    {
        qWarning() << Q_FUNC_INFO << "ShowFunction node not found";
        return false;
    }

    QXmlStreamAttributes attrs = root.attributes();

    if (attrs.hasAttribute(KXMLShowFunctionID))
        setFunctionID(attrs.value(KXMLShowFunctionID).toString().toUInt());
    if (attrs.hasAttribute(KXMLShowFunctionStartTime))
        setStartTime(attrs.value(KXMLShowFunctionStartTime).toString().toUInt());
    if (attrs.hasAttribute(KXMLShowFunctionDuration))
        setDuration(attrs.value(KXMLShowFunctionDuration).toString().toUInt());
    if (attrs.hasAttribute(KXMLShowFunctionColor))
        setColor(QColor(attrs.value(KXMLShowFunctionColor).toString()));
    if (attrs.hasAttribute(KXMLShowFunctionLocked))
        setLocked(true);

    root.skipCurrentElement();

    return true;
}

bool ShowFunction::saveXML(QXmlStreamWriter *doc) const
{
    Q_ASSERT(doc != NULL);

    /* Main tag */
    doc->writeStartElement(KXMLShowFunction);

    /* Attributes */
    doc->writeAttribute(KXMLShowFunctionID, QString::number(functionID()));
    doc->writeAttribute(KXMLShowFunctionStartTime, QString::number(startTime()));
    doc->writeAttribute(KXMLShowFunctionDuration, QString::number(duration()));
    if (color().isValid())
        doc->writeAttribute(KXMLShowFunctionColor, color().name());
    if (isLocked())
        doc->writeAttribute(KXMLShowFunctionLocked, QString::number(m_locked));

    doc->writeEndElement();

    return true;
}
