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

#include <QtGlobal>
#include <QtXml>

#include "vcmatrixcontrol.h"

VCMatrixControl::VCMatrixControl(quint8 id)
    : m_id(id)
{
    m_color = QColor();
    m_resource = QString();
    m_inputSource = NULL;
}

VCMatrixControl::VCMatrixControl(VCMatrixControl const& vcmc)
    : m_id(vcmc.m_id)
    , m_type(vcmc.m_type)
    , m_color(vcmc.m_color)
    , m_resource(vcmc.m_resource)
    , m_properties(vcmc.m_properties)
    , m_keySequence(vcmc.m_keySequence)
{
    if (vcmc.m_inputSource == NULL)
        m_inputSource = NULL;
    else
        m_inputSource = new QLCInputSource(vcmc.m_inputSource->universe(),
                                           vcmc.m_inputSource->channel());
}

VCMatrixControl::~VCMatrixControl()
{
    delete m_inputSource;
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

bool VCMatrixControl::loadXML(const QDomElement &root)
{
    QDomNode node;
    QDomElement tag;

    if (root.tagName() != KXMLQLCVCMatrixControl)
    {
        qWarning() << Q_FUNC_INFO << "Matrix control node not found";
        return false;
    }

    if (root.hasAttribute(KXMLQLCVCMatrixControlID) == false)
    {
        qWarning() << Q_FUNC_INFO << "Matrix control ID not found";
        return false;
    }

    m_id = root.attribute(KXMLQLCVCMatrixControlID).toUInt();

    /* Children */
    node = root.firstChild();
    while (node.isNull() == false)
    {
        tag = node.toElement();
        if (tag.tagName() == KXMLQLCVCMatrixControlType)
        {
            m_type = stringToType(tag.text());
        }
        else if (tag.tagName() == KXMLQLCVCMatrixControlColor)
        {
            m_color = QColor(tag.text());
        }
        else if (tag.tagName() == KXMLQLCVCMatrixControlResource)
        {
            m_resource = tag.text();
        }
        else if (tag.tagName() == KXMLQLCVCMatrixControlProperty)
        {
            if (tag.hasAttribute(KXMLQLCVCMatrixControlPropertyName))
            {
                QString pName = tag.attribute(KXMLQLCVCMatrixControlPropertyName);
                QString pValue = tag.text();
                m_properties[pName] = pValue;
            }
        }
        else if (tag.tagName() == KXMLQLCVCMatrixControlInput)
        {
            if (tag.hasAttribute(KXMLQLCVCMatrixControlInputUniverse) &&
                tag.hasAttribute(KXMLQLCVCMatrixControlInputChannel))
            {
                quint32 uni = tag.attribute(KXMLQLCVCMatrixControlInputUniverse).toUInt();
                quint32 ch = tag.attribute(KXMLQLCVCMatrixControlInputChannel).toUInt();
                m_inputSource = new QLCInputSource(uni, ch);
            }
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown VCMatrixControl tag:" << tag.tagName();
        }

        node = node.nextSibling();
    }

    return true;
}

bool VCMatrixControl::saveXML(QDomDocument *doc, QDomElement *mtx_root)
{
    QDomElement root;
    QDomElement tag;
    QDomText text;

    Q_ASSERT(doc != NULL);
    Q_ASSERT(mtx_root != NULL);

    root = doc->createElement(KXMLQLCVCMatrixControl);
    root.setAttribute(KXMLQLCVCMatrixControlID, m_id);
    mtx_root->appendChild(root);

    tag = doc->createElement(KXMLQLCVCMatrixControlType);
    root.appendChild(tag);
    text = doc->createTextNode(typeToString(m_type));
    tag.appendChild(text);

    if (m_type == StartColor || m_type == EndColor || m_type == StartColorKnob || m_type == EndColorKnob)
    {
        tag = doc->createElement(KXMLQLCVCMatrixControlColor);
        root.appendChild(tag);
        text = doc->createTextNode(m_color.name());
        tag.appendChild(text);
    }
    else
    {
        tag = doc->createElement(KXMLQLCVCMatrixControlResource);
        root.appendChild(tag);
        text = doc->createTextNode(m_resource);
        tag.appendChild(text);
    }

    if (!m_properties.isEmpty())
    {
        QHashIterator<QString, QString> it(m_properties);
        while(it.hasNext())
        {
            it.next();
            tag = doc->createElement(KXMLQLCVCMatrixControlProperty);
            tag.setAttribute(KXMLQLCVCMatrixControlPropertyName, it.key());
            root.appendChild(tag);
            text = doc->createTextNode(it.value());
            tag.appendChild(text);
        }
    }

    /* External input source */
    if (m_inputSource != NULL && m_inputSource->isValid())
    {
        tag = doc->createElement(KXMLQLCVCMatrixControlInput);
        tag.setAttribute(KXMLQLCVCMatrixControlInputUniverse, QString("%1").arg(m_inputSource->universe()));
        tag.setAttribute(KXMLQLCVCMatrixControlInputChannel, QString("%1").arg(m_inputSource->channel()));
        root.appendChild(tag);
    }

    /* Key sequence */
    if (m_keySequence.isEmpty() == false)
    {
        tag = doc->createElement(KXMLQLCVCMatrixControlKey);
        root.appendChild(tag);
        text = doc->createTextNode(m_keySequence.toString());
        tag.appendChild(text);
    }

    return true;
}

