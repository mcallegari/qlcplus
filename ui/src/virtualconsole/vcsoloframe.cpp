/*
  Q Light Controller
  vcsoloframe.cpp

  Copyright (c) Anders Thomsen

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

#include <QStyleOptionFrameV2>
#include <QMetaObject>
#include <QMessageBox>
#include <QPainter>
#include <QAction>
#include <QStyle>
#include <QDebug>
#include <QPoint>
#include <QSize>
#include <QMenu>
#include <QList>
#include <QtXml>

#include "vcpropertieseditor.h"
#include "virtualconsole.h"
#include "vcsoloframe.h"
#include "vcbutton.h"
#include "function.h"
#include "qlcfile.h"
#include "doc.h"

VCSoloFrame::VCSoloFrame(QWidget* parent, Doc* doc, bool canCollapse) : VCFrame(parent, doc, canCollapse)
{
    /* Set the class name "VCSoloFrame" as the object name as well */
    setObjectName(VCSoloFrame::staticMetaObject.className());
    setType(VCWidget::SoloFrameWidget);

    m_frameStyle = KVCFrameStyleSunken;

    if(canCollapse == true)
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
        resize(VCFrame::defaultSize);
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

/*****************************************************************************
* Solo behaviour
*****************************************************************************/

void VCSoloFrame::slotModeChanged(Doc::Mode mode)
{
    VCFrame::slotModeChanged(mode);

    // Get all buttons in this soloFrame
    QListIterator <VCWidget*> it(findChildren<VCWidget*>());

    while (it.hasNext() == true)
    {
        VCWidget* widget = it.next();
        if (widget != NULL)
        {
            if (widget->type() == VCWidget::ButtonWidget ||
                widget->type() == VCWidget::SliderWidget ||
                widget->type() == VCWidget::CueListWidget)
            {
                // make sure the widget's nearest soloframe is this
                if (thisIsNearestSoloFrameParent(widget))
                {
                    if (mode == Doc::Operate)
                    {
                        // listen to when the button function is started
                        connect(widget, SIGNAL(functionStarting()),
                                this, SLOT(slotWidgetFunctionStarting()));
                    }
                    else
                    {
                        // remove listener
                        disconnect(widget, SIGNAL(functionStarting()),
                                   this, SLOT(slotWidgetFunctionStarting()));
                    }
                }
            }
        }
    }
}

bool VCSoloFrame::thisIsNearestSoloFrameParent(QWidget* widget)
{
	VCSoloFrame* sf;

    while (widget != NULL)
    {
        widget = widget->parentWidget();

        sf = qobject_cast<VCSoloFrame*>(widget);
        if (sf != NULL)
        {
			return sf == this;
		}
    }

    return false;
}

void VCSoloFrame::slotWidgetFunctionStarting()
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
                widget->stopFunction();
        }
    }
}

QString VCSoloFrame::getCSS()
{
    QString str = "<style>\n"
            " .vcsoloframe {\n"
            " position: absolute;\n"
            " border-radius: 4px;\n"
            "}\n\n"

            ".vcsoloframeHeader {\n"
            " background: linear-gradient(to bottom, #BC0A0A 0%, #370303 100%);\n"
            " background: -ms-linear-gradient(top, #BC0A0A 0%, #370303 100%);\n"
            " background: -moz-linear-gradient(top, #BC0A0A 0%, #370303 100%);\n"
            " background: -o-linear-gradient(top, #BC0A0A 0%, #370303 100%);\n"
            " background: -webkit-gradient(linear, left top, left bottom, color-stop(0, #BC0A0A), color-stop(1, #370303));\n"
            " background: -webkit-linear-gradient(top, #BC0A0A 0%, #370303 100%);\n"
            " border-radius: 3px;\n"
            " margin: 2px;\n"
            " padding: 0 0 0 3px;\n"
            " height: 32px;\n"
            " font:normal 20px/1.2em sans-serif;\n"
            "}\n"
            "</style>\n";

    return str;
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
