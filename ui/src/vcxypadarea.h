/*
  Q Light Controller
  vcxypadarea.h

  Copyright (c) Stefan Krumm, Heikki Junnila

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

#ifndef VCXYPADAREA_H
#define VCXYPADAREA_H

#include <QPixmap>
#include <QString>
#include <QMutex>
#include <QFrame>

#include "doc.h"

class QPaintEvent;
class QMouseEvent;

/** @addtogroup ui_vc_widgets
 * @{
 */

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

    /** Move the current position by some relative amount */
    void nudgePosition(int dx, int dy);

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

    /** @reimp */
    void keyPressEvent(QKeyEvent *e);

    /** @reimp */
    void keyReleaseEvent (QKeyEvent * e);
};

/** @} */

#endif
