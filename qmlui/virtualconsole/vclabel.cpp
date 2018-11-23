/*
  Q Light Controller Plus
  vclabel.cpp

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

#include "vclabel.h"

VCLabel::VCLabel(Doc *doc, QObject *parent)
    : VCWidget(doc, parent)
{
    setType(VCWidget::LabelWidget);
}

VCLabel::~VCLabel()
{
    if (m_item)
        delete m_item;
}

QString VCLabel::defaultCaption()
{
    return tr("Label %1").arg(id() + 1);
}

void VCLabel::setupLookAndFeel(qreal pixelDensity, int page)
{
    setPage(page);
    setDefaultFontSize(pixelDensity * 3.5);
}

void VCLabel::render(QQuickView *view, QQuickItem *parent)
{
    if (view == nullptr || parent == nullptr)
        return;

    QQmlComponent *component = new QQmlComponent(view->engine(), QUrl("qrc:/VCLabelItem.qml"));

    if (component->isError())
    {
        qDebug() << component->errors();
        return;
    }

    m_item = qobject_cast<QQuickItem*>(component->create());

    m_item->setParentItem(parent);
    m_item->setProperty("labelObj", QVariant::fromValue(this));
}

VCWidget *VCLabel::createCopy(VCWidget *parent)
{
    Q_ASSERT(parent != nullptr);

    VCLabel *label = new VCLabel(m_doc, parent);
    if (label->copyFrom(this) == false)
    {
        delete label;
        label = nullptr;
    }

    return label;
}

/*********************************************************************
 * Load & Save
 *********************************************************************/

bool VCLabel::loadXML(QXmlStreamReader &root)
{
    if (root.name() != KXMLQLCVCLabel)
    {
        qWarning() << Q_FUNC_INFO << "Label node not found";
        return false;
    }

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
            qWarning() << Q_FUNC_INFO << "Unknown label tag:" << root.name().toString();
            root.skipCurrentElement();
        }
    }

    return true;
}

bool VCLabel::saveXML(QXmlStreamWriter *doc)
{
    Q_ASSERT(doc != nullptr);

    /* VC label entry */
    doc->writeStartElement(KXMLQLCVCLabel);

    saveXMLCommon(doc);

    /* Window state */
    saveXMLWindowState(doc);

    /* Appearance */
    saveXMLAppearance(doc);

    /* End the <Label> tag */
    doc->writeEndElement();

    return true;
}
