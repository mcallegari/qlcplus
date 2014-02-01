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

class MonitorFixtureItem;
class Doc;

class MonitorGraphicsView : public QGraphicsView
{
    Q_OBJECT

public:
    MonitorGraphicsView(Doc *doc, QWidget *parent = 0);

    void setGridSize(QSize size);

    void setGridMetrics(float value);

    QList <quint32> fixturesID() const;

    void addFixtureItem(quint32 id, QPointF pos = QPointF(0, 0));

protected:

    void updateGrid();

    void updateFixtureSize(quint32 id);

    /** Event caught when the GraphicsView is resized */
    void resizeEvent( QResizeEvent *event );

protected slots:
    /** Slot called when a MonitorFixtureItem is dropped after a drag */
    void slotFixtureMoved(MonitorFixtureItem * item);

signals:
    /** Signal emitted after fixture point -> metrics conversion */
    void fixtureMoved(quint32 id, QPointF pos);

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

    bool m_gridEnabled;

    QHash <quint32, MonitorFixtureItem*> m_fixtures;
};

#endif // MONITORGRAPHICSVIEW_H
