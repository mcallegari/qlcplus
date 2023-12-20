/*
  Q Light Controller Plus
  channelmodifiergraphicsview.cpp

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

#include <QGraphicsSceneMouseEvent>
#include <QGraphicsRectItem>
#include <QDebug>

#include "channelmodifiergraphicsview.h"

ChannelModifierGraphicsView::ChannelModifierGraphicsView(QWidget *parent)
    : QGraphicsView(parent)
    , m_currentHandler(NULL)
{
    m_scene = new QGraphicsScene(this);
    //m_scene->setSceneRect(this->rect());
    setScene(m_scene);

    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    m_bgRect = m_scene->addRect(0, 0, m_scene->width(), m_scene->height(),
                                QPen(Qt::NoPen), QBrush(QColor(70, 70, 70, 255), Qt::SolidPattern));
    m_bgRect->setZValue(0);
}

void ChannelModifierGraphicsView::setHandlerDMXValue(uchar pos, uchar value)
{
    //qDebug() << "Set new DMX value:" << pos << value;
    if (m_currentHandler == NULL)
        return;

    HandlerItem *handler = getSelectedHandler();
    if (handler != NULL)
    {
        handler->m_dmxMap.first = pos;
        handler->m_dmxMap.second = value;
        updateView();
    }
}

void ChannelModifierGraphicsView::addNewHandler()
{
    // always give the fact that there are at least two handlers !
    HandlerItem *prevHandler = getSelectedHandler();
    if (prevHandler == NULL)
        prevHandler = m_handlers.at(0);
    if (prevHandler == m_handlers.last())
        prevHandler = m_handlers.at(m_handlers.count() - 2);

    int prevHdlrIdx = 0;
    HandlerItem *nextHandler = NULL;
    for (prevHdlrIdx = 0; prevHdlrIdx < m_handlers.count(); prevHdlrIdx++)
    {
        HandlerItem *hdlr = m_handlers.at(prevHdlrIdx);
        if (hdlr == prevHandler)
        {
            nextHandler = m_handlers.at(prevHdlrIdx + 1);
            break;
        }
    }

    HandlerItem *newHandler = new HandlerItem;
    uchar halfDMXpos = prevHandler->m_dmxMap.first + ((nextHandler->m_dmxMap.first - prevHandler->m_dmxMap.first) / 2);
    uchar halfDMXval = prevHandler->m_dmxMap.second + ((nextHandler->m_dmxMap.second - prevHandler->m_dmxMap.second) / 2);
    newHandler->m_dmxMap = QPair<uchar, uchar>(halfDMXpos, halfDMXval);
    //qDebug() << "Half way DMX value:" << halfDMXpos << halfDMXval;
    newHandler->m_pos = getPositionFromDMX(newHandler->m_dmxMap);
    newHandler->m_item = updateHandlerItem(NULL, newHandler->m_pos);
    newHandler->m_item->setBoundingBox(QRectF(prevHandler->m_pos.x(), m_bgRect->y(),
                                           nextHandler->m_pos.x() - prevHandler->m_pos.x(),
                                           m_bgRect->rect().height()));
    newHandler->m_line = m_scene->addLine(newHandler->m_pos.x(), newHandler->m_pos.y(),
                                        prevHandler->m_pos.x(), prevHandler->m_pos.y(),
                                        QPen(Qt::yellow));
    m_scene->removeItem(nextHandler->m_line);
    nextHandler->m_line = m_scene->addLine(nextHandler->m_pos.x(), nextHandler->m_pos.y(),
                                           newHandler->m_pos.x(), newHandler->m_pos.y(),
                                           QPen(Qt::yellow));
    m_handlers.insert(prevHdlrIdx + 1, newHandler);
    updateView();
}

void ChannelModifierGraphicsView::removeHander()
{
    if (m_currentHandler == NULL)
        return;

    for (int i = 0; i < m_handlers.count(); i++)
    {
        HandlerItem *handler = m_handlers.at(i);
        if (handler->m_item == m_currentHandler)
        {
            m_currentHandler = NULL;
            m_scene->removeItem(handler->m_item);
            m_scene->removeItem(handler->m_line);
            m_handlers.takeAt(i);
            break;
        }
    }
    updateView();
    emit viewClicked(NULL);
}

void ChannelModifierGraphicsView::setModifierMap(QList<QPair<uchar, uchar> > map)
{
    m_scene->clear();
    m_handlers.clear();
    m_currentHandler = NULL;

    m_bgRect = m_scene->addRect(0, 0, m_scene->width(), m_scene->height(),
                                QPen(Qt::NoPen), QBrush(QColor(70, 70, 70, 255), Qt::SolidPattern));
    m_bgRect->setZValue(0);

    for (int i = 0; i < map.count(); i++)
    {
        QPair<uchar, uchar> dmxPair = map.at(i);
        HandlerItem *handler = new HandlerItem;
        handler->m_dmxMap = QPair<uchar, uchar>(dmxPair.first, dmxPair.second);
        handler->m_pos = getPositionFromDMX(dmxPair);
        handler->m_item = updateHandlerItem(NULL, handler->m_pos);
        if (i == 0)
            handler->m_line = NULL;
        else
            handler->m_line = m_scene->addLine(0,0,1,1,QPen(Qt::yellow));
        m_handlers.append(handler);
    }
    for (int i = 0; i < map.count(); i++)
        updateHandlerBoundingBox(i);
    updateView();
}

QList< QPair<uchar, uchar> > ChannelModifierGraphicsView::modifiersMap()
{
    QList< QPair<uchar, uchar> > modMap;
    foreach (HandlerItem *item, m_handlers)
        modMap.append(item->m_dmxMap);
    return modMap;
}

HandlerItem *ChannelModifierGraphicsView::getSelectedHandler()
{
    foreach (HandlerItem *handler, m_handlers)
        if (handler->m_item->isSelected())
            return handler;
    return NULL;
}

HandlerGraphicsItem *ChannelModifierGraphicsView::updateHandlerItem(HandlerGraphicsItem *item, QPoint pos)
{
    HandlerGraphicsItem *tmpItem = item;
    if (tmpItem == NULL)
    {
        tmpItem =  new HandlerGraphicsItem(pos.x() - 5, pos.y() - 5, 10, 10,
                                           QPen(Qt::yellow), QBrush(Qt::yellow));
        tmpItem->setZValue(1);
        tmpItem->setParent(m_scene);
        connect(tmpItem, SIGNAL(itemMoved(HandlerGraphicsItem *, QGraphicsSceneMouseEvent*,QRectF)),
                this, SLOT(slotItemMoved(HandlerGraphicsItem *,QGraphicsSceneMouseEvent*,QRectF)));
        connect(tmpItem, SIGNAL(itemSelected(HandlerGraphicsItem*)),
                this, SLOT(slotItemSelected(HandlerGraphicsItem*)));
        m_scene->addItem(tmpItem);
    }
    else
    {
        tmpItem->setRect(pos.x() - 5, pos.y() - 5, 10, 10);
    }
    return tmpItem;
}

void ChannelModifierGraphicsView::updateHandlerBoundingBox(int itemIndex)
{
    if (itemIndex < 0 || itemIndex >= m_handlers.count())
        return;

    HandlerItem *handler = m_handlers.at(itemIndex);
    if (itemIndex == 0)
    {
        handler->m_item->setBoundingBox(QRect(m_bgRect->x() - 1, m_bgRect->y(), 1, m_bgRect->rect().height()));
        return;
    }
    else if (itemIndex == m_handlers.count() - 1)
    {
        handler->m_item->setBoundingBox(QRect(m_bgRect->rect().right(), m_bgRect->y(), 1, m_bgRect->rect().height()));
        return;
    }
    else
    {
        HandlerItem *prevHandler = m_handlers.at(itemIndex - 1);
        HandlerItem *nextHandler = m_handlers.at(itemIndex + 1);
        handler->m_item->setBoundingBox(QRectF(prevHandler->m_pos.x(), m_bgRect->y(),
                                             nextHandler->m_pos.x() - prevHandler->m_pos.x(),
                                             m_bgRect->rect().height()));
    }
}

QPoint ChannelModifierGraphicsView::getPositionFromDMX(QPair<uchar, uchar>dmxMap)
{
    qreal xPos = m_bgRect->rect().x() + ((m_bgRect->rect().width() / 255) * (qreal)dmxMap.first);
    qreal yPos = m_bgRect->rect().y() + m_bgRect->rect().height() - ((m_bgRect->rect().height() / 255) * (qreal)dmxMap.second);
    //qDebug() << "New position from values <" << dmxMap.first << "," << dmxMap.second << "=" << xPos << yPos;
    return QPoint(xPos, yPos);
}

QPair<uchar, uchar> ChannelModifierGraphicsView::getDMXFromPosition(QPointF pos)
{
    if (pos.x() < m_bgRect->x()) pos.setX(m_bgRect->x());
    if (pos.y() < m_bgRect->y()) pos.setY(m_bgRect->y());
    QPair<uchar, uchar> newPos;
    newPos.first = ((pos.x() - m_bgRect->x()) * 255) / m_bgRect->rect().width();
    newPos.second = 255 - (((pos.y() - m_bgRect->y()) * 255) / m_bgRect->rect().height());
    //qDebug() << "Elapsed DMX value:" << newPos.first << newPos.second;
    return newPos;
}

void ChannelModifierGraphicsView::updateView()
{
    qDebug() << "Size after resize:" << width() << height();

    //m_scene->setSceneRect(0, 0, width(), height());
    int squareSize = width() - 20;
    if (height() < width())
        squareSize = height() - 20;

    QRect bgRect(5, 5, squareSize, squareSize);
    m_bgRect->setRect(bgRect);

    if (m_handlers.isEmpty())
    {
        // create one bottom-left handler and one top-right handler
        HandlerItem *blHdlr = new HandlerItem;
        blHdlr->m_dmxMap = QPair<uchar, uchar>(0, 0);
        blHdlr->m_pos = QPoint(bgRect.x(), bgRect.bottom());
        blHdlr->m_item = updateHandlerItem(NULL, blHdlr->m_pos);
        blHdlr->m_line = NULL;
        m_handlers.append(blHdlr);

        HandlerItem *trHdlr = new HandlerItem;
        trHdlr->m_dmxMap = QPair<uchar, uchar>(255, 255);
        trHdlr->m_pos = QPoint(bgRect.right(), bgRect.y());
        trHdlr->m_item = updateHandlerItem(NULL, trHdlr->m_pos);
        trHdlr->m_line = m_scene->addLine(blHdlr->m_pos.x(), blHdlr->m_pos.y(),
                                         trHdlr->m_pos.x(), trHdlr->m_pos.y(),
                                         QPen(Qt::yellow));
        m_handlers.append(trHdlr);
        updateHandlerBoundingBox(0);
        updateHandlerBoundingBox(1);
    }
    else
    {
        QPoint lastPos;
        for (int i = 0; i < m_handlers.count(); i++)
        {
            HandlerItem *handler = m_handlers.at(i);
            handler->m_pos = getPositionFromDMX(handler->m_dmxMap);
            handler->m_item = updateHandlerItem(handler->m_item, handler->m_pos);

            if (handler->m_line != NULL)
            {
                handler->m_line->setLine(lastPos.x(), lastPos.y(),
                                         handler->m_pos.x(), handler->m_pos.y());
            }
            updateHandlerBoundingBox(i);
            lastPos = handler->m_pos;
        }
    }
}

void ChannelModifierGraphicsView::resizeEvent(QResizeEvent *event)
{
    QGraphicsView::resizeEvent(event);
    updateView();
}

void ChannelModifierGraphicsView::mouseReleaseEvent(QMouseEvent *e)
{
    if (getSelectedHandler() == NULL)
    {
        if (m_currentHandler != NULL)
            m_currentHandler->setBrush(QBrush(Qt::yellow));
        m_currentHandler = NULL;
        emit viewClicked(e);

        QGraphicsView::mouseReleaseEvent(e);
    }
}

void ChannelModifierGraphicsView::slotItemSelected(HandlerGraphicsItem *item)
{
    if (m_currentHandler != NULL)
        m_currentHandler->setBrush(QBrush(Qt::yellow));
    m_currentHandler = item;
    HandlerItem *handler = getSelectedHandler();
    if (handler != NULL)
        emit itemClicked(handler->m_dmxMap.first, handler->m_dmxMap.second);
}

void ChannelModifierGraphicsView::slotItemMoved(HandlerGraphicsItem *item,
                                                QGraphicsSceneMouseEvent *event, QRectF limits)
{
    QPointF newPos(item->x(), item->y());

    if (!limits.contains(event->scenePos()))
    {
        qreal evX = event->scenePos().x();
        qreal evY = event->scenePos().y();

        if (evX < limits.x())
            newPos.setX(limits.left() - 5);
        else if (evX > limits.right())
            newPos.setX(limits.right() - 5);
        else
            newPos.setX(evX - 5);

        if (evY < limits.y())
            newPos.setY(limits.y() - 5);
        else if (evY > limits.bottom())
            newPos.setY(limits.bottom() - 5);
        else
            newPos.setY(evY - 5);
    }
    else
    {
        newPos.setX(event->scenePos().x() - 5);
        newPos.setY(event->scenePos().y() - 5);
    }
    HandlerItem *handler = getSelectedHandler();
    if (handler != NULL)
    {
        handler->m_dmxMap = getDMXFromPosition(newPos);
        emit itemDMXMapChanged(handler->m_dmxMap.first, handler->m_dmxMap.second);
    }
    updateView();
}

/********************************************************************
 * HandlerGraphicsItem class implementation
 ********************************************************************/

HandlerGraphicsItem::HandlerGraphicsItem(qreal x, qreal y, qreal w, qreal h,
                                         const QPen & pen, const QBrush &brush)
    : QGraphicsEllipseItem(x, y, w, h)
{
    setCursor(Qt::OpenHandCursor);
    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);

    setPen(pen);
    setBrush(brush);
}

void HandlerGraphicsItem::setBoundingBox(QRectF rect)
{
    m_boundingBox = rect;
    //qDebug() << Q_FUNC_INFO << rect;
}

QRectF HandlerGraphicsItem::boundingBox()
{
    return m_boundingBox;
}

void HandlerGraphicsItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsItem::mousePressEvent(event);
    setSelected(true);
    setBrush(QBrush(Qt::green));
    emit itemSelected(this);
}

void HandlerGraphicsItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsItem::mouseReleaseEvent(event);
    //qDebug() << Q_FUNC_INFO << "mouse RELEASE event - <" << event->pos().toPoint().x() << "> - <" << event->pos().toPoint().y() << ">";
    setCursor(Qt::OpenHandCursor);
    emit itemDropped(this);
}

void HandlerGraphicsItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    //qDebug() << "Mouse move event" << event->scenePos();
    //qDebug() << "x:" << x() << "y:" << y();
    emit itemMoved(this, event, m_boundingBox);
}


