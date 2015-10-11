/*
  Q Light Controller Plus
  vcspeeddialpreset.cpp

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

#include "vcspeeddialpreset.h"
#include "vcwidget.h"
#include "qlcfile.h"

VCSpeedDialPreset::VCSpeedDialPreset(quint8 id)
    : m_id(id)
    , m_value(1000)
{
}

VCSpeedDialPreset::VCSpeedDialPreset(VCSpeedDialPreset const& preset)
    : m_id(preset.m_id)
    , m_name(preset.m_name)
    , m_value(preset.m_value)
    , m_keySequence(preset.m_keySequence)
{
    if (preset.m_inputSource != NULL)
    {
        m_inputSource = QSharedPointer<QLCInputSource>(new QLCInputSource(preset.m_inputSource->universe(),
                                                       preset.m_inputSource->channel()));
        m_inputSource->setRange(preset.m_inputSource->lowerValue(), preset.m_inputSource->upperValue());
    }
}

VCSpeedDialPreset::~VCSpeedDialPreset()
{
}

bool VCSpeedDialPreset::operator<(VCSpeedDialPreset const& right) const
{
    return m_id < right.m_id;
}

bool VCSpeedDialPreset::compare(VCSpeedDialPreset const* left, VCSpeedDialPreset const* right)
{
    return *left < *right;
}

bool VCSpeedDialPreset::loadXML(const QDomElement &root)
{
    QDomNode node;
    QDomElement tag;

    if (root.tagName() != KXMLQLCVCSpeedDialPreset)
    {
        qWarning() << Q_FUNC_INFO << "Speed Dial Preset node not found";
        return false;
    }

    if (root.hasAttribute(KXMLQLCVCSpeedDialPresetID) == false)
    {
        qWarning() << Q_FUNC_INFO << "Speed Dial Preset ID not found";
        return false;
    }

    m_id = root.attribute(KXMLQLCVCSpeedDialPresetID).toUInt();

    /* Children */
    node = root.firstChild();
    while (node.isNull() == false)
    {
        tag = node.toElement();
        if (tag.tagName() == KXMLQLCVCSpeedDialPresetName)
        {
            m_name = tag.text();
        }
        else if (tag.tagName() == KXMLQLCVCSpeedDialPresetValue)
        {
            m_value = tag.text().toInt();
        }
        else if (tag.tagName() == KXMLQLCVCSpeedDialPresetInput)
        {
            if (tag.hasAttribute(KXMLQLCVCSpeedDialPresetInputUniverse) &&
                tag.hasAttribute(KXMLQLCVCSpeedDialPresetInputChannel))
            {
                quint32 uni = tag.attribute(KXMLQLCVCSpeedDialPresetInputUniverse).toUInt();
                quint32 ch = tag.attribute(KXMLQLCVCSpeedDialPresetInputChannel).toUInt();
                m_inputSource = QSharedPointer<QLCInputSource>(new QLCInputSource(uni, ch));

                uchar min = 0, max = UCHAR_MAX;
                if (tag.hasAttribute(KXMLQLCVCWidgetInputLowerValue))
                    min = uchar(tag.attribute(KXMLQLCVCWidgetInputLowerValue).toUInt());
                if (tag.hasAttribute(KXMLQLCVCWidgetInputUpperValue))
                    max = uchar(tag.attribute(KXMLQLCVCWidgetInputUpperValue).toUInt());
                m_inputSource->setRange(min, max);
            }
        }
        else if (tag.tagName() == KXMLQLCVCSpeedDialPresetKey)
        {
            m_keySequence = VCWidget::stripKeySequence(QKeySequence(tag.text()));
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown VCSpeedDialPreset tag:" << tag.tagName();
        }

        node = node.nextSibling();
    }

    return true;
}

bool VCSpeedDialPreset::saveXML(QDomDocument *doc, QDomElement *mtx_root)
{
    QDomElement root;
    QDomElement tag;
    QDomText text;

    Q_ASSERT(doc != NULL);
    Q_ASSERT(mtx_root != NULL);

    root = doc->createElement(KXMLQLCVCSpeedDialPreset);
    root.setAttribute(KXMLQLCVCSpeedDialPresetID, m_id);
    mtx_root->appendChild(root);

    tag = doc->createElement(KXMLQLCVCSpeedDialPresetName);
    root.appendChild(tag);
    text = doc->createTextNode(m_name);
    tag.appendChild(text);

    tag = doc->createElement(KXMLQLCVCSpeedDialPresetValue);
    root.appendChild(tag);
    text = doc->createTextNode(QString::number(m_value));
    tag.appendChild(text);

    /* External input source */
    if (!m_inputSource.isNull() && m_inputSource->isValid())
    {
        tag = doc->createElement(KXMLQLCVCSpeedDialPresetInput);
        tag.setAttribute(KXMLQLCVCSpeedDialPresetInputUniverse, QString("%1").arg(m_inputSource->universe()));
        tag.setAttribute(KXMLQLCVCSpeedDialPresetInputChannel, QString("%1").arg(m_inputSource->channel()));
        if (m_inputSource->lowerValue() != 0)
            tag.setAttribute(KXMLQLCVCWidgetInputLowerValue, QString::number(m_inputSource->lowerValue()));
        if (m_inputSource->upperValue() != UCHAR_MAX)
            tag.setAttribute(KXMLQLCVCWidgetInputUpperValue, QString::number(m_inputSource->upperValue()));
        root.appendChild(tag);
    }

    /* Key sequence */
    if (m_keySequence.isEmpty() == false)
    {
        tag = doc->createElement(KXMLQLCVCSpeedDialPresetKey);
        root.appendChild(tag);
        text = doc->createTextNode(m_keySequence.toString());
        tag.appendChild(text);
    }

    return true;
}


