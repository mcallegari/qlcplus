/*
  Q Light Controller Plus
  mainview2d.h

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

#ifndef MAINVIEW2D_H
#define MAINVIEW2D_H

#include <QObject>
#include <QQuickView>

#include "previewcontext.h"

class Doc;

class MainView2D : public PreviewContext
{
    Q_OBJECT
public:
    explicit MainView2D(QQuickView *view, Doc *doc, QObject *parent = 0);
    ~MainView2D();

    /** @reimp */
    void enableContext(bool enable);

    void reset();

    void createFixtureItem(quint32 fxID, qreal x, qreal y, bool mmCoords = true);

protected:
    /** First time 2D view variables initializations */
    void initialize2DProperties();

    /** Returns the first available space (in mm) for a rectangle
     * of the given width and height. */
    QPointF getAvailablePosition(QRectF &fxRect);

signals:

protected slots:
    void slotRefreshView();
    void slotUniversesWritten(int idx, const QByteArray& ua);

private:
    /** References to the 2D view and 2D contents for items creation */
    QQuickItem *m_view2D, *m_contents2D;

    /** Size of the grid. How many horizontal and vertical cells */
    QSize m_gridSize;

    /** The unit used by the grid. Meters = 1000mm, Feet = 304.8mm */
    float m_gridUnits;

    /** Scale of the grid */
    qreal m_gridScale;

    /** Size of a grid cell in pixels */
    qreal m_cellPixels;

    /** X offset of the grid to keep it centered */
    qreal m_xOffset;

    /** Y offset of the grid to keep it centered */
    qreal m_yOffset;

    /** Map of the fixture 2D items by ID */
    QMap<quint32, QQuickItem*> m_itemsMap;

    /** Pre-cached QML component for quick item creation */
    QQmlComponent *fixtureComponent;
};

#endif // MAINVIEW2D_H
