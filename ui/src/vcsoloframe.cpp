/*
  Q Light Controller
  vcsoloframe.cpp

  Copyright (c) Anders Thomsen

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  Version 2 as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details. The license is
  in the file "COPYING".

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
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
    QListIterator <VCButton*> it(findChildren<VCButton*>());

    while (it.hasNext() == true)
    {
        VCButton* button = it.next();

        // make sure the buttons nearest soloframe is this
        if (thisIsNearestSoloFrameParent(button))
        {
            if (mode == Doc::Operate)
            {
                // listen to when the button function is started
                connect(button, SIGNAL(functionStarting()),
                        this, SLOT(slotButtonFunctionStarting()),
                        Qt::DirectConnection);
            }
            else
            {
                // remove listener
                connect(button, SIGNAL(functionStarting()),
                        this, SLOT(slotButtonFunctionStarting()));
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

void VCSoloFrame::slotButtonFunctionStarting()
{
    VCButton* senderButton = qobject_cast<VCButton*>(sender());

    if (senderButton != NULL)
    {
        // get every button that is a child of this soloFrame and turn their
        // functions off
        QListIterator <VCButton*> it(findChildren<VCButton*>());

        while (it.hasNext() == true)
        {
            VCButton* button = it.next();
            if (button->action() == VCButton::Toggle)
            {
                Function* f = m_doc->function(button->function());
                if (f != NULL)
                {
                    f->stopAndWait();
                }
            }
        }
    }
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
