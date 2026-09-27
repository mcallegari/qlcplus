/*
  Q Light Controller Plus
  vcanimationpreset.cpp

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

#include "vcanimationpreset.h"

VCAnimationPreset::VCAnimationPreset(quint8 id)
    : m_id(id)
    , m_type(Color1)
{
}

VCAnimationPreset::VCAnimationPreset(const VCAnimationPreset &other)
{
    *this = other;
}

VCAnimationPreset &VCAnimationPreset::operator=(const VCAnimationPreset &other)
{
    if (this != &other)
    {
        m_id = other.m_id;
        m_type = other.m_type;
        m_color = other.m_color;
        m_resource = other.m_resource;
        m_properties = other.m_properties;
    }

    return *this;
}

VCAnimationPreset::~VCAnimationPreset()
{
}

VCAnimationPreset::WidgetType VCAnimationPreset::widgetType() const
{
    switch (m_type)
    {
    case Color1Knob:
    case Color2Knob:
    case Color3Knob:
    case Color4Knob:
    case Color5Knob:
        return Knob;
    default:
        return Button;
    }
}

int VCAnimationPreset::colorIndex() const
{
    switch (m_type)
    {
    case Color1: case Color1Knob: case Color1Reset: return 0;
    case Color2: case Color2Knob: case Color2Reset: return 1;
    case Color3: case Color3Knob: case Color3Reset: return 2;
    case Color4: case Color4Knob: case Color4Reset: return 3;
    case Color5: case Color5Knob: case Color5Reset: return 4;
    default: return -1;
    }
}

quint8 VCAnimationPreset::rgbToValue(QRgb color) const
{
    if (m_color == Qt::red)
        return QColor(color).red();
    if (m_color == Qt::green)
        return QColor(color).green();
    if (m_color == Qt::blue)
        return QColor(color).blue();

    return 0;
}

QRgb VCAnimationPreset::valueToRgb(quint8 value) const
{
    if (m_color == Qt::red)
        return qRgb(value, 0, 0);
    if (m_color == Qt::green)
        return qRgb(0, value, 0);
    if (m_color == Qt::blue)
        return qRgb(0, 0, value);

    return 0;
}

QString VCAnimationPreset::typeToString(VCAnimationPreset::ControlType type)
{
    switch (type)
    {
    case Color1: return "Color1";
    case Color2: return "Color2";
    case Color3: return "Color3";
    case Color4: return "Color4";
    case Color5: return "Color5";
    case Color1Reset: return "ResetColor1";
    case Color2Reset: return "ResetColor2";
    case Color3Reset: return "ResetColor3";
    case Color4Reset: return "ResetColor4";
    case Color5Reset: return "ResetColor5";
    case Animation: return "Animation";
    case Text: return "Text";
    case Color1Knob: return "Color1Knob";
    case Color2Knob: return "Color2Knob";
    case Color3Knob: return "Color3Knob";
    case Color4Knob: return "Color4Knob";
    case Color5Knob: return "Color5Knob";
    }
    return QString();
}

VCAnimationPreset::ControlType VCAnimationPreset::stringToType(const QString &str)
{
    if (str == "Color1" || str == "StartColor") return Color1;
    else if (str == "Color2" || str == "EndColor") return Color2;
    else if (str == "Color3") return Color3;
    else if (str == "Color4") return Color4;
    else if (str == "Color5") return Color5;
    else if (str == "ResetColor1") return Color1Reset;
    else if (str == "ResetColor2") return Color2Reset;
    else if (str == "ResetColor3") return Color3Reset;
    else if (str == "ResetColor4") return Color4Reset;
    else if (str == "ResetColor5") return Color5Reset;
    else if (str == "Animation") return Animation;
    else if (str == "Text") return Text;
    else if (str == "Color1Knob" || str == "StartColorKnob") return Color1Knob;
    else if (str == "Color2Knob" || str == "EndColorKnob") return Color2Knob;
    else if (str == "Color3Knob") return Color3Knob;
    else if (str == "Color4Knob") return Color4Knob;
    else if (str == "Color5Knob") return Color5Knob;
    else
        return Color1;
}

bool VCAnimationPreset::operator<(const VCAnimationPreset &right) const
{
    return m_id < right.m_id;
}

bool VCAnimationPreset::compare(const VCAnimationPreset *left, const VCAnimationPreset *right)
{
    return *left < *right;
}

bool VCAnimationPreset::loadXML(QXmlStreamReader &root)
{
    if (root.name() != KXMLQLCVCAnimationPreset)
    {
        qWarning() << Q_FUNC_INFO << "Animation control node not found";
        return false;
    }

    if (root.attributes().hasAttribute(KXMLQLCVCAnimationPresetID) == false)
    {
        qWarning() << Q_FUNC_INFO << "Animation control ID not found";
        return false;
    }

    m_id = root.attributes().value(KXMLQLCVCAnimationPresetID).toString().toUInt();

    while (root.readNextStartElement())
    {
        if (root.name() == KXMLQLCVCAnimationPresetType)
        {
            m_type = stringToType(root.readElementText());
        }
        else if (root.name() == KXMLQLCVCAnimationPresetColor)
        {
            m_color = QColor(root.readElementText());
        }
        else if (root.name() == KXMLQLCVCAnimationPresetResource)
        {
            m_resource = root.readElementText();
        }
        else if (root.name() == KXMLQLCVCAnimationPresetProperty)
        {
            if (root.attributes().hasAttribute(KXMLQLCVCAnimationPresetPropertyName))
            {
                QString pName = root.attributes().value(KXMLQLCVCAnimationPresetPropertyName).toString();
                QString pValue = root.readElementText();
                m_properties[pName] = pValue;
            }
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown VCAnimationPreset tag:" << root.name().toString();
            root.skipCurrentElement();
        }
    }

    return true;
}

bool VCAnimationPreset::saveXML(QXmlStreamWriter *doc) const
{
    Q_ASSERT(doc != nullptr);

    doc->writeStartElement(KXMLQLCVCAnimationPreset);
    doc->writeAttribute(KXMLQLCVCAnimationPresetID, QString::number(m_id));

    doc->writeTextElement(KXMLQLCVCAnimationPresetType, typeToString(m_type));

    /* Color/Knob presets carry a color; all other types carry a resource
     * (matches the legacy VCMatrixControl serialization) */
    switch (m_type)
    {
    case Color1: case Color2: case Color3: case Color4: case Color5:
    case Color1Knob: case Color2Knob: case Color3Knob: case Color4Knob: case Color5Knob:
        doc->writeTextElement(KXMLQLCVCAnimationPresetColor, m_color.name());
        break;
    default:
        doc->writeTextElement(KXMLQLCVCAnimationPresetResource, m_resource);
        break;
    }

    if (!m_properties.isEmpty())
    {
        QMapIterator<QString, QString> it(m_properties);
        while (it.hasNext())
        {
            it.next();
            doc->writeStartElement(KXMLQLCVCAnimationPresetProperty);
            doc->writeAttribute(KXMLQLCVCAnimationPresetPropertyName, it.key());
            doc->writeCharacters(it.value());
            doc->writeEndElement();
        }
    }

    /* End the <Control> tag */
    doc->writeEndElement();

    return true;
}
