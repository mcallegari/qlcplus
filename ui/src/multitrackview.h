/*
  Q Light Controller
  multitrackview.h

  Copyright (C) Massimo Callegari

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  Version 2 as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details. The license is
  in the file "COPYING".

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef MULTITRACKVIEW_H
#define MULTITRACKVIEW_H

#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QSlider>
#include <QWidget>

#include "sceneitems.h"
#include "chaser.h"
#include "track.h"

class MultiTrackView : public QGraphicsView
{
    Q_OBJECT
    
public:
    MultiTrackView(QWidget *parent = 0);
    
    /** Update tracks horizontal dividers when the view changes */
    void updateTracksDividers();

    /** Set the multitrack view size in pixels */
    void setViewSize(int width, int height);

    /** Auto calculation of view size based on items */
    void updateViewSize();

    /*********************************************************************
     * Contents
     *********************************************************************/

    /** Update the multitrack view with the scene elements */
    void resetView();

    /** Add a new track to the view */
    void addTrack(Track *track);

    /** Add a new sequence item to the given track */
    void addSequence(Chaser *chaser);

    /** Add a new audio item to the given track */
    void addAudio(Audio *audio);

    /** Delete the currently selected sequence */
    quint32 deleteSelectedFunction();

    /** Set the given track to active state */
    void activateTrack(Track *track);

    /** get the selected sequence item. If none, returns NULL */
    SequenceItem *getSelectedSequence();

    /** get the selected audio item. If none, returns NULL */
    AudioItem *getSelectedAudio();

private:
    /** Get the index of the currently selected track */
    int getActiveTrack();

    /*********************************************************************
     * Header
     *********************************************************************/
public:
    /** Set the type of header. Can be Time (seconds) or BPM,
     *  in various forms (4/4, 3/4) */
    void setHeaderType(SceneHeaderItem::TimeDivision type);

    SceneHeaderItem::TimeDivision getHeaderType();

    /** When BPM is selected, this function can set a precise
     *  value of time division */
    void setBPMValue(int value);

    void setSnapToGrid(bool enable);

    /*********************************************************************
     * Cursor
     *********************************************************************/
public:
    /** Move cursor to a given time */
    void moveCursor(quint32 timePos);

    /** Reset cursor to initial position */
    void rewindCursor();

    /** Get time in milliseconds of the current cursor position */
    quint32 getTimeFromCursor();

private:

    /** Return position in pixel of a given time (in msec) */
    quint32 getPositionFromTime(quint32 time);

    /** Return the time (in msec) from a given X position */
    quint32 getTimeFromPosition(qreal pos);

    void updateItem(SequenceItem *, quint32 time);
    void updateItem(AudioItem *, quint32 time);

private:
    QGraphicsScene *m_scene;
    QSlider *m_timeSlider;
    SceneHeaderItem *m_header;
    SceneCursorItem *m_cursor;
    QGraphicsItem * m_vdivider;
    QList <QGraphicsItem *> m_hdividers;
    QList <TrackItem *> m_tracks;
    QList <SequenceItem *> m_sequences;
    QList <AudioItem *> m_audio;
    bool m_snapToGrid;

signals:
    void sequenceMoved(SequenceItem *item);
    void audioMoved(AudioItem *item);
    void viewClicked(QMouseEvent * e);
    void timeChanged(quint32 msec);
    void trackClicked(Track *track);

public slots:
    void mouseReleaseEvent(QMouseEvent * e);

protected slots:
    void slotMoveCursor(QGraphicsSceneMouseEvent *event);
    void slotTimeScaleChanged(int val);
    void slotTrackClicked(TrackItem*);
    void slotTrackSoloFlagChanged(TrackItem*, bool);
    void slotTrackMuteFlagChanged(TrackItem*, bool);
    void slotViewScrolled(int);

    void slotSequenceMoved(QGraphicsSceneMouseEvent *, SequenceItem *);
    void slotSequenceMoved(QGraphicsSceneMouseEvent *, AudioItem *);
    void slotAlignToCursor(SequenceItem *);
    void slotAlignToCursor(AudioItem *);
};

#endif
