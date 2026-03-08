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
#include <climits>

#include "vcxypadpreset.h"
#include "function.h"
#include "vcwidget.h"

VCXYPadPreset::VCXYPadPreset(quint8 id)
    : m_id(id)
    , m_type(Position)
    , m_dmxPos(QPointF(0, 0))
    , m_funcID(Function::invalidId())
{
}

VCXYPadPreset::VCXYPadPreset(const VCXYPadPreset &preset)
{
    *this = preset;
}

VCXYPadPreset::~VCXYPadPreset()
{
}

VCXYPadPreset &VCXYPadPreset::operator=(const VCXYPadPreset &preset)
{
    if (this != &preset)
    {
        m_id = preset.m_id;
        m_type = preset.m_type;
        m_name = preset.m_name;
        m_dmxPos = preset.m_dmxPos;
        m_funcID = preset.m_funcID;
        m_fxGroup = preset.m_fxGroup;
        m_keySequence = preset.m_keySequence;

        if (preset.m_inputSource != nullptr)
        {
            m_inputSource = QSharedPointer<QLCInputSource>(new QLCInputSource(preset.m_inputSource->universe(),
                                                       preset.m_inputSource->channel()));

            m_inputSource->setFeedbackValue(QLCInputFeedback::LowerValue,
                                            preset.m_inputSource->feedbackValue(QLCInputFeedback::LowerValue));
            m_inputSource->setFeedbackValue(QLCInputFeedback::UpperValue,
                                            preset.m_inputSource->feedbackValue(QLCInputFeedback::UpperValue));
            m_inputSource->setFeedbackValue(QLCInputFeedback::MonitorValue,
                                            preset.m_inputSource->feedbackValue(QLCInputFeedback::MonitorValue));
        }
        else
        {
            m_inputSource.clear();
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

QString VCXYPadPreset::typeToString(PresetType type)
{
    if (type == EFX)
        return QStringLiteral("EFX");
    if (type == Scene)
        return QStringLiteral("Scene");
    if (type == FixtureGroup)
        return QStringLiteral("FixtureGroup");

    return QStringLiteral("Position");
}

VCXYPadPreset::PresetType VCXYPadPreset::stringToType(const QString &str)
{
    if (str == QStringLiteral("EFX"))
        return EFX;
    if (str == QStringLiteral("Scene"))
        return Scene;
    if (str == QStringLiteral("FixtureGroup"))
        return FixtureGroup;

    return Position;
}

QString VCXYPadPreset::color() const
{
    switch (m_type)
    {
        case EFX: return QStringLiteral("#BBBB8D");
        case Scene: return QStringLiteral("#BB8E8E");
        case FixtureGroup: return QStringLiteral("#95BB95");
        case Position:
        default:
            return QStringLiteral("#BBBBBB");
    }
}

static QSharedPointer<QLCInputSource> presetInputFromXML(QXmlStreamReader &root)
{
    if (root.name() != KXMLQLCVCWidgetInput)
        return QSharedPointer<QLCInputSource>();

    QXmlStreamAttributes attrs = root.attributes();
    if (attrs.hasAttribute(KXMLQLCVCWidgetInputUniverse) == false ||
        attrs.hasAttribute(KXMLQLCVCWidgetInputChannel) == false)
    {
        return QSharedPointer<QLCInputSource>();
    }

    quint32 uni = attrs.value(KXMLQLCVCWidgetInputUniverse).toString().toUInt();
    quint32 ch = attrs.value(KXMLQLCVCWidgetInputChannel).toString().toUInt();
    QSharedPointer<QLCInputSource> inputSource(new QLCInputSource(uni, ch));

    if (attrs.hasAttribute(KXMLQLCVCWidgetInputLowerValue))
        inputSource->setFeedbackValue(QLCInputFeedback::LowerValue,
                                      attrs.value(KXMLQLCVCWidgetInputLowerValue).toString().toUInt());
    if (attrs.hasAttribute(KXMLQLCVCWidgetInputUpperValue))
        inputSource->setFeedbackValue(QLCInputFeedback::UpperValue,
                                      attrs.value(KXMLQLCVCWidgetInputUpperValue).toString().toUInt());
    if (attrs.hasAttribute(KXMLQLCVCWidgetInputMonitorValue))
        inputSource->setFeedbackValue(QLCInputFeedback::MonitorValue,
                                      attrs.value(KXMLQLCVCWidgetInputMonitorValue).toString().toUInt());

    return inputSource;
}

bool VCXYPadPreset::loadXML(QXmlStreamReader &root)
{
    if (root.name() != KXMLQLCVCXYPadPreset)
    {
        qWarning() << Q_FUNC_INFO << "XY Pad Preset node not found";
        return false;
    }

    if (root.attributes().hasAttribute(KXMLQLCVCXYPadPresetID) == false)
    {
        qWarning() << Q_FUNC_INFO << "XY Pad Preset ID not found";
        return false;
    }

    m_id = root.attributes().value(KXMLQLCVCXYPadPresetID).toString().toUInt();

    QPointF pos;
    bool hasPosition = false;

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
            m_funcID = root.readElementText().toUInt();
        }
        else if (root.name() == KXMLQLCVCXYPadPresetXPos)
        {
            pos.setX(root.readElementText().toFloat());
            hasPosition = true;
        }
        else if (root.name() == KXMLQLCVCXYPadPresetYPos)
        {
            pos.setY(root.readElementText().toFloat());
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
            m_inputSource = presetInputFromXML(root);
            root.skipCurrentElement();
        }
        else if (root.name() == KXMLQLCVCWidgetKey)
        {
            m_keySequence = QKeySequence(root.readElementText());
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown VCXYPadPreset tag:" << root.name().toString();
            root.skipCurrentElement();
        }
    }

    if (hasPosition)
        m_dmxPos = pos;

    return true;
}

static void savePresetInput(QXmlStreamWriter *doc, const QSharedPointer<QLCInputSource> &inputSource)
{
    if (doc == nullptr || inputSource.isNull() || inputSource->isValid() == false)
        return;

    doc->writeStartElement(KXMLQLCVCWidgetInput);
    doc->writeAttribute(KXMLQLCVCWidgetInputUniverse, QString::number(inputSource->universe()));
    doc->writeAttribute(KXMLQLCVCWidgetInputChannel, QString::number(inputSource->channel()));

    if (inputSource->feedbackValue(QLCInputFeedback::LowerValue) != 0)
        doc->writeAttribute(KXMLQLCVCWidgetInputLowerValue,
                            QString::number(inputSource->feedbackValue(QLCInputFeedback::LowerValue)));
    if (inputSource->feedbackValue(QLCInputFeedback::UpperValue) != UCHAR_MAX)
        doc->writeAttribute(KXMLQLCVCWidgetInputUpperValue,
                            QString::number(inputSource->feedbackValue(QLCInputFeedback::UpperValue)));
    if (inputSource->feedbackValue(QLCInputFeedback::MonitorValue) != UCHAR_MAX)
        doc->writeAttribute(KXMLQLCVCWidgetInputMonitorValue,
                            QString::number(inputSource->feedbackValue(QLCInputFeedback::MonitorValue)));

    doc->writeEndElement();
}

bool VCXYPadPreset::saveXML(QXmlStreamWriter *doc) const
{
    Q_ASSERT(doc != nullptr);

    doc->writeStartElement(KXMLQLCVCXYPadPreset);
    doc->writeAttribute(KXMLQLCVCXYPadPresetID, QString::number(m_id));

    doc->writeTextElement(KXMLQLCVCXYPadPresetType, typeToString(m_type));
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
        for (const GroupHead &gh : m_fxGroup)
        {
            doc->writeStartElement(KXMLQLCVCXYPadPresetFixture);
            doc->writeAttribute(KXMLQLCVCXYPadPresetFixtureID, QString::number(gh.fxi));
            doc->writeAttribute(KXMLQLCVCXYPadPresetFixtureHead, QString::number(gh.head));
            doc->writeEndElement();
        }
    }

    savePresetInput(doc, m_inputSource);

    if (m_keySequence.isEmpty() == false)
        doc->writeTextElement(KXMLQLCVCWidgetKey, m_keySequence.toString());

    doc->writeEndElement();

    return true;
}
