/*
  Q Light Controller Plus
  vclabel.cpp

  Copyright (c) Heikki Junnila
                Stefan Krumm
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
#include <QInputDialog>
#include <QPaintEvent>
#include <QLineEdit>
#include <QPainter>
#include <QString>
#include <QDebug>
#include <QStyle>
#include <QSize>

#include "vclabel.h"
#include "doc.h"

VCLabel::VCLabel(QWidget* parent, Doc* doc) : VCWidget(parent, doc)
{
    /* Set the class name "VCLabel" as the object name as well */
    setObjectName(VCLabel::staticMetaObject.className());

    setType(VCWidget::LabelWidget);
    setCaption(tr("Label"));
    resize(QSize(100, 30));
}

VCLabel::~VCLabel()
{
}

/*****************************************************************************
 * Clipboard
 *****************************************************************************/

VCWidget* VCLabel::createCopy(VCWidget* parent)
{
    Q_ASSERT(parent != NULL);

    VCLabel* label = new VCLabel(parent, m_doc);
    if (label->copyFrom(this) == false)
    {
        delete label;
        label = NULL;
    }

    return label;
}

/*****************************************************************************
 * Properties
 *****************************************************************************/

void VCLabel::editProperties()
{
    bool ok = false;
    QString text = QInputDialog::getText(NULL, tr("Rename Label"), tr("Caption:"),
                                         QLineEdit::Normal, caption(), &ok);
    if (ok == true)
        setCaption(text);
}

/*****************************************************************************
 * Load & Save
 *****************************************************************************/

bool VCLabel::loadXML(QXmlStreamReader &root)
{
    if (root.name() != KXMLQLCVCLabel)
    {
        qWarning() << Q_FUNC_INFO << "Label node not found";
        return false;
    }

    /* Widget commons */
    loadXMLCommon(root);

    /* Children */
    while (root.readNextStartElement())
    {
        if (root.name() == KXMLQLCWindowState)
        {
            int x = 0, y = 0, w = 0, h = 0;
            bool visible = false;
            loadXMLWindowState(root, &x, &y, &w, &h, &visible);
            setGeometry(x, y, w, h);
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
    Q_ASSERT(doc != NULL);

    /* VC Label entry */
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

/****************************************************************************
 * Drawing
 ****************************************************************************/

void VCLabel::paintEvent(QPaintEvent* e)
{
    bool enabled = false;
    if (mode() == Doc::Operate && isDisabled() == false)
        enabled = true;

    QPainter painter(this);
    style()->drawItemText(&painter, rect(), Qt::AlignCenter | Qt::TextWordWrap, palette(),
                          enabled, caption(), foregroundRole());
    painter.end();

    VCWidget::paintEvent(e);
}
