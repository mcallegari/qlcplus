/*
  Q Light Controller
  vcclock.cpp

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
#include <QtGui>
#include <QStyle>

#include "qlcfile.h"

#include "virtualconsole.h"
#include "inputmap.h"
#include "vcclock.h"
#include "doc.h"

VCClock::VCClock(QWidget* parent, Doc* doc) : VCWidget(parent, doc)
{
    /* Set the class name "VCClock" as the object name as well */
    setObjectName(VCClock::staticMetaObject.className());

    setType(VCWidget::LabelWidget);
    setCaption("");
    resize(QSize(150, 50));
    QFont font = qApp->font();
    font.setBold(true);
    font.setPixelSize(28);
    setFont(font);

    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(update()));
    timer->start(1000);
}

VCClock::~VCClock()
{
}

/*****************************************************************************
 * Clipboard
 *****************************************************************************/

VCWidget* VCClock::createCopy(VCWidget* parent)
{
    Q_ASSERT(parent != NULL);

    VCClock* clock = new VCClock(parent, m_doc);
    if (clock->copyFrom(this) == false)
    {
        delete clock;
        clock = NULL;
    }

    return clock;
}

/*****************************************************************************
 * Properties
 *****************************************************************************/

void VCClock::editProperties()
{
    /*
    bool ok = false;
    QString text = QInputDialog::getText(NULL, tr("Rename Label"), tr("Caption:"),
                                         QLineEdit::Normal, caption(), &ok);
    if (ok == true)
        setCaption(text);
    */
}

/*****************************************************************************
 * Load & Save
 *****************************************************************************/

bool VCClock::loadXML(const QDomElement* root)
{
    Q_ASSERT(root != NULL);

    if (root->tagName() != KXMLQLCVCClock)
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

bool VCClock::saveXML(QDomDocument* doc, QDomElement* vc_root)
{
    QDomElement root;
    QDomElement tag;
    QDomText text;
    QString str;

    Q_ASSERT(doc != NULL);
    Q_ASSERT(vc_root != NULL);

    /* VC Label entry */
    root = doc->createElement(KXMLQLCVCClock);
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

void VCClock::paintEvent(QPaintEvent* e)
{
    QPainter painter(this);
    QDateTime currTime = QDateTime::currentDateTime();

    style()->drawItemText(&painter, rect(), Qt::AlignCenter | Qt::TextWordWrap, palette(),
                          true, currTime.time().toString(), foregroundRole());
    painter.end();

    VCWidget::paintEvent(e);
}
