/*
  Q Light Controller Plus
  monitorfixtureitem.cpp

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

#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QCursor>
#include <QDebug>

#include "monitorfixtureitem.h"
#include "fixture.h"
#include "doc.h"

MonitorFixtureItem::MonitorFixtureItem(Doc *doc, quint32 fid)
    : m_doc(doc)
    , m_fid(fid)
{
    Q_ASSERT(doc != NULL);

    setCursor(Qt::OpenHandCursor);
    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);

    Fixture *fxi = m_doc->fixture(fid);
    Q_ASSERT(fxi != NULL);
}

void MonitorFixtureItem::setSize(QSize size)
{
    prepareGeometryChange();
    m_width = size.width();
    m_height = size.height();
    update();
}

QRectF MonitorFixtureItem::boundingRect() const
{
    return QRectF(0, 0, m_width, m_height);
}

void MonitorFixtureItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    if (this->isSelected() == true)
        painter->setPen(QPen(Qt::white, 3));
    else
        painter->setPen(QPen(Qt::white, 1));

    // draw chaser background
    painter->setBrush(QBrush(QColor(33, 33, 33)));
    painter->drawRect(0, 0, m_width, m_height);
}

void MonitorFixtureItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsItem::mousePressEvent(event);
    m_pos = this->pos();
    this->setSelected(true);
}

void MonitorFixtureItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsItem::mouseReleaseEvent(event);
    qDebug() << Q_FUNC_INFO << "mouse RELEASE event - <" << event->pos().toPoint().x() << "> - <" << event->pos().toPoint().y() << ">";
    setCursor(Qt::OpenHandCursor);
    emit itemDropped(event, this);
}

void MonitorFixtureItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *)
{
}
