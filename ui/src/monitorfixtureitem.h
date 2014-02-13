/*
  Q Light Controller Plus
  monitorfixtureitem.h

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

#ifndef MONITORFIXTUREITEM_H
#define MONITORFIXTUREITEM_H

#include <QGraphicsItem>

class Doc;

typedef struct
{
    QGraphicsEllipseItem *m_item;
    QList <quint32> m_rgb;
    QList <quint32> m_cmy;
} FixtureHead;

class MonitorFixtureItem : public QObject, public QGraphicsItem
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)

public:
    MonitorFixtureItem(Doc *doc, quint32 fid);

    void setRealPosition(QPointF pos) { m_realPos = pos; }
    QPointF realPosition() { return m_realPos; }

    void setSize(QSize size);

    quint32 fixtureID() { return m_fid; }

    int headsCount() { return m_heads.count(); }

    void updateValues(const QByteArray& ua);

protected:
    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *);

signals:
    void itemDropped(MonitorFixtureItem *);

private:
    Doc *m_doc;
    /** The Fixture ID this item is associated to */
    quint32 m_fid;

    /** Width of the item */
    int m_width;

    /** Height of the item */
    int m_height;

    /** Position of the item top-left corner in millimeters */
    QPointF m_realPos;

    QList <FixtureHead> m_heads;
};

#endif // MONITORFIXTUREITEM_H
