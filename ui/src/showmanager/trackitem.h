/*
  Q Light Controller Plus
  trackitem.h

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

#ifndef TRACKITEM_H
#define TRACKITEM_H

#include <QGraphicsItem>
#include <QObject>
#include <QAction>
#include <QFont>

#include "track.h"

#define TRACK_HEIGHT        80
#define TRACK_WIDTH         150

/** @addtogroup ui_functions
 * @{
 */

/**
 *
 * Track class. Clickable item which reprensents a Show Manager track
 * and informs the view the track properties change
 *
 */

class TrackItem : public QObject, public QGraphicsItem
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)

public:
    TrackItem(Track *track, int number);

    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    /** Return pointer to the Track class associated to this item */
    Track *getTrack();

    /** Return the track number */
    int getTrackNumber();

    /** Set the track name */
    void setName(QString name);

    /** Enable/disable active state which higlight the left bar */
    void setActive(bool flag);

    /** Return if this track is active or not */
    bool isActive();

    /** Set mute and solo flags on/off */
    void setFlags(bool solo, bool mute);

    /** Return the mute state of the item */
    bool isMute();

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *);
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *);

protected slots:
    void slotTrackChanged(quint32 id);
    void slotMoveUpClicked();
    void slotMoveDownClicked();
    void slotChangeNameClicked();
    void slotDeleteTrackClicked();

signals:
    void itemClicked(TrackItem *);
    void itemDoubleClicked(TrackItem *);
    void itemSoloFlagChanged(TrackItem *, bool);
    void itemMuteFlagChanged(TrackItem *, bool);
    void itemMoveUpDown(Track *, int);
    void itemRequestDelete(Track *);

private:
    QString m_name;
    int m_number;
    QFont m_font;
    QFont m_btnFont;
    bool m_isActive;
    Track *m_track;
    QRectF *m_muteRegion;
    bool m_isMute;
    QRectF *m_soloRegion;
    bool m_isSolo;

    QAction *m_moveUp;
    QAction *m_moveDown;
    QAction *m_changeName;
    QAction *m_delete;
};

/** @} */

#endif
