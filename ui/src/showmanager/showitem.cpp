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

#include "headeritems.h"
#include "trackitem.h"
#include "showitem.h"
#include "function.h"

ShowItem::ShowItem(ShowFunction *function, QObject *)
    : m_color(100, 100, 100)
    , m_locked(false)
    , m_pressed(false)
    , m_width(50)
    , m_timeScale(3)
    , m_trackIdx(-1)
    , m_function(function)
    , m_alignToCursor(NULL)
    , m_lockAction(NULL)
{
    Q_ASSERT(function != NULL);

    setCursor(Qt::OpenHandCursor);
    setFlag(QGraphicsItem::ItemIsSelectable, true);

    m_font = qApp->font();
    m_font.setBold(true);
    m_font.setPixelSize(12);

    setLocked(m_function->isLocked());

    m_alignToCursor = new QAction(tr("Align to cursor"), this);
    connect(m_alignToCursor, SIGNAL(triggered()),
            this, SLOT(slotAlignToCursorClicked()));
    m_lockAction = new QAction(tr("Lock item"), this);
    connect(m_lockAction, SIGNAL(triggered()),
            this, SLOT(slotLockItemClicked()));
}

void ShowItem::updateTooltip()
{
    if (m_function == NULL)
        return;

    setToolTip(QString(tr("Name: %1\nStart time: %2\nDuration: %3\n%4"))
              .arg(functionName())
              .arg(Function::speedToString(m_function->startTime()))
              .arg(Function::speedToString(getDuration()))
              .arg(tr("Click to move this item along the timeline")));
}

QList<QAction *> ShowItem::getDefaultActions()
{
    QList<QAction *> actions;
    actions.append(m_alignToCursor);

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
    actions.append(m_lockAction);

    return actions;
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

void ShowItem::setStartTime(quint32 time)
{
    if (m_function == NULL)
        return;

    m_function->setStartTime(time);
    updateTooltip();
}

quint32 ShowItem::getStartTime()
{
    if (m_function)
        return m_function->startTime();
    return 0;
}

void ShowItem::setDuration(quint32 msec, bool stretch)
{
    Q_UNUSED(stretch)

    if (m_function == NULL)
        return;

    m_function->setDuration(msec);
    updateTooltip();
}

quint32 ShowItem::getDuration()
{
    if (m_function)
        return m_function->duration();
    return 0;
}

void ShowItem::setWidth(int w)
{
    m_width = w;
    updateTooltip();
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
    if (m_function)
        m_function->setColor(col);
    update();
}

QColor ShowItem::getColor()
{
    return m_color;
}

void ShowItem::setLocked(bool locked)
{
    m_locked = locked;
    if (m_function)
        m_function->setLocked(locked);
    setFlag(QGraphicsItem::ItemIsMovable, !locked);
    update();
}

bool ShowItem::isLocked()
{
    return m_locked;
}

void ShowItem::setFunctionID(quint32 id)
{
    if (m_function != NULL)
        m_function->setFunctionID(id);
}

quint32 ShowItem::functionID()
{
    if (m_function != NULL)
        return m_function->functionID();

    return Function::invalidId();
}

ShowFunction *ShowItem::showFunction() const
{
    return m_function;
}

QString ShowItem::functionName()
{
    return QString();
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
    if (event->button() == Qt::LeftButton)
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
    painter->drawRect(0, 0, m_width, TRACK_HEIGHT - 3);

    painter->setFont(m_font);
}

void ShowItem::postPaint(QPainter *painter)
{
    // draw the function name shadow
    painter->setPen(QPen(QColor(10, 10, 10, 150), 2));
    painter->drawText(QRect(4, 6, m_width - 6, 71), Qt::AlignLeft | Qt::TextWordWrap, functionName());

    // draw the function name
    painter->setPen(QPen(QColor(220, 220, 220, 255), 2));
    painter->drawText(QRect(3, 5, m_width - 5, 72), Qt::AlignLeft | Qt::TextWordWrap, functionName());

    if (m_locked)
        painter->drawPixmap(3, TRACK_HEIGHT >> 1, 24, 24, QIcon(":/lock.png").pixmap(24, 24));

    if (m_pressed)
    {
        quint32 s_time = 0;
        if (x() > TRACK_WIDTH)
            s_time = (double)(x() - TRACK_WIDTH - 2) * (m_timeScale * 500) /
                     (double)(HALF_SECOND_WIDTH);
        painter->drawText(3, TRACK_HEIGHT - 10, Function::speedToString(s_time));
    }
}


