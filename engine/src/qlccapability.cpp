/*
  Q Light Controller Plus
  qlccapability.cpp

  Copyright (C) Heikki Junnila
                Massimo Callegari

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

#include <QCoreApplication>
#include <QXmlStreamReader>
#include <QString>
#include <QDebug>
#include <QFile>

#include "qlccapability.h"
#include "qlcmacros.h"
#include "qlcconfig.h"
#include "qlcfile.h"

/************************************************************************
 * Initialization
 ************************************************************************/

QLCCapability::QLCCapability(uchar min, uchar max, const QString& name,
                             const QString &resource, const QColor &color1,
                             const QColor &color2, QObject *parent)
    : QObject(parent)
    , m_min(min)
    , m_max(max)
    , m_name(name)
    , m_resourceName(resource)
    , m_resourceColor1(color1)
    , m_resourceColor2(color2)
{
}

QLCCapability *QLCCapability::createCopy()
{
    QLCCapability* copy = new QLCCapability(m_min, m_max, m_name, m_resourceName,
                                            m_resourceColor1, m_resourceColor2);
    return copy;
}

QLCCapability::~QLCCapability()
{
}

QLCCapability& QLCCapability::operator=(const QLCCapability& capability)
{
    if (this != &capability)
    {
        m_min = capability.m_min;
        m_max = capability.m_max;
        m_name = capability.m_name;
        m_resourceName = capability.m_resourceName;
        m_resourceColor1 = capability.m_resourceColor1;
        m_resourceColor2 = capability.m_resourceColor2;
    }

    return *this;
}

bool QLCCapability::operator<(const QLCCapability& capability) const
{
    if (m_min < capability.m_min)
        return true;
    else
        return false;
}

/************************************************************************
 * Properties
 ************************************************************************/

uchar QLCCapability::min() const
{
    return m_min;
}

void QLCCapability::setMin(uchar value)
{
    m_min = value;
}

uchar QLCCapability::max() const
{
    return m_max;
}

void QLCCapability::setMax(uchar value)
{
    m_max = value;
}

uchar QLCCapability::middle() const
{
    return int((m_max + m_min) / 2);
}

QString QLCCapability::name() const
{
    return m_name;
}

void QLCCapability::setName(const QString& name)
{
    m_name = name;
}

QString QLCCapability::resourceName()
{
    return m_resourceName;
}

void QLCCapability::setResourceName(const QString& name)
{
    m_resourceName = name;
    // invalidate any previous color set
    m_resourceColor1 = QColor();
    m_resourceColor2 = QColor();
}

QColor QLCCapability::resourceColor1()
{
    return m_resourceColor1;
}

QColor QLCCapability::resourceColor2()
{
    return m_resourceColor2;
}

void QLCCapability::setResourceColors(QColor col1, QColor col2)
{
    m_resourceColor1 = col1;
    m_resourceColor2 = col2;
    // invalidate any previous resource path set
    m_resourceName = "";
}

bool QLCCapability::overlaps(const QLCCapability *cap)
{
    if (m_min >= cap->min() && m_min <= cap->max())
        return true;
    else if (m_max >= cap->min() && m_max <= cap->max())
        return true;
    else if (m_min <= cap->min() && m_max >= cap->min())
        return true;
    else
        return false;
}

/************************************************************************
 * Save & Load
 ************************************************************************/

bool QLCCapability::saveXML(QXmlStreamWriter *doc)
{
    Q_ASSERT(doc != NULL);

    /* QLCCapability entry */
    doc->writeStartElement(KXMLQLCCapability);

    /* Min limit attribute */
    doc->writeAttribute(KXMLQLCCapabilityMin, QString::number(m_min));

    /* Max limit attribute */
    doc->writeAttribute(KXMLQLCCapabilityMax, QString::number(m_max));

    /* Resource file attribute */
    if (m_resourceName.isEmpty() == false)
    {
        QString modFilename = m_resourceName;
        QDir dir = QDir::cleanPath(QLCFile::systemDirectory(GOBODIR).path());

        if (modFilename.contains(dir.path()))
        {
            modFilename.remove(dir.path());
            // The following line is a dirty workaround for an issue raised on Windows
            // When building with MinGW, dir.path() is something like "C:/QLC+/Gobos"
            // while QDir::separator() returns "\"
            // So, to avoid any string mismatch I remove the first character
            // no matter what it is
            modFilename.remove(0, 1);
        }

        doc->writeAttribute(KXMLQLCCapabilityResource, modFilename);
    }
    if (m_resourceColor1.isValid())
    {
        doc->writeAttribute(KXMLQLCCapabilityColor1, m_resourceColor1.name());
    }
    if (m_resourceColor2.isValid())
    {
        doc->writeAttribute(KXMLQLCCapabilityColor2, m_resourceColor2.name());
    }

    /* Name */
    doc->writeCharacters(m_name);
    doc->writeEndElement();

    return true;
}

bool QLCCapability::loadXML(QXmlStreamReader &doc)
{
    uchar min = 0;
    uchar max = 0;
    QString str;

    if (doc.name() != KXMLQLCCapability)
    {
        qWarning() << Q_FUNC_INFO << "Capability node not found";
        return false;
    }

    /* Get low limit attribute (critical) */
    QXmlStreamAttributes attrs = doc.attributes();
    str = attrs.value(KXMLQLCCapabilityMin).toString();
    if (str.isEmpty() == true)
    {
        qWarning() << Q_FUNC_INFO << "Capability has no minimum limit.";
        return false;
    }
    else
    {
        min = CLAMP(str.toInt(), 0, (int)UCHAR_MAX);
    }

    /* Get high limit attribute (critical) */
    str = attrs.value(KXMLQLCCapabilityMax).toString();
    if (str.isEmpty() == true)
    {
        qWarning() << Q_FUNC_INFO << "Capability has no maximum limit.";
        return false;
    }
    else
    {
        max = CLAMP(str.toInt(), 0, (int)UCHAR_MAX);
    }

    /* Get (optional) resource name for gobo/effect/... */
    if(attrs.hasAttribute(KXMLQLCCapabilityResource))
    {
        QString path = attrs.value(KXMLQLCCapabilityResource).toString();
        if (QFileInfo(path).isRelative())
        {
            QDir dir = QLCFile::systemDirectory(GOBODIR);
            path = dir.path() + QDir::separator() + path;
        }
        setResourceName(path);
    }

    /* Get (optional) color resource for color presets */
    if (attrs.hasAttribute(KXMLQLCCapabilityColor1))
    {
        QColor col1 = QColor(attrs.value(KXMLQLCCapabilityColor1).toString());
        QColor col2 = QColor();
        if (attrs.hasAttribute(KXMLQLCCapabilityColor2))
            col2 = QColor(attrs.value(KXMLQLCCapabilityColor2).toString());
        if (col1.isValid())
            setResourceColors(col1, col2);
    }

    if (min <= max)
    {
        setName(doc.readElementText());
        setMin(min);
        setMax(max);

        return true;
    }
    else
    {
        qWarning() << Q_FUNC_INFO << "Capability min(" << min
                   << ") is greater than max(" << max << ")";
        return false;
    }
}

