/*
  Q Light Controller Plus
  vcxypadpreset.cpp

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

#include <QtXml>

#include "vcxypadpreset.h"
#include "vcwidget.h"
#include "function.h"

VCXYPadPreset::VCXYPadPreset(quint8 id)
    : m_id(id)
    , m_dmxPos(QPointF())
    , m_efxID(Function::invalidId())
{

}

VCXYPadPreset::VCXYPadPreset(const VCXYPadPreset &vcpp)
    : m_id(vcpp.m_id)
    , m_type(vcpp.m_type)
    , m_dmxPos(vcpp.m_dmxPos)
    , m_efxID(vcpp.m_efxID)
    , m_keySequence(vcpp.m_keySequence)
{
    if (vcpp.m_inputSource != NULL)
    {
        m_inputSource = QSharedPointer<QLCInputSource>(new QLCInputSource(vcpp.m_inputSource->universe(),
                                               vcpp.m_inputSource->channel()));
        m_inputSource->setRange(vcpp.m_inputSource->lowerValue(), vcpp.m_inputSource->upperValue());
    }
}

VCXYPadPreset::~VCXYPadPreset()
{

}

void VCXYPadPreset::setEFXID(quint32 id)
{
    m_efxID = id;
}

quint32 VCXYPadPreset::efxID() const
{
    return m_efxID;
}

void VCXYPadPreset::setPosition(QPointF pos)
{
    m_dmxPos = pos;
}

QPointF VCXYPadPreset::position() const
{
    return m_dmxPos;
}

bool VCXYPadPreset::operator<(const VCXYPadPreset &right) const
{
    return m_id < right.m_id;
}

bool VCXYPadPreset::compare(const VCXYPadPreset *left, const VCXYPadPreset *right)
{
    return *left < *right;
}

QString VCXYPadPreset::typeToString(VCXYPadPreset::PresetType type)
{
    if (type == EFX)
        return "EFX";

    return "Position";
}

VCXYPadPreset::PresetType VCXYPadPreset::stringToType(QString str)
{
    if (str == "EFX")
        return EFX;

    return Position;
}

/************************************************************************
 * Load & Save
 ***********************************************************************/

bool VCXYPadPreset::loadXML(const QDomElement &root)
{
    QDomNode node;
    QDomElement tag;

    if (root.tagName() != KXMLQLCVCXYPadPreset)
    {
        qWarning() << Q_FUNC_INFO << "Matrix control node not found";
        return false;
    }

    if (root.hasAttribute(KXMLQLCVCXYPadPresetID) == false)
    {
        qWarning() << Q_FUNC_INFO << "XYPad Preset ID not found";
        return false;
    }

    m_id = root.attribute(KXMLQLCVCXYPadPresetID).toUInt();

    QPointF pos;
    bool hasPosition = false;

    /* Children */
    node = root.firstChild();
    while (node.isNull() == false)
    {
        tag = node.toElement();
        if (tag.tagName() == KXMLQLCVCXYPadPresetType)
        {
            m_type = stringToType(tag.text());
        }
        else if (tag.tagName() == KXMLQLCVCXYPadPresetEFXID)
        {
            setEFXID(tag.text().toUInt());
        }
        else if (tag.tagName() == KXMLQLCVCXYPadPresetXPos)
        {
            pos.setX(QString(tag.text()).toFloat());
            hasPosition = true;
        }
        else if (tag.tagName() == KXMLQLCVCXYPadPresetYPos)
        {
            pos.setY(QString(tag.text()).toFloat());
            hasPosition = true;
        }
        else if (tag.tagName() == KXMLQLCVCXYPadPresetInput)
        {
            if (tag.hasAttribute(KXMLQLCVCXYPadPresetInputUniverse) &&
                tag.hasAttribute(KXMLQLCVCXYPadPresetInputChannel))
            {
                quint32 uni = tag.attribute(KXMLQLCVCXYPadPresetInputUniverse).toUInt();
                quint32 ch = tag.attribute(KXMLQLCVCXYPadPresetInputChannel).toUInt();
                m_inputSource = QSharedPointer<QLCInputSource>(new QLCInputSource(uni, ch));
            }
        }
        else if (tag.tagName() == KXMLQLCVCXYPadPresetKey)
        {
            m_keySequence = VCWidget::stripKeySequence(QKeySequence(tag.text()));
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown VCMatrixControl tag:" << tag.tagName();
        }

        node = node.nextSibling();
    }
    if (hasPosition)
        m_dmxPos = pos;

    return true;
}

bool VCXYPadPreset::saveXML(QDomDocument *doc, QDomElement *xypad_root)
{
    QDomElement root;
    QDomElement tag;
    QDomText text;

    Q_ASSERT(doc != NULL);
    Q_ASSERT(xypad_root != NULL);

    root = doc->createElement(KXMLQLCVCXYPadPreset);
    root.setAttribute(KXMLQLCVCXYPadPresetID, m_id);
    xypad_root->appendChild(root);

    tag = doc->createElement(KXMLQLCVCXYPadPresetType);
    root.appendChild(tag);
    text = doc->createTextNode(typeToString(m_type));
    tag.appendChild(text);

    if (m_type == EFX)
    {
        tag = doc->createElement(KXMLQLCVCXYPadPresetEFXID);
        root.appendChild(tag);
        text = doc->createTextNode(QString::number(m_efxID));
        tag.appendChild(text);
    }
    else if (m_type == Position)
    {
        tag = doc->createElement(KXMLQLCVCXYPadPresetXPos);
        root.appendChild(tag);
        text = doc->createTextNode(QString::number(m_dmxPos.x()));
        tag.appendChild(text);
        tag = doc->createElement(KXMLQLCVCXYPadPresetYPos);
        root.appendChild(tag);
        text = doc->createTextNode(QString::number(m_dmxPos.y()));
        tag.appendChild(text);
    }

    /* External input source */
    if (!m_inputSource.isNull() && m_inputSource->isValid())
    {
        tag = doc->createElement(KXMLQLCVCXYPadPresetInput);
        tag.setAttribute(KXMLQLCVCXYPadPresetInputUniverse, QString("%1").arg(m_inputSource->universe()));
        tag.setAttribute(KXMLQLCVCXYPadPresetInputChannel, QString("%1").arg(m_inputSource->channel()));
        root.appendChild(tag);
    }

    /* Key sequence */
    if (m_keySequence.isEmpty() == false)
    {
        tag = doc->createElement(KXMLQLCVCXYPadPresetKey);
        root.appendChild(tag);
        text = doc->createTextNode(m_keySequence.toString());
        tag.appendChild(text);
    }

    return true;
}


