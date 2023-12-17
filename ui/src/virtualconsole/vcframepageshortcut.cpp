/*
  Q Light Controller Plus
  vcframepageshortcut.cpp

  Copyright (c) Lukas Jähn
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

VCFramePageShortcut::VCFramePageShortcut(int pageIndex, quint8 inputID)
    : m_id(inputID)
    , m_page(pageIndex)
    , m_inputSource(QSharedPointer<QLCInputSource>())
{
    setName();
}

VCFramePageShortcut::~VCFramePageShortcut()
{
}

QString VCFramePageShortcut::name() const
{
    return m_name;
}

void VCFramePageShortcut::setName(QString name)
{
    if (name.isEmpty())
        m_name = QObject::tr("Page: %1").arg(m_page + 1);
    else
        m_name = name;
}

bool VCFramePageShortcut::loadXML(QXmlStreamReader &root)
{
    if (root.name() != KXMLQLCVCFramePageShortcut)
    {
        qWarning() << Q_FUNC_INFO << "Frame page shortcut node not found";
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
    setName(root.attributes().value(KXMLQLCVCFramePageShortcutName).toString());

    /* Children */
    while (root.readNextStartElement())
    {
        if (root.name() == KXMLQLCVCWidgetInput)
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
    doc->writeAttribute(KXMLQLCVCFramePageShortcutName, m_name);

    /* External input source */
    if (!m_inputSource.isNull() && m_inputSource->isValid())
        VCWidget::saveXMLInput(doc, m_inputSource);

    /* Key sequence */
    if (m_keySequence.toString().isEmpty() == false)
        doc->writeTextElement(KXMLQLCVCWidgetKey, m_keySequence.toString());

    /* End the <Preset> tag */
    doc->writeEndElement();

    return true;
}
