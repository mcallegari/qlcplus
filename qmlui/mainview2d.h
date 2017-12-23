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
class QLCFixtureMode;
class MonitorProperties;

class MainView2D : public PreviewContext
{
    Q_OBJECT

    Q_PROPERTY(QSize gridSize READ gridSize NOTIFY gridSizeChanged)
    Q_PROPERTY(int gridUnits READ gridUnits WRITE setGridUnits NOTIFY gridUnitsChanged)
    Q_PROPERTY(qreal gridScale READ gridScale WRITE setGridScale NOTIFY gridScaleChanged)
    Q_PROPERTY(qreal cellPixels READ cellPixels WRITE setCellPixels NOTIFY cellPixelsChanged)
    Q_PROPERTY(int pointOfView READ pointOfView WRITE setPointOfView NOTIFY pointOfViewChanged)

public:
    explicit MainView2D(QQuickView *view, Doc *doc, QObject *parent = 0);
    ~MainView2D();

    /** @reimp */
    void enableContext(bool enable);

    /** @reimp */
    void setUniverseFilter(quint32 universeFilter);

    void resetItems();

    void createFixtureItem(quint32 fxID, QVector3D pos, bool mmCoords = true);

    QList<quint32> selectFixturesRect(QRectF rect);

    void updateFixture(Fixture *fixture);

    void updateFixtureSelection(QList<quint32>fixtures);

    void updateFixtureSelection(quint32 fxID, bool enable);

    void updateFixtureRotation(quint32 fxID, QVector3D degrees);

    void updateFixturePosition(quint32 fxID, QVector3D pos);

    void removeFixtureItem(quint32 fxID);

    /** Get/Set the grid width/height */
    QSize gridSize() const;
    void setGridSize(QVector3D sz);

    /** Get/Set the grid measurement units */
    int gridUnits() const;
    void setGridUnits(int units);

    /** Get/Set a temporary value of scaling */
    qreal gridScale() const;
    void setGridScale(qreal gridScale);

    qreal cellPixels() const;
    void setCellPixels(qreal cellPixels);

    /** Get/Set the 2D grid point of view */
    int pointOfView() const;
    void setPointOfView(int pointOfView);

protected:
    /** First time 2D view variables initializations */
    void initialize2DProperties();

    QPointF item2DPosition(QVector3D pos, bool mmCoords);
    float item2DRotation(QVector3D rot);
    QSizeF item2DDimension(QLCFixtureMode *fxMode);

signals:
    void gridSizeChanged();
    void gridUnitsChanged();
    void gridScaleChanged(qreal gridScale);
    void cellPixelsChanged(qreal cellPixels);
    void pointOfViewChanged(int pointOfView);

protected slots:
    /** @reimp */
    void slotRefreshView();

private:
    /** References to the 2D view and 2D contents for items creation */
    QQuickItem *m_contents2D;

    /** Reference to the Doc Monitor properties */
    MonitorProperties *m_monProps;

    /** Size of the grid. How many horizontal and vertical cells */
    QSize m_gridSize;

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
