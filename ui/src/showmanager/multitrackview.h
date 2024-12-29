/*
  Q Light Controller Plus
  multitrackview.h

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

#ifndef MULTITRACKVIEW_H
#define MULTITRACKVIEW_H

#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QSlider>
#include <QWidget>

#include "rgbmatrixitem.h"
#include "sequenceitem.h"
#include "headeritems.h"
#include "trackitem.h"
#include "audioitem.h"
#include "efxitem.h"
#include "videoitem.h"
#include "chaser.h"
#include "track.h"

/** @addtogroup ui_shows
 * @{
 */

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
    void addSequence(Chaser *chaser, Track *track = NULL, ShowFunction *sf = NULL);

    /** Add a new audio item to the given track */
    void addAudio(Audio *audio, Track *track = NULL, ShowFunction *sf = NULL);

    /** Add a new RGB Matrix item to the given track */
    void addRGBMatrix(RGBMatrix *rgbm, Track *track = NULL, ShowFunction *sf = NULL);

    /** Add a new EFX item to the given track */
    void addEFX(EFX *efx, Track *track = NULL, ShowFunction *sf = NULL);

    /** Add a new video item to the given track */
    void addVideo(Video *video, Track *track = NULL, ShowFunction *sf = NULL);

    /** Delete the currently selected item */
    quint32 deleteSelectedItem();

    /** Delete a specific ShowFuntion and related ShowItem from the
     *  given track */
    void deleteShowItem(Track *track, ShowFunction *sf);

    /** Set the given track to active state */
    void activateTrack(Track *track);

    /** get the selected Show item. If none, returns NULL */
    ShowItem *getSelectedItem();

private:
    /** Retrieve the index of the given Track.
     *  If trk is NULL, this function returns the currently
     *  selected track.
     */
    int getTrackIndex(Track *trk);

    void setItemCommonProperties(ShowItem *item, ShowFunction *func, int trackNum);

    /*********************************************************************
     * Header
     *********************************************************************/
public:
    /** Set the type of header. Can be Time (seconds) or BPM,
     *  in various forms (4/4, 3/4) */
    void setHeaderType(Show::TimeDivision type);

    Show::TimeDivision getHeaderType();

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

    /** Return position in pixel of a given time (in msec) */
    quint32 getPositionFromTime(quint32 time);

    /** Return the time (in msec) from a given X position */
    quint32 getTimeFromPosition(qreal pos);

private:
    QGraphicsScene *m_scene;
    QSlider *m_timeSlider;
    ShowHeaderItem *m_header;
    ShowCursorItem *m_cursor;
    QGraphicsItem * m_vdivider;
    QList <QGraphicsItem *> m_hdividers;
    QList <TrackItem *> m_tracks;
    QList <ShowItem *>m_items;
    bool m_snapToGrid;

public slots:
    void mouseReleaseEvent(QMouseEvent *e);
    void wheelEvent(QWheelEvent *event);

protected slots:
    void slotHeaderClicked(QGraphicsSceneMouseEvent *event);
    void slotTimeScaleChanged(int val);
    void slotTrackClicked(TrackItem *track);
    void slotTrackDoubleClicked(TrackItem *track);
    void slotTrackSoloFlagChanged(TrackItem*, bool);
    void slotTrackMuteFlagChanged(TrackItem*, bool);
    void slotViewScrolled(int);

    void slotItemMoved(QGraphicsSceneMouseEvent *event, ShowItem *item);
    void slotAlignToCursor(ShowItem *item);

signals:
    void showItemMoved(ShowItem *item, quint32 time, bool moved);
    void viewClicked(QMouseEvent * e);
    void timeChanged(quint32 msec);
    void trackClicked(Track *track);
    void trackDoubleClicked(Track *track);
    void trackMoved(Track *, int);
    void trackDelete(Track *);
};

/** @} */

#endif
