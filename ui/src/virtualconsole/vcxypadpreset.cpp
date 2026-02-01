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

#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QDebug>

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

VCXYPadPreset::VCXYPadPreset(const VCXYPadPreset &other)
{
    *this = other;
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

VCXYPadPreset &VCXYPadPreset::operator=(const VCXYPadPreset &vcpp)
{
    if (this != &vcpp)
    {
        m_id = vcpp.m_id;
        m_type = vcpp.m_type;
        m_name = vcpp.m_name;
        m_dmxPos = vcpp.m_dmxPos;
        m_funcID = vcpp.m_funcID;
        m_fxGroup = vcpp.m_fxGroup;
        m_keySequence = vcpp.m_keySequence;

        if (vcpp.m_inputSource != NULL)
        {
            m_inputSource = QSharedPointer<QLCInputSource>(new QLCInputSource(vcpp.m_inputSource->universe(),
                                                   vcpp.m_inputSource->channel()));

            m_inputSource->setFeedbackValue(QLCInputFeedback::LowerValue, vcpp.m_inputSource->feedbackValue(QLCInputFeedback::LowerValue));
            m_inputSource->setFeedbackValue(QLCInputFeedback::UpperValue, vcpp.m_inputSource->feedbackValue(QLCInputFeedback::UpperValue));
        }
    }
    return *this;
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

bool VCXYPadPreset::loadXML(QXmlStreamReader &root)
{
    if (root.name() != KXMLQLCVCXYPadPreset)
    {
        qWarning() << Q_FUNC_INFO << "Matrix control node not found";
        return false;
    }

    if (root.attributes().hasAttribute(KXMLQLCVCXYPadPresetID) == false)
    {
        qWarning() << Q_FUNC_INFO << "XYPad Preset ID not found";
        return false;
    }

    m_id = root.attributes().value(KXMLQLCVCXYPadPresetID).toString().toUInt();

    QPointF pos;
    bool hasPosition = false;

    /* Children */
    while (root.readNextStartElement())
    {
        if (root.name() == KXMLQLCVCXYPadPresetType)
        {
            m_type = stringToType(root.readElementText());
        }
        else if (root.name() == KXMLQLCVCXYPadPresetName)
        {
            m_name = root.readElementText();
        }
        else if (root.name() == KXMLQLCVCXYPadPresetFuncID)
        {
            setFunctionID(root.readElementText().toUInt());
        }
        else if (root.name() == KXMLQLCVCXYPadPresetXPos)
        {
            pos.setX(QString(root.readElementText()).toFloat());
            hasPosition = true;
        }
        else if (root.name() == KXMLQLCVCXYPadPresetYPos)
        {
            pos.setY(QString(root.readElementText()).toFloat());
            hasPosition = true;
        }
        else if (root.name() == KXMLQLCVCXYPadPresetFixture)
        {
            quint32 fxID = Fixture::invalidId();
            int head = -1;
            QXmlStreamAttributes attrs = root.attributes();

            if (attrs.hasAttribute(KXMLQLCVCXYPadPresetFixtureID))
                fxID = attrs.value(KXMLQLCVCXYPadPresetFixtureID).toString().toUInt();
            if (attrs.hasAttribute(KXMLQLCVCXYPadPresetFixtureHead))
                head = attrs.value(KXMLQLCVCXYPadPresetFixtureHead).toString().toInt();

            if (fxID != Fixture::invalidId() && head != -1)
                m_fxGroup.append(GroupHead(fxID, head));
            root.skipCurrentElement();
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

    if (hasPosition)
        m_dmxPos = pos;

    return true;
}

bool VCXYPadPreset::saveXML(QXmlStreamWriter *doc)
{
    Q_ASSERT(doc != NULL);

    doc->writeStartElement(KXMLQLCVCXYPadPreset);
    doc->writeAttribute(KXMLQLCVCXYPadPresetID, QString::number(m_id));

    /* Preset type */
    doc->writeTextElement(KXMLQLCVCXYPadPresetType, typeToString(m_type));
    /* Preset name */
    doc->writeTextElement(KXMLQLCVCXYPadPresetName, m_name);

    if (m_type == EFX || m_type == Scene)
    {
        doc->writeTextElement(KXMLQLCVCXYPadPresetFuncID, QString::number(m_funcID));
    }
    else if (m_type == Position)
    {
        doc->writeTextElement(KXMLQLCVCXYPadPresetXPos, QString::number(m_dmxPos.x()));
        doc->writeTextElement(KXMLQLCVCXYPadPresetYPos, QString::number(m_dmxPos.y()));
    }
    else if (m_type == FixtureGroup)
    {
        foreach (GroupHead gh, fixtureGroup())
        {
            doc->writeStartElement(KXMLQLCVCXYPadPresetFixture);
            doc->writeAttribute(KXMLQLCVCXYPadPresetFixtureID, QString::number(gh.fxi));
            doc->writeAttribute(KXMLQLCVCXYPadPresetFixtureHead, QString::number(gh.head));
            doc->writeEndElement();
        }
    }

    /* External input source */
    if (!m_inputSource.isNull() && m_inputSource->isValid())
        VCWidget::saveXMLInput(doc, m_inputSource);

    /* Key sequence */
    if (m_keySequence.isEmpty() == false)
        doc->writeTextElement(KXMLQLCVCWidgetKey, m_keySequence.toString());

    /* End the <Preset> tag */
    doc->writeEndElement();

    return true;
}


