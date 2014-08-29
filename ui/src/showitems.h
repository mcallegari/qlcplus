/*
  Q Light Controller Plus
  showitems.h

  Copyright (C) Heikki Junnila

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


#ifndef SCENEITEMS_H
#define SCENEITEMS_H

#include <QGraphicsItem>
#include <QObject>
#include <QAction>
#include <QFont>

#include "chaser.h"
#include "audio.h"
#include "track.h"
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
#include "video.h"
#endif

/** @addtogroup ui_functions
 * @{
 */

#define HEADER_HEIGHT       35
#define TRACK_HEIGHT        80
#define TRACK_WIDTH         150
#define HALF_SECOND_WIDTH   25

/*********************************************************************
 *
 * Scene Header class. Clickable time line header
 *
 *********************************************************************/

class SceneHeaderItem :  public QObject, public QGraphicsItem
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)

public:
    SceneHeaderItem(int);

    enum TimeDivision
    {
        Time = 0,
        BPM_4_4,
        BPM_3_4,
        BPM_2_4,
        Invalid
    };

    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    void setTimeScale(int val);
    int getTimeScale();

    void setTimeDivisionType(TimeDivision type);
    TimeDivision getTimeDivisionType();
    void setBPMValue(int value);

    int getHalfSecondWidth();
    float getTimeDivisionStep();

    void setWidth(int);
    void setHeight(int);

    static QString tempoToString(TimeDivision type);
    static TimeDivision stringToTempo(QString tempo);

signals:
    void itemClicked(QGraphicsSceneMouseEvent *);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);

private:
    /** Total width of the item */
    int m_width;
    /** Total height of the item */
    int m_height;
    /** Distance in pixels between the time division bars */
    float m_timeStep;
    /** Divisor of the time division hit bar (the highest bar) */
    char m_timeHit;
    /** Scale of the time division */
    int m_timeScale;
    /** When BPM mode is active, this holds the number of BPM to display */
    int m_BPMValue;
    /** The type of time division */
    TimeDivision m_type;
};

/***************************************************************************
 *
 * Scene Cursor class. Cursor which marks the time position in a scene
 *
 ***************************************************************************/
class SceneCursorItem : public QGraphicsItem
{
public:
    SceneCursorItem(int h);

    void setHeight(int height);

    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    void setTime(quint32 t);
    quint32 getTime();
private:
    int m_height;
    quint32 m_time;
};

/****************************************************************************
 *
 * Track class. Clickable item which informs the view the track properties change
 *
 ****************************************************************************/
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

/***************************************************************************************
 *
 * Sequence Item. Clickable and draggable object identifying a chaser in sequence mode
 *
 ***************************************************************************************/
class SequenceItem : public QObject, public QGraphicsItem
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)

public:
    SequenceItem(Chaser *seq);

    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    void setTimeScale(int val);
    int getWidth();

    QPointF getDraggingPos();

    void setTrackIndex(int idx);
    int getTrackIndex();

    void setColor(QColor col);
    QColor getColor();

    void setLocked(bool locked);
    bool isLocked();

    void setSelectedStep(int idx);

    /** Return a pointer to a Chaser associated to this item */
    Chaser *getChaser();

signals:
    void itemDropped(QGraphicsSceneMouseEvent *, SequenceItem *);
    void alignToCursor(SequenceItem *);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *);

protected slots:
    void slotSequenceChanged(quint32);
    void slotAlignToCursorClicked();
    void slotLockItemClicked();

private:
    /** Calculate sequence width for paint() and boundingRect() */
    void calculateWidth();

private:
    QColor m_color;
    /** Reference to the actual Chaser object which holds the sequence steps */
    Chaser *m_chaser;
    /** width of the graphics object. Recalculated every time a chaser step  changes */
    int m_width;
    /** Position of the item top-left corner. This is used to handle unwanted dragging */
    QPointF m_pos;
    /** horizontal scale to adapt width to the current time line */
    int m_timeScale;
    /** track index this sequence belongs to */
    int m_trackIdx;
    /** index of the selected step for highlighting (-1 if none) */
    int m_selectedStep;
    /** Locked state of the item */
    bool m_locked;

    QFont m_font;
    bool m_pressed;

    QAction *m_alignToCursor;
    QAction *m_lockAction;
};

/**************************************************************************
 *
 * Audio Item. Clickable and draggable object identifying an Audio object
 *
 **************************************************************************/
class AudioItem : public QObject, public QGraphicsItem
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)

public:
    AudioItem(Audio *aud);

    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    void setTimeScale(int val);
    int getWidth();

    QPointF getDraggingPos();

    void setTrackIndex(int idx);
    int getTrackIndex();

    void setColor(QColor col);
    QColor getColor();

    void setLocked(bool locked);
    bool isLocked();

    /** Return a pointer to a Audio object associated to this item */
    Audio *getAudio();

public slots:
    void updateDuration();

signals:
    void itemDropped(QGraphicsSceneMouseEvent *, AudioItem *);
    void alignToCursor(AudioItem *);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);

protected slots:
    void slotAudioChanged(quint32);

    void slotAudioPreviewLeft(bool active);
    void slotAudioPreviewRight(bool active);
    void slotAudioPreviewStereo(bool active);
    void slotAlignToCursorClicked();
    void slotLockItemClicked();

private:
    /** Calculate sequence width for paint() and boundingRect() */
    void calculateWidth();
    /** Retrieve a sample value from an audio buffer, given the sample size */
    qint32 getSample(unsigned char *data, quint32 *idx, int sampleSize);
    /** Routine that decode the whole and create the waveform QPixmap */
    void createWaveform(bool left, bool right);

private:
    QFont m_font;
    QColor m_color;
    /** Locked state of the item */
    bool m_locked;
    /** Reference to the actual Audio object */
    Audio *m_audio;
    /** width of the graphics object */
    int m_width;
    /** Position of the item top-left corner. This is used to handle unwanted dragging */
    QPointF m_pos;
    /** horizontal scale to adapt width to the current time line */
    int m_timeScale;
    /** track index this Audio object belongs to */
    int m_trackIdx;

    /** Context menu actions */
    QAction *m_previewLeftAction;
    QAction *m_previewRightAction;
    QAction *m_previewStereoAction;
    QAction *m_alignToCursor;
    QAction *m_lockAction;

    /** Pixmap holding the waveform (if enabled) */
    QPixmap *m_preview;

    bool m_pressed;
};


#if QT_VERSION >= 0x050000
/**************************************************************************
 *
 * Video Item. Clickable and draggable object identifying a Video object
 *
 **************************************************************************/
class VideoItem : public QObject, public QGraphicsItem
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)

public:
    VideoItem(Video *vid);

    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    void setTimeScale(int val);
    int getWidth();

    QPointF getDraggingPos();

    void setTrackIndex(int idx);
    int getTrackIndex();

    void setColor(QColor col);
    QColor getColor();

    void setLocked(bool locked);
    bool isLocked();

    /** Return a pointer to a Video object associated to this item */
    Video *getVideo();

public slots:
    void updateDuration();

signals:
    void itemDropped(QGraphicsSceneMouseEvent *, VideoItem *);
    void alignToCursor(VideoItem *);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);

protected slots:
    void slotVideoChanged(quint32);
    void slotAlignToCursorClicked();
    void slotLockItemClicked();
    void slotScreenChanged();
    void slotFullscreenToggled(bool toggle);

private:
    /** Calculate sequence width for paint() and boundingRect() */
    void calculateWidth();

private:
    QFont m_font;
    QColor m_color;
    /** Locked state of the item */
    bool m_locked;
    /** Reference to the actual Video object */
    Video *m_video;
    /** width of the graphics object */
    int m_width;
    /** Position of the item top-left corner. This is used to handle unwanted dragging */
    QPointF m_pos;
    /** horizontal scale to adapt width to the current time line */
    int m_timeScale;
    /** track index this Video object belongs to */
    int m_trackIdx;

    /** Context menu actions */
    QAction *m_alignToCursor;
    QAction *m_lockAction;
    QAction *m_fullscreenAction;

    bool m_pressed;
};
#endif

/** @} */

#endif
