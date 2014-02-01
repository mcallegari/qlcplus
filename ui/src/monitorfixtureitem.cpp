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

    setToolTip(fxi->name());

    for (int i = 0; i < fxi->heads(); i++)
    {
        QGraphicsEllipseItem *head = new QGraphicsEllipseItem(this);
        head->setPen(QPen(Qt::white, 1));
        head->setBrush(QBrush(Qt::black));
        m_heads.append(head);
    }
}

void MonitorFixtureItem::setSize(QSize size)
{
    prepareGeometryChange();
    m_width = size.width();
    m_height = size.height();

    // calculate the diameter of every single head
    double headArea = (m_width * m_height) / m_heads.count();
    double headSide = sqrt(headArea);
    int columns = (m_width / headSide) + 0.5;
    int rows = (m_height / headSide) + 0.5;

    double cellWidth = m_width / columns;
    double cellHeight = m_height / rows;
    double headDiam = (cellWidth < cellHeight)?cellWidth:cellHeight;

    int ypos = (cellHeight - headDiam) / 2;
    for (int i = 0; i < rows; i++)
    {
        int xpos = (cellWidth - headDiam) / 2;
        for (int j = 0; j < columns; j++)
        {
            QGraphicsEllipseItem *head = m_heads.at((i * j) + j);
            head->setRect(xpos, ypos, headDiam, headDiam);
            xpos += cellWidth;
        }
        ypos += cellHeight;
    }
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
        painter->setPen(QPen(Qt::white, 1));
    else
        painter->setPen(QPen(Qt::gray, 1));

    // draw chaser background
    painter->setBrush(QBrush(QColor(33, 33, 33)));
    painter->drawRect(0, 0, m_width, m_height);
    foreach (QGraphicsEllipseItem *head, m_heads)
        head->update();
}

void MonitorFixtureItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsItem::mousePressEvent(event);
    this->setSelected(true);
}

void MonitorFixtureItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsItem::mouseReleaseEvent(event);
    qDebug() << Q_FUNC_INFO << "mouse RELEASE event - <" << event->pos().toPoint().x() << "> - <" << event->pos().toPoint().y() << ">";
    setCursor(Qt::OpenHandCursor);
    emit itemDropped(this);
}

void MonitorFixtureItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *)
{
}
