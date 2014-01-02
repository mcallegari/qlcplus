/*
  Q Light Controller
  efxpreviewarea.h

  Copyright (C) Heikki Junnila

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

#ifndef EFXPREVIEWAREA_H
#define EFXPREVIEWAREA_H

#include <QPolygon>
#include <QWidget>
#include <QTimer>

#include "ui_efxeditor.h"
#include "efx.h"

class QPaintEvent;

/** @addtogroup ui_functions
 * @{
 */

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

/** @} */

#endif
