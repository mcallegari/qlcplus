/*
  Q Light Controller
  vcxypadarea.cpp

  Copyright (c) Heikki Junnila, Stefan Krumm

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

#include <QMouseEvent>
#include <QPainter>
#include <QPixmap>
#include <QCursor>
#include <QMutex>
#include <QDebug>
#include <QPoint>

#include "qlcmacros.h"

#include "vcxypadarea.h"
#include "vcframe.h"

/*****************************************************************************
 * Initialization
 *****************************************************************************/

VCXYPadArea::VCXYPadArea(QWidget* parent)
    : QFrame(parent)
    , m_changed(false)
    , m_pixmap(QPixmap(":/xypad-point.png"))
    , m_rangeSrcRect(QRect())
    , m_rangeDestRect(QRect())
{
    setFrameStyle(KVCFrameStyleSunken);
    setWindowTitle("XY Pad");
    setMode(Doc::Design);
}

VCXYPadArea::~VCXYPadArea()
{
}

void VCXYPadArea::setMode(Doc::Mode mode)
{
    m_mode = mode;
    if (mode == Doc::Design)
        setEnabled(false);
    else
        setEnabled(true);
    update();
}

/*****************************************************************************
 * Current XY position
 *****************************************************************************/

QPoint VCXYPadArea::position()
{
    m_mutex.lock();
    QPoint pos(m_pos);
    m_changed = false;
    m_mutex.unlock();
    return pos;
}

void VCXYPadArea::setPosition(const QPoint& point)
{
    m_mutex.lock();
    if (m_pos != point)
    {
        m_pos = point;
        m_changed = true;
    }
    m_mutex.unlock();

    emit positionChanged(point);
}

bool VCXYPadArea::hasPositionChanged()
{
    m_mutex.lock();
    bool changed = m_changed;
    m_mutex.unlock();
    return changed;
}

/*************************************************************************
 * Range window
 *************************************************************************/

QRect VCXYPadArea::rangeWindow()
{
    return m_rangeDestRect;
}

void VCXYPadArea::setRangeWindow(QRect rect)
{
    m_rangeSrcRect = rect;
    updateRangeWindow();
}

void VCXYPadArea::updateRangeWindow()
{
    int x = (width() * m_rangeSrcRect.x()) / 255;
    int y = (height() * m_rangeSrcRect.y()) / 255;
    int w = (width() * m_rangeSrcRect.width()) / 255;
    int h = (height() * m_rangeSrcRect.height()) / 255;
    m_rangeDestRect = QRect(x, y, w - x, h - y);
}

/*****************************************************************************
 * Event handlers
 *****************************************************************************/

void VCXYPadArea::paintEvent(QPaintEvent* e)
{
    /* Let the parent class draw its stuff first */
    QFrame::paintEvent(e);

    QPainter p(this);
    QPen pen;

    /* Draw name (offset just a bit to avoid frame) */
    p.drawText(1, 1, width() - 2, height() - 2,
               Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap, windowTitle());

    /* Draw crosshairs to indicate the center position */
    pen.setStyle(Qt::DotLine);
    pen.setColor(palette().color(QPalette::WindowText));
    pen.setWidth(0);
    p.setPen(pen);
    p.drawLine(width() / 2, 0, width() / 2, height());
    p.drawLine(0, height() / 2, width(), height() / 2);

    /* Draw the range window if not full size */
    if (m_rangeDestRect.isValid())
    {
        pen.setStyle(Qt::SolidLine);
        pen.setColor(QColor(0, 120, 0, 170));
        p.setPen(pen);
        p.fillRect(m_rangeDestRect, QBrush(QColor(155, 200, 165, 130)));
        p.drawRect(m_rangeDestRect);
        if (m_mode == Doc::Operate)
        {
            QPoint pt(CLAMP(m_pos.x(), m_rangeDestRect.x(),
                                       m_rangeDestRect.x() + m_rangeDestRect.width()),
                      CLAMP(m_pos.y(), m_rangeDestRect.y(),
                                       m_rangeDestRect.y() + m_rangeDestRect.height()));
            setPosition(pt);
        }
    }

    /* Draw the current point pixmap */
    p.drawPixmap(m_pos.x() - (m_pixmap.width() / 2),
                 m_pos.y() - (m_pixmap.height() / 2),
                 m_pixmap);
}

void VCXYPadArea::resizeEvent(QResizeEvent *e)
{
    QFrame::resizeEvent(e);
    updateRangeWindow();
}

void VCXYPadArea::mousePressEvent(QMouseEvent* e)
{
    if (m_mode == Doc::Operate)
    {
        QPoint pt(CLAMP(e->x(), 0, width()), CLAMP(e->y(), 0, height()));
        setPosition(pt);
        setMouseTracking(true);
        setCursor(Qt::CrossCursor);
        update();
    }

    QFrame::mousePressEvent(e);
}

void VCXYPadArea::mouseReleaseEvent(QMouseEvent* e)
{
    if (m_mode == Doc::Operate)
    {
        QPoint pt(CLAMP(e->x(), 0, width()), CLAMP(e->y(), 0, height()));
        setPosition(pt);
        setMouseTracking(false);
        unsetCursor();
    }

    QFrame::mouseReleaseEvent(e);
}

void VCXYPadArea::mouseMoveEvent(QMouseEvent* e)
{
    if (m_mode == Doc::Operate)
    {
        QPoint pt(CLAMP(e->x(), 0, width()), CLAMP(e->y(), 0, height()));
        setPosition(pt);
        update();
    }

    QFrame::mouseMoveEvent(e);
}

