/*
  Q Light Controller
  vcxypadarea.h

  Copyright (c) Stefan Krumm, Heikki Junnila

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

#ifndef VCXYPADAREA_H
#define VCXYPADAREA_H

#include <QPixmap>
#include <QString>
#include <QMutex>
#include <QFrame>

#include "doc.h"

class QPaintEvent;
class QMouseEvent;

class VCXYPadArea : public QFrame
{
    Q_OBJECT
    Q_DISABLE_COPY(VCXYPadArea)

    /*************************************************************************
     * Initialization
     *************************************************************************/
public:
    VCXYPadArea(QWidget* parent);
    virtual ~VCXYPadArea();

    void setMode(Doc::Mode mode);

private:
    Doc::Mode m_mode;

    /*************************************************************************
     * Current position
     *************************************************************************/
public:
    /** Get the pad's current position (i.e. where the point is) */
    QPoint position();

    /** Set the pad's current position (i.e. move the point) */
    void setPosition(const QPoint& point);

    /** Check if the position has changed since the last currentXYPosition() call */
    bool hasPositionChanged();

signals:
    void positionChanged(const QPoint& point);

private:
    QPoint m_pos;
    bool m_changed;
    QMutex m_mutex;
    QPixmap m_pixmap;

    /*************************************************************************
     * Range window
     *************************************************************************/
public:
    QRect rangeWindow();

    void setRangeWindow(QRect rect);

private:
    void updateRangeWindow();
private:
    QRect m_rangeSrcRect;
    QRect m_rangeDestRect;

    /*************************************************************************
     * Event handlers
     *************************************************************************/
protected:
    /** @reimp */
    void paintEvent(QPaintEvent* e);

    /** @reimp */
    void resizeEvent(QResizeEvent *e);

    /** @reimp */
    void mousePressEvent(QMouseEvent* e);

    /** @reimp */
    void mouseReleaseEvent(QMouseEvent* e);

    /** @reimp */
    void mouseMoveEvent(QMouseEvent* e);
};

#endif
