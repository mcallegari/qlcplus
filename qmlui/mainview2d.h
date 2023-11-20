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
    Q_PROPERTY(QPoint gridPosition READ gridPosition WRITE setGridPosition NOTIFY gridPositionChanged)
    Q_PROPERTY(int gridUnits READ gridUnits WRITE setGridUnits NOTIFY gridUnitsChanged)
    Q_PROPERTY(qreal gridScale READ gridScale WRITE setGridScale NOTIFY gridScaleChanged)
    Q_PROPERTY(qreal cellPixels READ cellPixels WRITE setCellPixels NOTIFY cellPixelsChanged)
    Q_PROPERTY(int pointOfView READ pointOfView WRITE setPointOfView NOTIFY pointOfViewChanged)
    Q_PROPERTY(QString backgroundImage READ backgroundImage WRITE setBackgroundImage NOTIFY backgroundImageChanged)

public:
    explicit MainView2D(QQuickView *view, Doc *doc, QObject *parent = 0);
    ~MainView2D();

    /** @reimp */
    void enableContext(bool enable);

    /** @reimp */
    void setUniverseFilter(quint32 universeFilter);

    void resetItems();

    void createFixtureItems(quint32 fxID, QVector3D pos, bool mmCoords = true);

    void createFixtureItem(quint32 fxID, quint16 headIndex, quint16 linkedIndex,
                           QVector3D pos, bool mmCoords = true);

    /** Set/update the flags of a fixture item */
    void setFixtureFlags(quint32 itemID, quint32 flags);

    /** Select some Fixtures included in the provided $rect area */
    QList<quint32> selectFixturesRect(QRectF rect);

    /** Return the ID of a Fixture at the given $pos or -1 if not found */
    Q_INVOKABLE int itemIDAtPos(QPointF pos);

    /** Update the fixture preview items when some channels have changed */
    void updateFixture(Fixture *fixture, QByteArray &previous);

    /** Update a single fixture item for a specific Fixture ID, head index and linked index */
    void updateFixtureItem(Fixture *fixture, quint16 headIndex, quint16 linkedIndex, QByteArray &previous);

    /** Update the selection status of a list of Fixture item IDs */
    void updateFixtureSelection(QList<quint32>fixtures);

    /** Update the selection status of a Fixture with the provided $itemID */
    void updateFixtureSelection(quint32 itemID, bool enable);

    /** Update the rotation of a Fixture with the provided $itemID */
    void updateFixtureRotation(quint32 itemID, QVector3D degrees);

    /** Update the position of a Fixture with the provided $itemID */
    void updateFixturePosition(quint32 itemID, QVector3D pos);

    /** Update the position of a Fixture with the provided $itemID */
    void updateFixtureSize(quint32 itemID, Fixture *fixture);

    /** Remove a Fixture item with the provided $itemID from the preview */
    void removeFixtureItem(quint32 itemID);

    /** Get/Set the grid width/height */
    QSize gridSize() const;
    void setGridSize(QVector3D sz);

    /** Get/Set the grid position in pixels */
    QPoint gridPosition() const;
    void setGridPosition(QPoint pos);

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

    /** Get/Set the main background image */
    QString backgroundImage();
    void setBackgroundImage(QString image);

protected:
    /** First time 2D view variables initializations */
    bool initialize2DProperties();

    /** Update the Quick item selection and reparent for dragging if needed */
    void selectFixture(QQuickItem *fxItem, bool enable);

signals:
    void gridSizeChanged();
    void gridPositionChanged();
    void gridUnitsChanged();
    void gridScaleChanged(qreal gridScale);
    void cellPixelsChanged(qreal cellPixels);
    void pointOfViewChanged(int pointOfView);
    void backgroundImageChanged();

public slots:
    /** @reimp */
    void slotRefreshView();

private:
    /** References to the 2D grid item for positioning */
    QQuickItem *m_gridItem;

    /** Reference to the Doc Monitor properties */
    MonitorProperties *m_monProps;

    /** Size of the grid. How many horizontal and vertical cells */
    QSize m_gridSize;
    /** X/Y offset of the grid (in pixels) to keep it centered */
    QPoint m_gridPosition;

    /** Scale of the grid */
    qreal m_gridScale;

    /** Size of a grid cell in pixels */
    qreal m_cellPixels;

    /** Pre-cached QML component for quick item creation */
    QQmlComponent *fixtureComponent;
};

#endif // MAINVIEW2D_H
