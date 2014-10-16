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

VCMatrixControl::VCMatrixControl(const VCMatrixControl *vcmc)
    : m_id(vcmc->m_id)
    , m_type(vcmc->m_type)
    , m_color(vcmc->m_color)
    , m_resource(vcmc->m_resource)
    , m_keySequence(vcmc->m_keySequence)
{
    if (vcmc->m_inputSource == NULL)
        m_inputSource = NULL;
    else
        m_inputSource = new QLCInputSource(vcmc->m_inputSource->universe(),
                                           vcmc->m_inputSource->channel());
}

VCMatrixControl::~VCMatrixControl()
{
    if (m_inputSource != NULL)
        delete m_inputSource;
}

QString VCMatrixControl::typeToString(VCMatrixControl::ControlType type)
{
    switch(type)
    {
        case StartColor: return "StartColor"; break;
        case EndColor: return "EndColor"; break;
        case Animation: return "Animation"; break;
        case Image: return "Image"; break;
        case Text: return "Text"; break;
    }
    return QString();
}

VCMatrixControl::ControlType VCMatrixControl::stringToType(QString str)
{
    if (str == "StartColor") return StartColor;
    else if (str == "EndColor") return EndColor;
    else if (str == "Animation") return Animation;
    else if (str == "Image") return Image;
    else if (str == "Text") return Text;
    else
        return StartColor;
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

    if (m_type == StartColor || m_type == EndColor)
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

