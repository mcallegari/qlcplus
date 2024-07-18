/*
  Q Light Controller Plus
  monitorgraphicsview.cpp

  Copyright (C) Massimo Callegari

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

#ifndef MONITORGRAPHICSVIEW_H
#define MONITORGRAPHICSVIEW_H

#include <QGraphicsView>
#include <QHash>

#include "fixture.h"

class MonitorProperties;
class MonitorFixtureItem;
class Doc;

/** \addtogroup ui_mon DMX Monitor
 * @{
 */

class MonitorGraphicsView : public QGraphicsView
{
    Q_OBJECT

public:
    MonitorGraphicsView(Doc *doc, QWidget *parent = 0);
    ~MonitorGraphicsView();

    /** Set the graphics view size in monitor units */
    void setGridSize(QSize size);

    /** Get the grid size in monitor units */
    QSize gridSize() const { return m_gridSize; }

    /** Set the measure unit to use */
    void setGridMetrics(float value);

    /** Get the currently selected fixture ID.
     *  Fixture::invalidId is returned if none is selected */
    quint32 selectedFixtureID();

    /** Return a list of the fixture IDs in the current view */
    QList <quint32> fixturesID() const;

    /** Retrieve the currently selected MonitorFixtureItem.
     *  Return NULL if none */
    MonitorFixtureItem *getSelectedItem();

    /** Set the gel color of the fixture with the given ID */
    void setFixtureGelColor(quint32 id, QColor col);

    /** Set the rotation degrees of the fixture with the given ID */
    void setFixtureRotation(quint32 id, ushort degrees);

    /** Show/hide fixtures items labels */
    void showFixturesLabels(bool visible);

    /** Return the gel color of the fixture with the given ID */
    QColor fixtureGelColor(quint32 id);

    /** Add a fixture to the current view */
    void addFixture(quint32 id, QPointF pos = QPointF(0, 0));

    /** Remove the fixture with the given ID from the view
     *  If no ID is specified, the currently selected
     *  fixture will be removed (if possible)
     */
    bool removeFixture(quint32 id = Fixture::invalidId());

    /** Remove all fixtures from the current view */
    void clearFixtures();

    /** Support function to convert a position in millimeters
     *  to a position in pixels */
    QPointF realPositionToPixels(qreal xpos, qreal ypos);

    /** Update the position and the scale of the fixture with
     *  the given ID
     */
    void updateFixture(quint32 id);

    /** Set a background image for the view */
    void setBackgroundImage(QString filename);

    /** Retrieve the path to the background image currently set */
    QString backgroundImage() { return m_backgroundImage; }

protected:
    /** Triggers the whole view repaint and metrics
     *  computation */
    void updateGrid();

    /** Event caught when the GraphicsView is resized */
    void resizeEvent(QResizeEvent *event);

public slots:
    void mouseReleaseEvent(QMouseEvent *e);

protected slots:
    /** Slot called when a MonitorFixtureItem is dropped after a drag */
    void slotFixtureMoved(MonitorFixtureItem *item);

signals:
    /** Signal emitted after fixture point -> metrics conversion */
    void fixtureMoved(quint32 id, QPointF pos);

    /** Signal emitted when the graphics view is clicked */
    void viewClicked(QMouseEvent *e);

private:
    Doc *m_doc;
    QGraphicsScene *m_scene;

    /** Size of the grid. How many horizontal and vertical cells */
    QSize m_gridSize;

    /** Size of a grid cell in pixels */
    int m_cellPixels;

    /** X offset of the grid to keep it centered */
    qreal m_xOffset;

    /** Y offset of the grid to keep it centered */
    qreal m_yOffset;

    /** The unit used by the grid. Meters = 1000mm, Feet = 304.8mm */
    float m_unitValue;

    /** List of Fixture items represented graphically */
    QList <QGraphicsLineItem *> m_gridItems;

    /** Flag to enable/disable the grid rendering */
    bool m_gridEnabled;

    /** Path to the view background image */
    QString m_backgroundImage;

    QPixmap m_bgPixmap;

    QGraphicsPixmapItem *m_bgItem;

    /** Map of the rendered MonitorFixtureItem with their ID */
    QHash <quint32, MonitorFixtureItem*> m_fixtures;
};

/** @} */

#endif // MONITORGRAPHICSVIEW_H
