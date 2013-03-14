/*
  Q Light Controller
  qlccapability.cpp

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

#include <QCoreApplication>
#include <QString>
#include <QDebug>
#include <QFile>
#include <QtXml>

#include "qlccapability.h"
#include "qlcmacros.h"
#include "qlcconfig.h"

/************************************************************************
 * Initialization
 ************************************************************************/

QLCCapability::QLCCapability(uchar min, uchar max, const QString& name,
                             const QString &resource, const QColor &color1, const QColor &color2)
{
    m_min = min;
    m_max = max;
    m_name = name;
    m_resourceName = resource;
    m_resourceColor1 = color1;
    m_resourceColor2 = color2;
}

QLCCapability::QLCCapability(const QLCCapability* capability)
{
    m_min = 0;
    m_max = UCHAR_MAX;

    if (capability != NULL)
        *this = *capability;
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

bool QLCCapability::overlaps(const QLCCapability& cap)
{
    if (m_min >= cap.min() && m_min <= cap.max())
        return true;
    else if (m_max >= cap.min() && m_max <= cap.max())
        return true;
    else if (m_min <= cap.min() && m_max >= cap.min())
        return true;
    else
        return false;
}

/************************************************************************
 * Save & Load
 ************************************************************************/

bool QLCCapability::saveXML(QDomDocument* doc, QDomElement* root)
{
    QDomElement tag;
    QDomText text;
    QString str;

    Q_ASSERT(doc != NULL);
    Q_ASSERT(root != NULL);

    /* QLCCapability entry */
    tag = doc->createElement(KXMLQLCCapability);
    root->appendChild(tag);

    /* Min limit attribute */
    str.setNum(m_min);
    tag.setAttribute(KXMLQLCCapabilityMin, str);

    /* Max limit attribute */
    str.setNum(m_max);
    tag.setAttribute(KXMLQLCCapabilityMax, str);

    /* Resource file attribute */
    if (m_resourceName.isEmpty() == false)
    {
        QString modFilename = m_resourceName;
        QDir dir;
#ifdef __APPLE__
        dir.setPath(QString("%1/../%2").arg(QCoreApplication::applicationDirPath())
                    .arg(GOBODIR));
        dir = dir.cleanPath(dir.path());
#else
        dir.setPath(GOBODIR);
#endif
        if (modFilename.contains(dir.path()))
            modFilename.remove(dir.path() + QDir::separator() );

        tag.setAttribute(KXMLQLCCapabilityResource, modFilename);
    }
    if (m_resourceColor1.isValid())
    {
        tag.setAttribute(KXMLQLCCapabilityColor1, m_resourceColor1.name());
    }
    if (m_resourceColor2.isValid())
    {
        tag.setAttribute(KXMLQLCCapabilityColor2, m_resourceColor2.name());
    }

    /* Name value */
    text = doc->createTextNode(m_name);
    tag.appendChild(text);

    return true;
}

bool QLCCapability::loadXML(const QDomElement& root)
{
    uchar min = 0;
    uchar max = 0;
    QString str;

    if (root.tagName() != KXMLQLCCapability)
    {
        qWarning() << Q_FUNC_INFO << "Capability node not found";
        return false;
    }

    /* Get low limit attribute (critical) */
    str = root.attribute(KXMLQLCCapabilityMin);
    if (str.isEmpty() == true)
    {
        qWarning() << Q_FUNC_INFO << "Capability has no minimum limit.";
        return false;
    }
    else
    {
        min = CLAMP(str.toInt(), 0, UCHAR_MAX);
    }

    /* Get high limit attribute (critical) */
    str = root.attribute(KXMLQLCCapabilityMax);
    if (str.isEmpty() == true)
    {
        qWarning() << Q_FUNC_INFO << "Capability has no maximum limit.";
        return false;
    }
    else
    {
        max = CLAMP(str.toInt(), 0, UCHAR_MAX);
    }

    /* Get (optional) resource name for gobo/effect/... */
    if(root.hasAttribute(KXMLQLCCapabilityResource))
    {
        QString path = root.attribute(KXMLQLCCapabilityResource);
        if (QFileInfo(path).isRelative())
        {
#ifdef __APPLE__
            QDir dir;
            dir.setPath(QString("%1/../%2").arg(QCoreApplication::applicationDirPath())
                        .arg(GOBODIR));
            path = dir.path() + QDir::separator() + path;
#else
            path = QString(GOBODIR) + QDir::separator() + path;
#endif
        }
        setResourceName(path);
    }

    /* Get (optional) color resource for color presets */
    if (root.hasAttribute(KXMLQLCCapabilityColor1))
    {
        QColor col1 = QColor(root.attribute(KXMLQLCCapabilityColor1));
        QColor col2 = QColor();
        if (root.hasAttribute(KXMLQLCCapabilityColor2))
            col2 = QColor(root.attribute(KXMLQLCCapabilityColor2));
        if (col1.isValid())
        {
            setResourceColors(col1, col2);
        }
    }

    if (min <= max)
    {
        setName(root.text());
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

