/*
  Q Light Controller Plus
  sceneitems.h

  Copyright (C) Heikki Junnila

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


#ifndef SCENEITEMS_H
#define SCENEITEMS_H

#include <QGraphicsItem>
#include <QObject>
#include <QAction>
#include <QFont>

#include "chaser.h"
#include "audio.h"
#include "track.h"

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

protected slots:
    void slotTrackChanged(quint32 id);

signals:
    void itemClicked(TrackItem *);
    void itemSoloFlagChanged(TrackItem *, bool);
    void itemMuteFlagChanged(TrackItem *, bool);

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

    QFont m_font;
    bool m_pressed;

    QAction *m_alignToCursor;
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
    void slotAudioPreviewStero(bool active);
    void slotAlignToCursorClicked();

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
    /** Reference to the actual Chaser object which holds the sequence steps */
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

    /** Pixmap holding the waveform (if enabled) */
    QPixmap *m_preview;

    bool m_pressed;
};

#endif
