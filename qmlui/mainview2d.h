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
class Fixture;
class MonitorProperties;

class MainView2D : public PreviewContext
{
    Q_OBJECT

    Q_PROPERTY(QSize gridSize READ gridSize WRITE setGridSize NOTIFY gridSizeChanged)
    Q_PROPERTY(float gridUnits READ gridUnits WRITE setGridUnits NOTIFY gridUnitsChanged)

public:
    explicit MainView2D(QQuickView *view, Doc *doc, QObject *parent = 0);
    ~MainView2D();

    /** @reimp */
    void enableContext(bool enable);

    /** @reimp */
    void setUniverseFilter(quint32 universeFilter);

    void resetItems();

    void createFixtureItem(quint32 fxID, qreal x, qreal y, bool mmCoords = true);

    QList<quint32> selectFixturesRect(QRectF rect);

    void updateFixture(Fixture *fixture);

    void updateFixtureSelection(QList<quint32>fixtures);

    void updateFixtureSelection(quint32 fxID, bool enable);

    void updateFixtureRotation(quint32 fxID, int degrees);

    QSize gridSize() const;
    void setGridSize(QSize sz);

    float gridUnits() const;
    void setGridUnits(float units);

protected:
    /** First time 2D view variables initializations */
    void initialize2DProperties();

signals:
    void gridSizeChanged();
    void gridUnitsChanged();

protected slots:
    void slotRefreshView();

private:
    /** References to the 2D view and 2D contents for items creation */
    QQuickItem *m_view2D, *m_contents2D;

    MonitorProperties *m_monProps;

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

    /** Pre-cached QML component for quick item creation */
    QQmlComponent *fixtureComponent;
};

#endif // MAINVIEW2D_H
