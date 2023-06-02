/*
  Q Light Controller Plus
  videoitem.h

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

#ifndef VIDEOITEM_H
#define VIDEOITEM_H

#include <QGraphicsItem>
#include <QObject>
#include <QAction>
#include <QFont>

#include "showitem.h"
#include "video.h"

/** @addtogroup ui_functions
 * @{
 */

/**
 *
 * Video Item. Clickable and draggable object identifying a Video object
 *
 */
class VideoItem : public ShowItem
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)

public:
    VideoItem(Video *vid, ShowFunction *func);

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    /** @reimp */
    void setTimeScale(int val);

    /** @reimp */
    void setDuration(quint32 msec, bool stretch);

    /** @reimp */
    QString functionName();

    /** Return a pointer to a Video Function associated to this item */
    Video *getVideo();

protected:
    /** @reimp */
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);

protected slots:
    void slotVideoChanged(quint32);
    void slotVideoDurationChanged(qint64);
    void slotScreenChanged();
    void slotFullscreenToggled(bool toggle);

private:
    /** Calculate sequence width for paint() and boundingRect() */
    void calculateWidth();

private:
    /** Reference to the actual Video Function */
    Video *m_video;

    /** Context menu actions */
    QAction *m_fullscreenAction;
};

/** @} */

#endif
