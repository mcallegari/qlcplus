/*
  Q Light Controller Plus
  trackitem.cpp

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
#include <QApplication>
#include <QPainter>
#include <QMenu>

#include "trackitem.h"

TrackItem::TrackItem(Track *track, int number)
    : m_number(number)
    , m_isActive(false)
    , m_track(track)
    , m_isMute(false)
    , m_isSolo(false)
{
    m_font = qApp->font();
    m_font.setBold(true);
    m_font.setPixelSize(12);

    m_btnFont = qApp->font();
    m_btnFont.setBold(true);
    m_btnFont.setPixelSize(12);

    if (track != NULL)
    {
        m_name = m_track->name();
        m_isMute = m_track->isMute();
        connect(m_track, SIGNAL(changed(quint32)), this, SLOT(slotTrackChanged(quint32)));
    }
    else
        m_name = QString("Track %1").arg(m_number + 1);

    m_soloRegion = new QRectF(17.0, 10.0, 25.0, 16.0);
    m_muteRegion = new QRectF(45.0, 10.0, 25.0, 16.0);

    m_moveUp = new QAction(QIcon(":/up.png"), tr("Move up"), this);
    connect(m_moveUp, SIGNAL(triggered()),
            this, SLOT(slotMoveUpClicked()));
    m_moveDown = new QAction(QIcon(":/down.png"), tr("Move down"), this);
    connect(m_moveDown, SIGNAL(triggered()),
            this, SLOT(slotMoveDownClicked()));

    m_changeName = new QAction(QIcon(":/editclear.png"), tr("Change name"), this);
    connect(m_changeName, SIGNAL(triggered()),
            this, SLOT(slotChangeNameClicked()));

    m_delete = new QAction(QIcon(":/editdelete.png"), tr("Delete"), this);
    connect(m_delete, SIGNAL(triggered()),
            this, SLOT(slotDeleteTrackClicked()));
}

Track *TrackItem::getTrack()
{
    return m_track;
}

int TrackItem::getTrackNumber()
{
    return m_number;
}

void TrackItem::setName(QString name)
{
    if (!name.isEmpty())
        m_name = name;
    update();
}

void TrackItem::setActive(bool flag)
{
    m_isActive = flag;
    update();
}

bool TrackItem::isActive()
{
    return m_isActive;
}

void TrackItem::setFlags(bool solo, bool mute)
{
    m_isSolo = solo;
    m_isMute = mute;
    update();
}

bool TrackItem::isMute()
{
    return m_isMute;
}

void TrackItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    m_isActive = true;
    QGraphicsItem::mousePressEvent(event);
    if (m_soloRegion->contains(event->pos().toPoint()))
    {
        m_isSolo = !m_isSolo;
        emit itemSoloFlagChanged(this, m_isSolo);
    }
    if (m_muteRegion->contains(event->pos().toPoint()))
    {
        m_isMute = !m_isMute;
        emit itemMuteFlagChanged(this, m_isMute);
    }
    emit itemClicked(this);
}

void TrackItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *)
{
    QMenu menu;
    QFont menuFont = qApp->font();
    menuFont.setPixelSize(14);
    menu.setFont(menuFont);

    if (m_number > 0)
        menu.addAction(m_moveUp);
    menu.addAction(m_moveDown);
    menu.addAction(m_changeName);
    menu.addAction(m_delete);
    menu.exec(QCursor::pos());
}

void TrackItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *)
{
    emit itemDoubleClicked(this);
}

QRectF TrackItem::boundingRect() const
{
    return QRectF(0, 0, TRACK_WIDTH - 4, TRACK_HEIGHT);
}

void TrackItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    // draw background gradient
    QLinearGradient linearGrad(QPointF(0, 0), QPointF(0, TRACK_HEIGHT));
    linearGrad.setColorAt(0, QColor(50, 64, 75, 255));
    //linearGrad.setColorAt(1, QColor(99, 127, 148, 255));
    linearGrad.setColorAt(1, QColor(76, 98, 115, 255));
    painter->setBrush(linearGrad);
    painter->drawRect(0, 0, TRACK_WIDTH - 4, TRACK_HEIGHT - 1);

    // Draw left bar that shows if the track is active or not
    painter->setPen(QPen(QColor(48, 61, 72, 255), 1));
    if (m_isActive == true)
        painter->setBrush(QBrush(QColor(0, 255, 0, 255)));
    else
        painter->setBrush(QBrush(QColor(129, 145, 160, 255)));
    painter->drawRoundedRect(1, 1, 10, 40, 2, 2);

    // draw solo button
    if (m_isSolo)
        painter->setBrush(QBrush(QColor(255, 255, 0, 255)));
    else
        painter->setBrush(QBrush(QColor(129, 145, 160, 255)));
    painter->drawRoundedRect(m_soloRegion->toRect(), 3.0, 3.0);
    painter->setFont(m_btnFont);
    painter->drawText(25, 23, "S");

    // draw mute button
    if (m_isMute)
        painter->setBrush(QBrush(QColor(255, 0, 0, 255)));
    else
        painter->setBrush(QBrush(QColor(129, 145, 160, 255)));
    painter->drawRoundedRect(m_muteRegion->toRect(), 3.0, 3.0);
    painter->drawText(51, 23, "M");

    // draw bound Scene indicator
    if (m_track->getSceneID() != Function::invalidId())
        painter->drawPixmap(TRACK_WIDTH - 33, 5, 24, 24, QIcon(":/scene.png").pixmap(24, 24));

    painter->setFont(m_font);
    // draw shadow
    painter->setPen(QPen(QColor(10, 10, 10, 150), 2));
    painter->drawText(QRect(5, 47, TRACK_WIDTH - 7, 28), Qt::AlignLeft | Qt::TextWordWrap | Qt::AlignBottom, m_name);
    // draw track name
    painter->setPen(QPen(QColor(200, 200, 200, 255), 2));
    painter->drawText(QRect(4, 47, TRACK_WIDTH - 7, 28), Qt::AlignLeft | Qt::TextWordWrap | Qt::AlignBottom, m_name);
}

void TrackItem::slotTrackChanged(quint32 id)
{
    Q_UNUSED(id);

    m_name = m_track->name();
    update();
}

void TrackItem::slotMoveUpClicked()
{
    emit itemMoveUpDown(m_track, -1);
}

void TrackItem::slotMoveDownClicked()
{
    emit itemMoveUpDown(m_track, 1);
}

void TrackItem::slotChangeNameClicked()
{
    emit itemDoubleClicked(this);
}

void TrackItem::slotDeleteTrackClicked()
{
    emit itemRequestDelete(m_track);
}
