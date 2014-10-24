/*
  Q Light Controller Plus
  vclabel.cpp

  Copyright (c) Heikki Junnila, Stefan Krumm

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

#include <QInputDialog>
#include <QPaintEvent>
#include <QLineEdit>
#include <QPainter>
#include <QString>
#include <QDebug>
#include <QStyle>
#include <QSize>
#include <QtXml>

#include "qlcfile.h"

#include "virtualconsole.h"
#include "mastertimer.h"
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

bool VCLabel::loadXML(const QDomElement* root)
{
    Q_ASSERT(root != NULL);

    if (root->tagName() != KXMLQLCVCLabel)
    {
        qWarning() << Q_FUNC_INFO << "Label node not found";
        return false;
    }

    /* Widget commons */
    loadXMLCommon(root);

    /* Children */
    QDomNode node = root->firstChild();
    while (node.isNull() == false)
    {
        QDomElement tag = node.toElement();
        if (tag.tagName() == KXMLQLCWindowState)
        {
            int x = 0, y = 0, w = 0, h = 0;
            bool visible = false;
            loadXMLWindowState(&tag, &x, &y, &w, &h, &visible);
            setGeometry(x, y, w, h);
        }
        else if (tag.tagName() == KXMLQLCVCWidgetAppearance)
        {
            loadXMLAppearance(&tag);
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown label tag:" << tag.tagName();
        }

        node = node.nextSibling();
    }

    return true;
}

bool VCLabel::saveXML(QDomDocument* doc, QDomElement* vc_root)
{
    QDomElement root;

    Q_ASSERT(doc != NULL);
    Q_ASSERT(vc_root != NULL);

    /* VC Label entry */
    root = doc->createElement(KXMLQLCVCLabel);
    vc_root->appendChild(root);

    saveXMLCommon(doc, &root);

    /* Window state */
    saveXMLWindowState(doc, &root);

    /* Appearance */
    saveXMLAppearance(doc, &root);

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
