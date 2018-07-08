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

class EFXPreviewArea;
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
    QPointF position(bool resetChanged = true) const;

    /** Set the pad's current position (i.e. move the point) */
    void setPosition(const QPointF& point);

    /** Move the current position by some relative amount */
    void nudgePosition(qreal dx, qreal dy);

    /** Check if the position has changed since the last currentXYPosition() call */
    bool hasPositionChanged();

signals:
    void positionChanged(const QPointF& point);

public slots:
    void slotFixturePositions(const QVariantList positions);

private:
    /** Make sure the m_dmxPos is inside m_rangeDmxRect */
    void checkDmxRange();

    /** Compute m_windowPos from mdmxPos */
    void updateWindowPos();

    QString positionString() const;

    QString angleString() const;

private:

    /** Position in DMX coordinates 0.0..(256.0 - 1/256) */
    QPointF m_dmxPos;

    /** Position in window coordinates */
    QPoint m_windowPos;

    /** Optimization - compute window pos on demand */
    bool m_updateWindowPos;

    mutable bool m_changed;
    mutable QMutex m_mutex;

    /** Used to display active point - blue */
    QPixmap m_activePixmap;

    /** Used to display fixture positions - yellow */
    QPixmap m_fixturePixmap;

    QVariantList m_fixturePositions;

    /*************************************************************************
     * Range window
     *************************************************************************/
public:
    QRectF rangeWindow();

    void setRangeWindow(QRectF rect);

private:
    /** Compute m_rangeWindowRect from m_rangeDmxRect */
    void updateRangeWindow();

private:
    /** Range in dmx domain */
    QRectF m_rangeDmxRect;

    /** Range in window coordinates */
    QRect m_rangeWindowRect;

    /*************************************************************************
     * Degrees range
     *************************************************************************/
public:
    QRectF degreesRange() const;

    void setDegreesRange(QRectF rect);

private:
    /** Range in degrees (for the full range) */
    QRectF m_degreesRange;

    /*************************************************************************
     * EFX Preview
     *************************************************************************/
public:
    void enableEFXPreview(bool enable);
    void setEFXPolygons(const QPolygonF& pattern, const QVector<QPolygonF> fixtures);
    void setEFXInterval(uint duration);

private:
    EFXPreviewArea* m_previewArea;

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
