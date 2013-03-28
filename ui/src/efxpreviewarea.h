/*
  Q Light Controller
  efxpreviewarea.h

  Copyright (C) Heikki Junnila

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

#ifndef EFXPREVIEWAREA_H
#define EFXPREVIEWAREA_H

#include <QPolygon>
#include <QWidget>
#include <QTimer>

#include "ui_efxeditor.h"
#include "efx.h"

class QPaintEvent;

/**
 * The area that is used to draw a preview of
 * the EFX function currently being edited.
 */
class EFXPreviewArea : public QWidget
{
    Q_OBJECT

public:
    EFXPreviewArea(QWidget* parent);
    ~EFXPreviewArea();

    /**
     * Set an an array of X-Y points that can be used for drawing a preview
     *
     * @param points The point array
     */
    void setPoints(const QVector<QPoint>& points);

    /**
     * Set an an array of X-Y points that can be used for drawing individual fixture positions
     *
     * @param fixturePoints The array of point arrays (one array for each fixture)
     */
    void setFixturePoints(const QVector<QVector<QPoint> >& fixturePoints);

    /**
     * Tell the preview area to draw the points.
     *
     * @param timerInterval Timer interval between repaints in milliseconds
     */
    void draw(int timerInterval = 20);

    /** Scale the points in the given polygon of size [0, 255] to the given target size */
    static QPolygon scale(const QPolygon& poly, const QSize& target);

protected:
    /** @reimp */
    void resizeEvent(QResizeEvent* e);

    /** @reimp */
    void paintEvent(QPaintEvent* e);

private slots:
    /** Animation timeout */
    void slotTimeout();

private:
    /** Points that are drawn in the preview area */
    QPolygon m_points;
    QPolygon m_original;

    QVector <QPolygon> m_fixturePoints;
    QVector <QPolygon> m_originalFixturePoints;

    /** Animation timer */
    QTimer m_timer;

    /** Animation position */
    int m_iter;
};

#endif
