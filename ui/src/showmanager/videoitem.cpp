/*
  Q Light Controller Plus
  videoitem.cpp

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

#include <QApplication>
#include <QPainter>
#include <QMenu>

#include "videoitem.h"
#include "trackitem.h"
#include "headeritems.h"

VideoItem::VideoItem(Video *vid)
    : ShowItem()
    , m_video(vid)
    , m_fullscreenAction(NULL)
{
    Q_ASSERT(vid != NULL);

    setStartTime(m_video->getStartTime());
    setColor(m_video->getColor());
    setLocked(m_video->isLocked());
    setFunctionID(m_video->id());

    calculateWidth();
    connect(m_video, SIGNAL(changed(quint32)), this, SLOT(slotVideoChanged(quint32)));

    m_fullscreenAction = new QAction(tr("Fullscreen"), this);
    m_fullscreenAction->setCheckable(true);
    if (m_video->fullscreen() == true)
        m_fullscreenAction->setChecked(true);
    connect(m_fullscreenAction, SIGNAL(toggled(bool)),
            this, SLOT(slotFullscreenToggled(bool)));
}

void VideoItem::calculateWidth()
{
    int newWidth = 0;
    qint64 video_duration = m_video->getDuration();

    if (video_duration != 0)
        newWidth = ((50/(float)getTimeScale()) * (float)video_duration) / 1000;
    else
        newWidth = 100;

    if (newWidth < (50 / m_timeScale))
        newWidth = 50 / m_timeScale;
    setWidth(newWidth);
}

void VideoItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    float timeScale = 50/(float)m_timeScale;

    if (this->isSelected() == true)
        painter->setPen(QPen(Qt::white, 3));
    else
        painter->setPen(QPen(Qt::white, 1));
    painter->setBrush(QBrush(m_color));

    painter->drawRect(0, 0, m_width, TRACK_HEIGHT - 3);

    painter->setFont(m_font);

    if (m_video->fadeInSpeed() != 0)
    {
        int fadeXpos = (timeScale * (float)m_video->fadeInSpeed()) / 1000;
        painter->setPen(QPen(Qt::gray, 1));
        painter->drawLine(1, TRACK_HEIGHT - 4, fadeXpos, 2);
    }

    if (m_video->fadeOutSpeed() != 0)
    {
        int fadeXpos = (timeScale * (float)m_video->fadeOutSpeed()) / 1000;
        painter->setPen(QPen(Qt::gray, 1));
        painter->drawLine(m_width - fadeXpos, 2, m_width - 1, TRACK_HEIGHT - 4);
    }

    // draw shadow
    painter->setPen(QPen(QColor(10, 10, 10, 150), 2));
    painter->drawText(QRect(4, 6, m_width - 6, 71), Qt::AlignLeft | Qt::TextWordWrap, m_video->name());

    // draw video name
    painter->setPen(QPen(QColor(220, 220, 220, 255), 2));
    painter->drawText(QRect(3, 5, m_width - 5, 72), Qt::AlignLeft | Qt::TextWordWrap, m_video->name());

    if (m_pressed)
    {
        quint32 s_time = (double)(x() - TRACK_WIDTH - 2) * (m_timeScale * 500) /
                         (double)(HALF_SECOND_WIDTH);
        painter->drawText(3, TRACK_HEIGHT - 10, Function::speedToString(s_time));
    }

    if (m_locked)
        painter->drawPixmap(3, TRACK_HEIGHT >> 1, 24, 24, QIcon(":/lock.png").pixmap(24, 24));
}

void VideoItem::updateDuration()
{
    setStartTime(m_video->getStartTime());
    prepareGeometryChange();
    calculateWidth();
}

void VideoItem::setTimeScale(int val)
{
    ShowItem::setTimeScale(val);
    calculateWidth();
}

void VideoItem::setStartTime(quint32 time)
{
    if (m_video == NULL)
        return;

    m_video->setStartTime(time);
    setToolTip(QString(tr("Name: %1\nStart time: %2\nDuration: %3\n%4"))
              .arg(m_video->name())
              .arg(Function::speedToString(m_video->getStartTime()))
              .arg(Function::speedToString(m_video->getDuration()))
              .arg(tr("Click to move this video across the timeline")));
}

quint32 VideoItem::getStartTime()
{
    if (m_video)
        return m_video->getStartTime();
    return 0;
}

void VideoItem::setLocked(bool locked)
{
    ShowItem::setLocked(locked);
    m_video->setLocked(locked);
}

Video *VideoItem::getVideo()
{
    return m_video;
}

void VideoItem::slotVideoChanged(quint32)
{
    prepareGeometryChange();
    calculateWidth();
}

void VideoItem::slotScreenChanged()
{
    QAction *action = (QAction *)sender();
    int scrIdx = action->data().toInt();

    m_video->setScreen(scrIdx);
}

void VideoItem::slotFullscreenToggled(bool toggle)
{
    m_video->setFullscreen(toggle);
}

void VideoItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *)
{
    QMenu menu;
    QFont menuFont = qApp->font();
    menuFont.setPixelSize(14);
    menu.setFont(menuFont);

    int screenCount = m_video->getScreenCount();
    if (screenCount > 0)
    {
        for (int i = 0; i < screenCount; i++)
        {
            QAction *scrAction = new QAction(tr("Screen %1").arg(i + 1), this);
            scrAction->setCheckable(true);
            if (m_video->screen() == i)
                scrAction->setChecked(true);
            scrAction->setData(i);
            connect(scrAction, SIGNAL(triggered()),
                    this, SLOT(slotScreenChanged()));
            menu.addAction(scrAction);
        }
    }
    menu.addAction(m_fullscreenAction);
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
