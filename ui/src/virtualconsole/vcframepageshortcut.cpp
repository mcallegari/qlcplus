/*
  Q Light Controller Plus
  vcframepageshortcut.cpp

  Copyright (c) Heikki Junnila
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

#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QDebug>

#include "vcframepageshortcut.h"
#include "vcwidget.h"
#include "qlcfile.h"

VCFramePageShortcut::VCFramePageShortcut(int pageIndex, quint8 inputID)
    : m_id(inputID)
    , m_page(pageIndex)
    , m_name(QObject::tr("Page: %1").arg(pageIndex + 1))
{
}

VCFramePageShortcut::~VCFramePageShortcut()
{
}

bool VCFramePageShortcut::operator<(VCFramePageShortcut const& right) const
{
    return m_id < right.m_id;
}

bool VCFramePageShortcut::compare(VCFramePageShortcut const* left, VCFramePageShortcut const* right)
{
    return *left < *right;
}

bool VCFramePageShortcut::loadXML(QXmlStreamReader &root)
{
    if (root.name() != KXMLQLCVCFramePageShortcut)
    {
        qWarning() << Q_FUNC_INFO << "Frame page shortcut node not found";
        return false;
    }

    if (root.attributes().hasAttribute(KXMLQLCVCFramePageShortcutID) == false)
    {
        qWarning() << Q_FUNC_INFO << "Frame page shortcut ID not found";
        return false;
    }

    if (root.attributes().hasAttribute(KXMLQLCVCFramePageShortcutPage) == false)
    {
        qWarning() << Q_FUNC_INFO << "Frame page shortcut page not found";
        return false;
    }

    if (root.attributes().hasAttribute(KXMLQLCVCFramePageShortcutName) == false)
    {
        qWarning() << Q_FUNC_INFO << "Frame page shortcut name not found";
        return false;
    }

    m_page = root.attributes().value(KXMLQLCVCFramePageShortcutPage).toString().toInt();
    m_id = root.attributes().value(KXMLQLCVCFramePageShortcutID).toString().toUInt();
    m_name = root.attributes().value(KXMLQLCVCFramePageShortcutName).toString();

    /* Children */
    while (root.readNextStartElement())
    {
        if (root.name() == KXMLQLCVCFramePageShortcutInput)
        {
            QXmlStreamAttributes attrs = root.attributes();

            if (attrs.hasAttribute(KXMLQLCVCFramePageShortcutInputUniverse) &&
                attrs.hasAttribute(KXMLQLCVCFramePageShortcutInputChannel))
            {
                quint32 uni = attrs.value(KXMLQLCVCFramePageShortcutInputUniverse).toString().toUInt();
                quint32 ch = attrs.value(KXMLQLCVCFramePageShortcutInputChannel).toString().toUInt();
                m_inputSource = QSharedPointer<QLCInputSource>(new QLCInputSource(uni, ch));

                uchar min = 0, max = UCHAR_MAX;
                if (attrs.hasAttribute(KXMLQLCVCWidgetInputLowerValue))
                    min = uchar(attrs.value(KXMLQLCVCWidgetInputLowerValue).toString().toUInt());
                if (attrs.hasAttribute(KXMLQLCVCWidgetInputUpperValue))
                    max = uchar(attrs.value(KXMLQLCVCWidgetInputUpperValue).toString().toUInt());
                m_inputSource->setRange(min, max);
            }
            root.skipCurrentElement();
        }
        else if (root.name() == KXMLQLCVCFramePageShortcutKey)
        {
            m_keySequence = VCWidget::stripKeySequence(QKeySequence(root.readElementText()));
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown VCFramePageShortcut tag:" << root.name().toString();
            root.skipCurrentElement();
        }
    }

    return true;
}

bool VCFramePageShortcut::saveXML(QXmlStreamWriter *doc)
{
    Q_ASSERT(doc != NULL);

    doc->writeStartElement(KXMLQLCVCFramePageShortcut);
    doc->writeAttribute(KXMLQLCVCFramePageShortcutPage, QString::number(m_page));
    doc->writeAttribute(KXMLQLCVCFramePageShortcutID, QString::number(m_id));
    doc->writeAttribute(KXMLQLCVCFramePageShortcutName, m_name);

    /* External input source */
    if (!m_inputSource.isNull() && m_inputSource->isValid())
    {
        doc->writeStartElement(KXMLQLCVCFramePageShortcutInput);
        doc->writeAttribute(KXMLQLCVCFramePageShortcutInputUniverse, QString("%1").arg(m_inputSource->universe()));
        doc->writeAttribute(KXMLQLCVCFramePageShortcutInputChannel, QString("%1").arg(m_inputSource->channel()));
        if (m_inputSource->lowerValue() != 0)
            doc->writeAttribute(KXMLQLCVCWidgetInputLowerValue, QString::number(m_inputSource->lowerValue()));
        if (m_inputSource->upperValue() != UCHAR_MAX)
            doc->writeAttribute(KXMLQLCVCWidgetInputUpperValue, QString::number(m_inputSource->upperValue()));
        doc->writeEndElement();
    }

    /* Key sequence */
    if (m_keySequence.isEmpty() == false)
        doc->writeTextElement(KXMLQLCVCFramePageShortcutKey, m_keySequence.toString());

    /* End the <Preset> tag */
    doc->writeEndElement();

    return true;
}
