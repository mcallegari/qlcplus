/*
  Q Light Controller Plus
  channelmodifiergraphicsview.h

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

#ifndef CHANNELMODIFIERGRAPHICSVIEW_H
#define CHANNELMODIFIERGRAPHICSVIEW_H

#include <QGraphicsView>
#include <QGraphicsEllipseItem>
#include <QObject>

class HandlerGraphicsItem : public QObject, public QGraphicsEllipseItem
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)

public:
    HandlerGraphicsItem(qreal x, qreal y, qreal w, qreal h, const QPen & pen = QPen(),
                        const QBrush & brush = QBrush());

    void setBoundingBox(QRectF rect);
    QRectF boundingBox();

private:
    QRectF m_boundingBox;

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);

signals:
    void itemDropped(HandlerGraphicsItem *);
    void itemSelected(HandlerGraphicsItem *item);
    void itemMoved(HandlerGraphicsItem *item, QGraphicsSceneMouseEvent *event, QRectF limits);
};

typedef struct
{
    HandlerGraphicsItem *m_item;
    QGraphicsLineItem *m_line;
    QPoint m_pos;
    QPair <uchar, uchar> m_dmxMap;
} HandlerItem;

class ChannelModifierGraphicsView : public QGraphicsView
{
    Q_OBJECT
public:
    ChannelModifierGraphicsView(QWidget *parent);

    /** Set the DMX position map of the currently selected handler */
    void setHandlerDMXValue(uchar pos, uchar value);

    /** Add a new graphics handler half-way between the currently
     *  selected handler and the next. If no handler is selected,
     *  the new handler will be created after the first available handler */
    void addNewHandler();

    /** Remove the currently selected handler */
    void removeHander();

    void setModifierMap(QList< QPair<uchar, uchar> > map);

    QList< QPair<uchar, uchar> > modifiersMap();

private:
    QGraphicsScene *m_scene;
    QGraphicsRectItem *m_bgRect;
    QList<HandlerItem *> m_handlers;
    HandlerGraphicsItem *m_currentHandler;

protected:
    /** Get a pointer to the currently selected handler item */
    HandlerItem *getSelectedHandler();

    /** Update the position of a given handler after a resize event */
    HandlerGraphicsItem *updateHandlerItem(HandlerGraphicsItem *item, QPoint pos);

    /** Update the handler's bounding box at index $itemIndex to limit
     *  the drag and drop movements */
    void updateHandlerBoundingBox(int itemIndex);

    /** Calculate the XY position of a handler from its DMX position map */
    QPoint getPositionFromDMX(QPair<uchar, uchar> dmxMap);

    /** Calculate the DMX position map of a handler from a XY position */
    QPair<uchar, uchar> getDMXFromPosition(QPointF pos);

    /** Triggers the whole view repaint and lines computation */
    void updateView();

    /** Event caught when the GraphicsView is resized */
    void resizeEvent(QResizeEvent *event);

    void mouseReleaseEvent(QMouseEvent *e);

protected slots:
    void slotItemSelected(HandlerGraphicsItem *item);
    void slotItemMoved(HandlerGraphicsItem *item, QGraphicsSceneMouseEvent *event, QRectF limits);

signals:
    /** Signal emitted when the graphics view is clicked */
    void viewClicked(QMouseEvent * e);

    /** Signal emitted when a handler is clicked */
    void itemClicked(uchar pos, uchar value);

    /** Signal emitted when a handler is dragged and changes position */
    void itemDMXMapChanged(uchar pos, uchar value);
};

#endif // CHANNELMODIFIERGRAPHICSVIEW_H
