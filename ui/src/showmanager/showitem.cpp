/*
  Q Light Controller Plus
  showitem.cpp

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

#include <QGraphicsSceneEvent>
#include <QApplication>
#include <QPainter>
#include <QDebug>
#include <QMenu>

#include "trackitem.h"
#include "showitem.h"
#include "function.h"

ShowItem::ShowItem(QObject *)
    : m_color(100, 100, 100)
    , m_locked(false)
    , m_pressed(false)
    , m_width(50)
    , m_timeScale(3)
    , m_trackIdx(-1)
    , m_functionID(Function::invalidId())
    , m_alignToCursor(NULL)
    , m_lockAction(NULL)
{
    setCursor(Qt::OpenHandCursor);
    setFlag(QGraphicsItem::ItemIsSelectable, true);

    m_font = qApp->font();
    m_font.setBold(true);
    m_font.setPixelSize(12);

    m_alignToCursor = new QAction(tr("Align to cursor"), this);
    connect(m_alignToCursor, SIGNAL(triggered()),
            this, SLOT(slotAlignToCursorClicked()));
    m_lockAction = new QAction(tr("Lock item"), this);
    connect(m_lockAction, SIGNAL(triggered()),
            this, SLOT(slotLockItemClicked()));
}

void ShowItem::setTimeScale(int val)
{
    prepareGeometryChange();
    m_timeScale = val;
}

int ShowItem::getTimeScale()
{
    return m_timeScale;
}

void ShowItem::setWidth(int w)
{
    m_width = w;
}

int ShowItem::getWidth()
{
    return m_width;
}

QPointF ShowItem::getDraggingPos()
{
    return m_pos;
}

void ShowItem::setTrackIndex(int idx)
{
    m_trackIdx = idx;
}

int ShowItem::getTrackIndex()
{
    return m_trackIdx;
}

void ShowItem::setColor(QColor col)
{
    m_color = col;
    update();
}

QColor ShowItem::getColor()
{
    return m_color;
}

void ShowItem::setLocked(bool locked)
{
    m_locked = locked;
    setFlag(QGraphicsItem::ItemIsMovable, !locked);
}

bool ShowItem::isLocked()
{
    return m_locked;
}

void ShowItem::setFunctionID(quint32 id)
{
    m_functionID = id;
}

quint32 ShowItem::getFunctionID()
{
    return m_functionID;
}

void ShowItem::slotAlignToCursorClicked()
{
    emit alignToCursor(this);
}

void ShowItem::slotLockItemClicked()
{
    setLocked(!isLocked());
    //update();
}

void ShowItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsItem::mousePressEvent(event);
    m_pos = this->pos();
    if(event->button() == Qt::LeftButton)
        m_pressed = true;
    this->setSelected(true);
}

void ShowItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsItem::mouseReleaseEvent(event);
    qDebug() << Q_FUNC_INFO << "mouse RELEASE event - <" << event->pos().toPoint().x() << "> - <" << event->pos().toPoint().y() << ">";
    setCursor(Qt::OpenHandCursor);
    m_pressed = false;
    emit itemDropped(event, this);
}

void ShowItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    Q_UNUSED(event)

    QMenu menu;
    QFont menuFont = qApp->font();
    menuFont.setPixelSize(14);
    menu.setFont(menuFont);

    menu.addAction(m_alignToCursor);
    if (isLocked())
    {
        m_lockAction->setText(tr("Unlock item"));
        m_lockAction->setIcon(QIcon(":/unlock.png"));
    }
    else
    {
        m_lockAction->setText(tr("Lock item"));
        m_lockAction->setIcon(QIcon(":/lock.png"));
    }
    menu.addAction(m_lockAction);
    menu.exec(QCursor::pos());
}


QRectF ShowItem::boundingRect() const
{
    return QRectF(0, 0, m_width, TRACK_HEIGHT - 3);
}

void ShowItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    if (this->isSelected() == true)
        painter->setPen(QPen(Qt::white, 3));
    else
        painter->setPen(QPen(Qt::white, 1));

    // draw item background
    painter->setBrush(QBrush(m_color));
    painter->drawRect(0, 0, m_width, 77);
}


