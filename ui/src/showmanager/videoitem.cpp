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

#include <QDesktopWidget>
#include <QApplication>
#include <QPainter>
#include <QMenu>

#include "videoitem.h"
#include "trackitem.h"
#include "headeritems.h"

VideoItem::VideoItem(Video *vid, ShowFunction *func)
    : ShowItem(func)
    , m_video(vid)
    , m_fullscreenAction(NULL)
{
    Q_ASSERT(vid != NULL);

    if (func->color().isValid())
        setColor(func->color());
    else
        setColor(ShowFunction::defaultColor(Function::Video));

    if (func->duration() == 0)
        func->setDuration(m_video->totalDuration());

    calculateWidth();
    connect(m_video, SIGNAL(changed(quint32)),
            this, SLOT(slotVideoChanged(quint32)));
    connect(m_video, SIGNAL(totalTimeChanged(qint64)),
            this, SLOT(slotVideoDurationChanged(qint64)));

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
    qint64 video_duration = m_video->totalDuration();

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
    float timeScale = 50/(float)m_timeScale;

    ShowItem::paint(painter, option, widget);

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

    ShowItem::postPaint(painter);
}

void VideoItem::setTimeScale(int val)
{
    ShowItem::setTimeScale(val);
    calculateWidth();
}

void VideoItem::setDuration(quint32 msec, bool stretch)
{
    Q_UNUSED(msec)
    Q_UNUSED(stretch)
    // nothing to do
}

QString VideoItem::functionName()
{
    if (m_video)
        return m_video->name();
    return QString();
}

Video *VideoItem::getVideo()
{
    return m_video;
}

void VideoItem::slotVideoChanged(quint32)
{
    prepareGeometryChange();
    calculateWidth();
    if (m_function)
        m_function->setDuration(m_video->totalDuration());
}

void VideoItem::slotVideoDurationChanged(qint64)
{
    prepareGeometryChange();
    calculateWidth();
    if (m_function)
        m_function->setDuration(m_video->totalDuration());
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

    int screenCount = 0;
    QDesktopWidget *desktop = qApp->desktop();
    if (desktop != NULL)
        screenCount = desktop->screenCount();

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
    foreach(QAction *action, getDefaultActions())
        menu.addAction(action);

    menu.exec(QCursor::pos());
}
