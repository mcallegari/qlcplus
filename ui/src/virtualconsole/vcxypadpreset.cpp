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
    , m_name(QString())
    , m_dmxPos(QPointF())
    , m_funcID(Function::invalidId())
{

}

VCXYPadPreset::VCXYPadPreset(const VCXYPadPreset &vcpp)
    : m_id(vcpp.m_id)
    , m_type(vcpp.m_type)
    , m_name(vcpp.m_name)
    , m_dmxPos(vcpp.m_dmxPos)
    , m_funcID(vcpp.m_funcID)
    , m_fxGroup(vcpp.m_fxGroup)
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

QString VCXYPadPreset::getColor() const
{
    switch(m_type)
    {
        case EFX: return ("#BBBB8D"); break;
        case Scene: return ("#BB8E8E"); break;
        case FixtureGroup: return ("#95BB95"); break;
        case Position:
        default:
            return ("#BBBBBB");
        break;
    }
}

void VCXYPadPreset::setFunctionID(quint32 id)
{
    m_funcID = id;
}

quint32 VCXYPadPreset::functionID() const
{
    return m_funcID;
}

void VCXYPadPreset::setPosition(QPointF pos)
{
    m_dmxPos = pos;
}

QPointF VCXYPadPreset::position() const
{
    return m_dmxPos;
}

void VCXYPadPreset::setFixtureGroup(QList<GroupHead> heads)
{
    m_fxGroup = heads;
}

QList<GroupHead> VCXYPadPreset::fixtureGroup() const
{
    return m_fxGroup;
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
    else if (type == Scene)
        return "Scene";
    else if (type == FixtureGroup)
        return "FixtureGroup";

    return "Position";
}

VCXYPadPreset::PresetType VCXYPadPreset::stringToType(QString str)
{
    if (str == "EFX")
        return EFX;
    else if (str == "Scene")
        return Scene;
    else if (str == "FixtureGroup")
        return FixtureGroup;

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
        else if (tag.tagName() == KXMLQLCVCXYPadPresetName)
        {
            m_name = tag.text();
        }
        else if (tag.tagName() == KXMLQLCVCXYPadPresetFuncID)
        {
            setFunctionID(tag.text().toUInt());
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
        else if (tag.tagName() == KXMLQLCVCXYPadPresetFixture)
        {
            quint32 fxID = Fixture::invalidId();
            int head = -1;

            if (tag.hasAttribute(KXMLQLCVCXYPadPresetFixtureID))
                fxID = tag.attribute(KXMLQLCVCXYPadPresetFixtureID).toUInt();
            if (tag.hasAttribute(KXMLQLCVCXYPadPresetFixtureHead))
                head = tag.attribute(KXMLQLCVCXYPadPresetFixtureHead).toInt();

            if (fxID != Fixture::invalidId() && head != -1)
                m_fxGroup.append(GroupHead(fxID, head));
        }
        else if (tag.tagName() == KXMLQLCVCXYPadPresetInput)
        {
            if (tag.hasAttribute(KXMLQLCVCXYPadPresetInputUniverse) &&
                tag.hasAttribute(KXMLQLCVCXYPadPresetInputChannel))
            {
                quint32 uni = tag.attribute(KXMLQLCVCXYPadPresetInputUniverse).toUInt();
                quint32 ch = tag.attribute(KXMLQLCVCXYPadPresetInputChannel).toUInt();
                m_inputSource = QSharedPointer<QLCInputSource>(new QLCInputSource(uni, ch));

                uchar min = 0, max = UCHAR_MAX;
                if (tag.hasAttribute(KXMLQLCVCWidgetInputLowerValue))
                    min = uchar(tag.attribute(KXMLQLCVCWidgetInputLowerValue).toUInt());
                if (tag.hasAttribute(KXMLQLCVCWidgetInputUpperValue))
                    max = uchar(tag.attribute(KXMLQLCVCWidgetInputUpperValue).toUInt());
                m_inputSource->setRange(min, max);
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

    tag = doc->createElement(KXMLQLCVCXYPadPresetName);
    root.appendChild(tag);
    text = doc->createTextNode(m_name);
    tag.appendChild(text);

    if (m_type == EFX || m_type == Scene)
    {
        tag = doc->createElement(KXMLQLCVCXYPadPresetFuncID);
        root.appendChild(tag);
        text = doc->createTextNode(QString::number(m_funcID));
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
    else if (m_type == FixtureGroup)
    {
        foreach (GroupHead gh, fixtureGroup())
        {
            tag = doc->createElement(KXMLQLCVCXYPadPresetFixture);
            tag.setAttribute(KXMLQLCVCXYPadPresetFixtureID, gh.fxi);
            tag.setAttribute(KXMLQLCVCXYPadPresetFixtureHead, gh.head);
            root.appendChild(tag);
        }
    }

    /* External input source */
    if (!m_inputSource.isNull() && m_inputSource->isValid())
    {
        tag = doc->createElement(KXMLQLCVCXYPadPresetInput);
        tag.setAttribute(KXMLQLCVCXYPadPresetInputUniverse, QString("%1").arg(m_inputSource->universe()));
        tag.setAttribute(KXMLQLCVCXYPadPresetInputChannel, QString("%1").arg(m_inputSource->channel()));
        if (m_inputSource->lowerValue() != 0)
            tag.setAttribute(KXMLQLCVCWidgetInputLowerValue, QString::number(m_inputSource->lowerValue()));
        if (m_inputSource->upperValue() != UCHAR_MAX)
            tag.setAttribute(KXMLQLCVCWidgetInputUpperValue, QString::number(m_inputSource->upperValue()));
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


