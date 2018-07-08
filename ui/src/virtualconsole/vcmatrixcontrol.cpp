/*
  Q Light Controller Plus
  vcmatrixcontrol.cpp

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

#include "vcmatrixcontrol.h"
#include "vcwidget.h"

VCMatrixControl::VCMatrixControl(quint8 id)
    : m_id(id)
{
    m_color = QColor();
    m_resource = QString();
}

VCMatrixControl::VCMatrixControl(VCMatrixControl const& vcmc)
    : m_id(vcmc.m_id)
    , m_type(vcmc.m_type)
    , m_color(vcmc.m_color)
    , m_resource(vcmc.m_resource)
    , m_properties(vcmc.m_properties)
    , m_keySequence(vcmc.m_keySequence)
{
    if (vcmc.m_inputSource != NULL)
    {
        m_inputSource = QSharedPointer<QLCInputSource>(new QLCInputSource(vcmc.m_inputSource->universe(),
                                               vcmc.m_inputSource->channel()));
        m_inputSource->setRange(vcmc.m_inputSource->lowerValue(), vcmc.m_inputSource->upperValue());
    }
}

VCMatrixControl::~VCMatrixControl()
{
}

quint8 VCMatrixControl::rgbToValue(QRgb color) const
{
    if (m_color == Qt::red)
        return QColor(color).red();
    if (m_color == Qt::green)
        return QColor(color).green();
    if (m_color == Qt::blue)
        return QColor(color).blue();

    // We're never supposed to be here
    Q_ASSERT(false);
    return 0;
}

QRgb VCMatrixControl::valueToRgb(quint8 value) const
{
    if (m_color == Qt::red)
        return qRgb(value, 0, 0);
    if (m_color == Qt::green)
        return qRgb(0, value, 0);
    if (m_color == Qt::blue)
        return qRgb(0, 0, value);

    // We're never supposed to be here
    Q_ASSERT(false);
    return 0;
}

VCMatrixControl::WidgetType VCMatrixControl::widgetType() const
{
    switch(m_type)
    {
        case StartColor:
        case EndColor:
        case Animation:
        case Image:
        case Text:
        case ResetEndColor:
            return Button;
        case StartColorKnob:
        case EndColorKnob:
            return Knob;
    }

    // We're never supposed to be here
    Q_ASSERT(false);
    return Button;
}

QString VCMatrixControl::typeToString(VCMatrixControl::ControlType type)
{
    switch(type)
    {
        case StartColor: return "StartColor"; break;
        case EndColor: return "EndColor"; break;
        case ResetEndColor: return "ResetEndColor"; break;
        case Animation: return "Animation"; break;
        case Image: return "Image"; break;
        case Text: return "Text"; break;
        case StartColorKnob: return "StartColorKnob"; break;
        case EndColorKnob: return "EndColorKnob"; break;
    }
    return QString();
}

VCMatrixControl::ControlType VCMatrixControl::stringToType(QString str)
{
    if (str == "StartColor") return StartColor;
    else if (str == "EndColor") return EndColor;
    else if (str == "ResetEndColor") return ResetEndColor;
    else if (str == "Animation") return Animation;
    else if (str == "Image") return Image;
    else if (str == "Text") return Text;
    else if (str == "StartColorKnob") return StartColorKnob;
    else if (str == "EndColorKnob") return EndColorKnob;
    else
        return StartColor;
}

bool VCMatrixControl::operator<(VCMatrixControl const& right) const
{
    return m_id < right.m_id;
}

bool VCMatrixControl::compare(VCMatrixControl const* left, VCMatrixControl const* right)
{
    return *left < *right;
}

bool VCMatrixControl::loadXML(QXmlStreamReader &root)
{
    if (root.name() != KXMLQLCVCMatrixControl)
    {
        qWarning() << Q_FUNC_INFO << "Matrix control node not found";
        return false;
    }

    if (root.attributes().hasAttribute(KXMLQLCVCMatrixControlID) == false)
    {
        qWarning() << Q_FUNC_INFO << "Matrix control ID not found";
        return false;
    }

    m_id = root.attributes().value(KXMLQLCVCMatrixControlID).toString().toUInt();

    /* Children */
    while (root.readNextStartElement())
    {
        if (root.name() == KXMLQLCVCMatrixControlType)
        {
            m_type = stringToType(root.readElementText());
        }
        else if (root.name() == KXMLQLCVCMatrixControlColor)
        {
            m_color = QColor(root.readElementText());
        }
        else if (root.name() == KXMLQLCVCMatrixControlResource)
        {
            m_resource = root.readElementText();
        }
        else if (root.name() == KXMLQLCVCMatrixControlProperty)
        {
            if (root.attributes().hasAttribute(KXMLQLCVCMatrixControlPropertyName))
            {
                QString pName = root.attributes().value(KXMLQLCVCMatrixControlPropertyName).toString();
                QString pValue = root.readElementText();
                m_properties[pName] = pValue;
            }
        }
        else if (root.name() == KXMLQLCVCWidgetInput)
        {
            m_inputSource = VCWidget::getXMLInput(root);
            root.skipCurrentElement();
        }
        else if (root.name() == KXMLQLCVCWidgetKey)
        {
            m_keySequence = VCWidget::stripKeySequence(QKeySequence(root.readElementText()));
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown VCMatrixControl tag:" << root.name().toString();
            root.skipCurrentElement();
        }
    }

    return true;
}

bool VCMatrixControl::saveXML(QXmlStreamWriter *doc)
{
    Q_ASSERT(doc != NULL);

    doc->writeStartElement(KXMLQLCVCMatrixControl);
    doc->writeAttribute(KXMLQLCVCMatrixControlID, QString::number(m_id));

    doc->writeTextElement(KXMLQLCVCMatrixControlType, typeToString(m_type));

    if (m_type == StartColor || m_type == EndColor || m_type == StartColorKnob || m_type == EndColorKnob)
    {
        doc->writeTextElement(KXMLQLCVCMatrixControlColor, m_color.name());
    }
    else
    {
        doc->writeTextElement(KXMLQLCVCMatrixControlResource, m_resource);
    }

    if (!m_properties.isEmpty())
    {
        QHashIterator<QString, QString> it(m_properties);
        while(it.hasNext())
        {
            it.next();
            doc->writeStartElement(KXMLQLCVCMatrixControlProperty);
            doc->writeAttribute(KXMLQLCVCMatrixControlPropertyName, it.key());
            doc->writeCharacters(it.value());
            doc->writeEndElement();
        }
    }

    /* External input source */
    if (!m_inputSource.isNull() && m_inputSource->isValid())
        VCWidget::saveXMLInput(doc, m_inputSource);

    /* Key sequence */
    if (m_keySequence.isEmpty() == false)
        doc->writeTextElement(KXMLQLCVCWidgetKey, m_keySequence.toString());

    /* End the <Control> tag */
    doc->writeEndElement();

    return true;
}

