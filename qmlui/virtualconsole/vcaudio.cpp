/*
  Q Light Controller Plus
  vcaudio.cpp

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
#include <QQmlEngine>

#include "vcaudio.h"
#include "doc.h"

VCAudio::VCAudio(Doc *doc, QObject *parent)
    : VCWidget(doc, parent)
{
    setType(VCWidget::AudioTriggersWidget);
}

VCAudio::~VCAudio()
{
    if (m_item)
        delete m_item;
}

QString VCAudio::defaultCaption()
{
    return tr("Audio Trigger %1").arg(id() + 1);
}

void VCAudio::setupLookAndFeel(qreal pixelDensity, int page)
{
    setPage(page);
    QFont wFont = font();
    wFont.setBold(true);
    wFont.setPointSize(pixelDensity * 5.0);
    setFont(wFont);
}

void VCAudio::render(QQuickView *view, QQuickItem *parent)
{
    if (view == nullptr || parent == nullptr)
        return;

    QQmlComponent *component = new QQmlComponent(view->engine(), QUrl("qrc:/VCAudioItem.qml"));

    if (component->isError())
    {
        qDebug() << component->errors();
        return;
    }

    m_item = qobject_cast<QQuickItem*>(component->create());

    m_item->setParentItem(parent);
    m_item->setProperty("audioObj", QVariant::fromValue(this));
}

QString VCAudio::propertiesResource() const
{
    return QString("qrc:/VCAudioProperties.qml");
}

VCWidget *VCAudio::createCopy(VCWidget *parent)
{
    Q_ASSERT(parent != nullptr);

    VCAudio *audio = new VCAudio(m_doc, parent);
    if (audio->copyFrom(this) == false)
    {
        delete audio;
        audio = nullptr;
    }

    return audio;
}

bool VCAudio::copyFrom(const VCWidget *widget)
{
    const VCAudio *audio = qobject_cast<const VCAudio*> (widget);
    if (audio == nullptr)
        return false;

    /* Copy and set properties */

    /* Copy object lists */

    /* Common stuff */
    return VCWidget::copyFrom(widget);
}

FunctionParent VCAudio::functionParent() const
{
    return FunctionParent(FunctionParent::AutoVCWidget, id());
}

/*********************************************************************
 * Load & Save
 *********************************************************************/

bool VCAudio::loadXML(QXmlStreamReader &root)
{
    if (root.name() != KXMLQLCVCAudio)
    {
        qWarning() << Q_FUNC_INFO << "Audio node not found";
        return false;
    }

    QXmlStreamAttributes attrs = root.attributes();

    /* Widget commons */
    loadXMLCommon(root);

    while (root.readNextStartElement())
    {
        if (root.name() == KXMLQLCWindowState)
        {
            bool visible = false;
            int x = 0, y = 0, w = 0, h = 0;
            loadXMLWindowState(root, &x, &y, &w, &h, &visible);
            setGeometry(QRect(x, y, w, h));
        }
        else if (root.name() == KXMLQLCVCWidgetAppearance)
        {
            loadXMLAppearance(root);
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown audio tag:" << root.name().toString();
            root.skipCurrentElement();
        }
    }

    return true;
}

bool VCAudio::saveXML(QXmlStreamWriter *doc)
{
    Q_ASSERT(doc != nullptr);

    /* VC object entry */
    doc->writeStartElement(KXMLQLCVCAudio);

    saveXMLCommon(doc);

    /* Window state */
    saveXMLWindowState(doc);

    /* Appearance */
    saveXMLAppearance(doc);

    /* Write the <end> tag */
    doc->writeEndElement();

    return true;
}
