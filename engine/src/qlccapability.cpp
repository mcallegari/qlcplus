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
#include <QMetaEnum>
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

QLCCapability::QLCCapability(uchar min, uchar max, const QString& name, QObject *parent)
    : QObject(parent)
    , m_preset(Custom)
    , m_min(min)
    , m_max(max)
    , m_name(name)
{
}

QLCCapability *QLCCapability::createCopy()
{
    QLCCapability *copy = new QLCCapability(m_min, m_max, m_name);
    copy->setPreset(preset());
    for (int i = 0; i < m_resources.count(); i++)
        copy->setResource(i, m_resources.at(i));
    foreach (AliasInfo alias, m_aliases)
        copy->addAlias(alias);

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
        m_preset = capability.m_preset;
        m_resources = capability.m_resources;
        m_aliases = capability.m_aliases;
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

QString QLCCapability::presetToString(QLCCapability::Preset preset)
{
    int index = staticMetaObject.indexOfEnumerator("Preset");
    return staticMetaObject.enumerator(index).valueToKey(preset);
}

QLCCapability::Preset QLCCapability::stringToPreset(const QString &preset)
{
    int index = staticMetaObject.indexOfEnumerator("Preset");
    return Preset(staticMetaObject.enumerator(index).keyToValue(preset.toStdString().c_str()));
}

QLCCapability::Preset QLCCapability::preset() const
{
    return m_preset;
}

void QLCCapability::setPreset(QLCCapability::Preset preset)
{
    if (preset == m_preset)
        return;

    m_preset = preset;
}

QLCCapability::PresetType QLCCapability::presetType() const
{
    switch (m_preset)
    {
        case StrobeFreq:
        case PulseFreq:
            return SingleValue;
        case StrobeFreqRange:
        case PulseFreqRange:
            return DoubleValue;
        case ColorMacro:
            return SingleColor;
        case ColorDoubleMacro:
            return DoubleColor;
        case GoboMacro:
        case GenericPicture:
            return Picture;
        default: return None;
    }
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

QVariant QLCCapability::resource(int index)
{
    if (index < 0 || index >= m_resources.count())
        return QVariant();

    return m_resources.at(index);
}

void QLCCapability::setResource(int index, QVariant value)
{
    if (index < 0)
        return;
    else if (index < m_resources.count())
        m_resources[index] = value;
    else
        m_resources.append(value);
}

QVariantList QLCCapability::resources()
{
    return m_resources;
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

/********************************************************************
 * Aliases
 ********************************************************************/

QList<AliasInfo> QLCCapability::aliasList()
{
    return m_aliases;
}

void QLCCapability::addAlias(AliasInfo alias)
{
    m_aliases.append(alias);
}

void QLCCapability::removeAlias(AliasInfo alias)
{
    for (int i = 0; i < m_aliases.count(); i++)
    {
        AliasInfo info = m_aliases.at(i);

        if (alias.targetMode == info.targetMode &&
            alias.sourceChannel == info.sourceChannel &&
            alias.targetChannel == info.targetChannel)
        {
            m_aliases.takeAt(i);
            return;
        }
    }
}

void QLCCapability::replaceAliases(QList<AliasInfo> list)
{
    m_aliases.clear();
    foreach (AliasInfo info, list)
        m_aliases.append(info);
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

    /* Preset attribute if not custom */
    if (m_preset != Custom)
        doc->writeAttribute(KXMLQLCCapabilityPreset, presetToString(m_preset));

    /* First resource attribute */
    if (resource(0).isValid())
    {
        if (presetType() == Picture)
        {
            QString modFilename = resource(0).toString();
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

            doc->writeAttribute(KXMLQLCCapabilityRes1, modFilename);
        }
        else if (presetType() == SingleColor || presetType() == DoubleColor)
        {
            QColor col = resource(0).value<QColor>();
            if (col.isValid())
                doc->writeAttribute(KXMLQLCCapabilityRes1, col.name());
        }
    }
    if (resource(0).isValid())
    {
        if (presetType() == DoubleColor)
        {
            QColor col = resource(1).value<QColor>();
            if (col.isValid())
                doc->writeAttribute(KXMLQLCCapabilityRes2, col.name());
        }
    }

    /* Name */
    if (m_aliases.isEmpty())
        doc->writeCharacters(m_name);
    else
        doc->writeCharacters(QString("%1\n   ").arg(m_name)); // to preserve indentation

    /* Aliases */
    foreach (AliasInfo info, m_aliases)
    {
        doc->writeStartElement(KXMLQLCCapabilityAlias);
        doc->writeAttribute(KXMLQLCCapabilityAliasMode, info.targetMode);
        doc->writeAttribute(KXMLQLCCapabilityAliasSourceName, info.sourceChannel);
        doc->writeAttribute(KXMLQLCCapabilityAliasTargetName, info.targetChannel);
        doc->writeEndElement();
    }

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

    if (attrs.hasAttribute(KXMLQLCCapabilityPreset))
    {
        str = attrs.value(KXMLQLCCapabilityPreset).toString();
        setPreset(stringToPreset(str));
    }

    /* Get (optional) resource name for gobo/effect/... */
    if(attrs.hasAttribute(KXMLQLCCapabilityResource))
    {
        QString path = attrs.value(KXMLQLCCapabilityResource).toString();
        if (QFileInfo(path).isRelative())
        {
            QDir dir = QLCFile::systemDirectory(GOBODIR);
            path = dir.path() + QDir::separator() + path;
            setPreset(GoboMacro);
        }
        else
            setPreset(GenericPicture);
        setResource(0, path);
    }

    /* Get (optional) color resource for color presets */
    if (attrs.hasAttribute(KXMLQLCCapabilityColor1))
    {
        QColor col1 = QColor(attrs.value(KXMLQLCCapabilityColor1).toString());
        QColor col2 = QColor();
        if (attrs.hasAttribute(KXMLQLCCapabilityColor2))
            col2 = QColor(attrs.value(KXMLQLCCapabilityColor2).toString());

        if (col1.isValid())
        {
            setResource(0, col1);

            if (col2.isValid())
            {
                setResource(1, col2);
                setPreset(ColorDoubleMacro);
            }
            else
            {
                setPreset(ColorMacro);
            }
        }
    }

    if (min <= max)
    {
        doc.readNext();
        setName(doc.text().toString().simplified());
        setMin(min);
        setMax(max);
    }
    else
    {
        qWarning() << Q_FUNC_INFO << "Capability min(" << min
                   << ") is greater than max(" << max << ")";
        return false;
    }

    /* Subtags */
    while (doc.readNextStartElement())
    {
        if (doc.name() == KXMLQLCCapabilityAlias)
        {
            AliasInfo alias;
            QXmlStreamAttributes attrs = doc.attributes();

            alias.targetMode = attrs.value(KXMLQLCCapabilityAliasMode).toString();
            alias.sourceChannel = attrs.value(KXMLQLCCapabilityAliasSourceName).toString();
            alias.targetChannel = attrs.value(KXMLQLCCapabilityAliasTargetName).toString();
            addAlias(alias);

            //qDebug() << "Alias found for mode" << alias.targetMode;
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown capability tag: " << doc.name();
        }
        doc.skipCurrentElement();
    }

    return true;
}

