/*
  Q Light Controller Plus
  vcsoloframe.cpp

  Copyright (c) Anders Thomsen
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

#include <QMetaObject>
#include <QMessageBox>
#include <QSettings>
#include <QPainter>
#include <QAction>
#include <QStyle>
#include <QDebug>
#include <QPoint>
#include <QSize>
#include <QMenu>
#include <QList>

#include "vcpropertieseditor.h"
#include "virtualconsole.h"
#include "vcsoloframe.h"
#include "vcsoloframeproperties.h"
#include "vcbutton.h"
#include "doc.h"

VCSoloFrame::VCSoloFrame(QWidget* parent, Doc* doc, bool canCollapse)
    : VCFrame(parent, doc, canCollapse)
    , m_soloframeMixing(false)
{
    /* Set the class name "VCSoloFrame" as the object name as well */
    setObjectName(VCSoloFrame::staticMetaObject.className());
    setType(VCWidget::SoloFrameWidget);

    m_frameStyle = KVCFrameStyleSunken;

    if (canCollapse == true)
    {
        QString txtColor = "white";
        if (m_hasCustomForegroundColor)
            txtColor = this->foregroundColor().name();
        m_label->setStyleSheet("QLabel { background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #BC0A0A, stop: 1 #370303); "
                               "color: " + txtColor + "; border-radius: 3px; padding: 3px; margin-left: 2px; }");
    }

    QSettings settings;
    QVariant var = settings.value(SETTINGS_SOLOFRAME_SIZE);
    if (var.isValid() == true)
        resize(var.toSize());
    else
        resize(defaultSize);
    m_width = this->width();
    m_height = this->height();
}

VCSoloFrame::~VCSoloFrame()
{
}

/*****************************************************************************
 * Clipboard
 *****************************************************************************/

VCWidget* VCSoloFrame::createCopy(VCWidget* parent)
{
    Q_ASSERT(parent != NULL);

    VCSoloFrame* frame = new VCSoloFrame(parent, m_doc, true);
    if (frame->copyFrom(this) == false)
    {
        delete frame;
        frame = NULL;
    }

    return frame;
}

bool VCSoloFrame::copyFrom(const VCWidget* widget)
{
    const VCSoloFrame* frame = qobject_cast<const VCSoloFrame*> (widget);
    if (frame == NULL)
        return false;

    setSoloframeMixing(frame->soloframeMixing());

    return VCFrame::copyFrom(widget);
}

/*****************************************************************************
* Solo behaviour
*****************************************************************************/

void VCSoloFrame::updateChildrenConnection(bool doConnect)
{
    QListIterator <VCWidget*> it(findChildren<VCWidget*>());
    while (it.hasNext())
    {
        VCWidget* widget = it.next();
        if (widget != NULL && thisIsNearestSoloFrameParent(widget))
        {
            if (doConnect)
            {
                connect(widget, SIGNAL(functionStarting(quint32, qreal)),
                        this, SLOT(slotWidgetFunctionStarting(quint32, qreal)));
            }
            else
            {
                disconnect(widget, SIGNAL(functionStarting(quint32, qreal)),
                        this, SLOT(slotWidgetFunctionStarting(quint32, qreal)));
            }
        }
    }
}

void VCSoloFrame::slotModeChanged(Doc::Mode mode)
{
    VCFrame::slotModeChanged(mode);

    updateChildrenConnection(mode == Doc::Operate);
}

void VCSoloFrame::setLiveEdit(bool liveEdit)
{
    VCFrame::setLiveEdit(liveEdit);

    if (m_doc->mode() == Doc::Design)
        return;

    updateChildrenConnection(!liveEdit);
}

bool VCSoloFrame::thisIsNearestSoloFrameParent(QWidget* widget)
{
    while (widget != NULL)
    {
        widget = widget->parentWidget();

        VCSoloFrame *sf = qobject_cast<VCSoloFrame*>(widget);
        if (sf != NULL)
        {
            return sf == this;
        }
    }

    return false;
}

void VCSoloFrame::slotWidgetFunctionStarting(quint32 fid, qreal intensity)
{
    VCWidget* senderWidget = qobject_cast<VCWidget*>(sender());

    if (senderWidget != NULL)
    {
        // get every widget that is a child of this soloFrame and turn their
        // functions off
        QListIterator <VCWidget*> it(findChildren<VCWidget*>());

        while (it.hasNext() == true)
        {
            VCWidget* widget = it.next();
            if (widget != NULL && widget != senderWidget)
                widget->notifyFunctionStarting(fid, soloframeMixing() ? intensity : 1.0);
        }
    }
}

/*****************************************************************************
 * Properties
 *****************************************************************************/

void VCSoloFrame::editProperties()
{
    VCSoloFrameProperties prop(NULL, this, m_doc);
    if (prop.exec() == QDialog::Accepted)
    {
        applyProperties(prop);
    }
};

bool VCSoloFrame::soloframeMixing() const
{
    return m_soloframeMixing;
}

void VCSoloFrame::setSoloframeMixing(bool soloframeMixing)
{
    m_soloframeMixing = soloframeMixing;
}

/*****************************************************************************
 * Load & Save
 *****************************************************************************/

QString VCSoloFrame::xmlTagName() const
{
    return KXMLQLCVCSoloFrame;
}

/*****************************************************************************
 * Event handling
 *****************************************************************************/

void VCSoloFrame::paintEvent(QPaintEvent* e)
{
    /* No point coming here if there is no VC instance */
    VirtualConsole* vc = VirtualConsole::instance();
    if (vc == NULL)
        return;

    QPainter painter(this);

    QWidget::paintEvent(e);

    /* Draw selection frame */
    bool drawSelectionFrame = false;
    if (mode() == Doc::Design && vc->isWidgetSelected(this) == true)
        drawSelectionFrame = true;

    /* Draw a dotted line around the widget */
    QPen pen(drawSelectionFrame ? Qt::DashLine : Qt::SolidLine);
    pen.setColor(Qt::red);

    if (drawSelectionFrame == true)
    {
        pen.setCapStyle(Qt::RoundCap);
        pen.setWidth(0);
    }
    else
    {
        pen.setCapStyle(Qt::FlatCap);
        pen.setWidth(1);
    }

    painter.setPen(pen);
    painter.drawRect(0, 0, rect().width()-1, rect().height()-1);

    if (drawSelectionFrame)
    {
        /* Draw a resize handle */
        QIcon icon(":/resize.png");
        painter.drawPixmap(rect().width() - 16, rect().height() - 16,
                           icon.pixmap(QSize(16, 16), QIcon::Normal, QIcon::On));
    }
}
