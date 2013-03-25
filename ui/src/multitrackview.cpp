/*
  Q Light Controller
  multitrackview.cpp

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

#include <QGraphicsSceneMouseEvent>
#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QMouseEvent>
#include <QMessageBox>
#include <QScrollBar>
#include <QSlider>
#include <QDebug>

#include "multitrackview.h"
#include "sceneitems.h"
#include "track.h"

#define HEADER_HEIGHT       35
#define TRACK_HEIGHT        80
#define TRACK_WIDTH         150
#define VIEW_DEFAULT_WIDTH  2000
#define VIEW_DEFAULT_HEIGHT 600

MultiTrackView::MultiTrackView(QWidget *parent) :
        QGraphicsView(parent)
{
    m_scene = new QGraphicsScene();
    m_scene->setSceneRect(0, 0, VIEW_DEFAULT_WIDTH, VIEW_DEFAULT_HEIGHT);
    setSceneRect(0, 0, VIEW_DEFAULT_WIDTH, VIEW_DEFAULT_HEIGHT);
    setScene(m_scene);
	
    m_timeSlider = new QSlider(Qt::Horizontal);
    m_timeSlider->setRange(1, 15);
    m_timeSlider->setValue(3);
    m_timeSlider->setSingleStep(1);
    m_timeSlider->setFixedSize(TRACK_WIDTH - 4, HEADER_HEIGHT);

    m_timeSlider->setStyleSheet("QSlider { background-color: #969696; }"
                          "QSlider::groove:horizontal {"
                          "border: 1px solid #999999;"
                          "height: 10px;"
                          "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #b1b1b1, stop:1 #d4d4d4);"
                          "}"
                          "QSlider::handle:horizontal {"
                          "background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #c4c4c4, stop:1 #8f8f8f);"
                          "border: 1px solid #5c5c5c;"
                          "width: 20px;"
                          "margin: -2px 0; /* handle is placed by default on the contents rect of the groove. Expand outside the groove */"
                          "border-radius: 4px;"
                          "}");
    connect(m_timeSlider, SIGNAL(valueChanged(int)), this, SLOT(slotTimeScaleChanged(int)));
    m_scene->addWidget(m_timeSlider);

    // draw vertical "splitter" between tracks and sequences
    m_scene->addRect(TRACK_WIDTH - 3, 0, 3, m_scene->height(),
                        QPen( QColor(150, 150, 150, 255) ),
                        QBrush( QColor(190, 190, 190, 255) ) );
    // draw horizontal lines for tracks
    updateTracksDividers();

    m_header = new SceneHeaderItem(m_scene->width());
    m_header->setPos(TRACK_WIDTH, 0);
    connect(m_header, SIGNAL(itemClicked(QGraphicsSceneMouseEvent *)),
            this, SLOT(slotMoveCursor(QGraphicsSceneMouseEvent *)));
    m_scene->addItem(m_header);

    m_cursor = new SceneCursorItem(m_scene->height());
    m_cursor->setPos(TRACK_WIDTH, 0);
    m_cursor->setZValue(99); // make sure the cursor is always on top of everything else
    m_scene->addItem(m_cursor);
    connect(horizontalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(slotViewScrolled(int)));
}

void MultiTrackView::updateTracksDividers()
{
    if (m_hdividers.count() > 0)
    {
        for (int c = 0; c < m_hdividers.count(); c++)
            m_scene->removeItem(m_hdividers.at(c));
        m_hdividers.clear();
    }
    int ypos = 35 + TRACK_HEIGHT;
    for (int j = 0; j < 5; j++)
    {
        QGraphicsItem *item = m_scene->addRect(0, ypos + (j * TRACK_HEIGHT),
                                               m_scene->width(), 2,
                                               QPen( QColor(150, 150, 150, 255) ),
                                               QBrush( QColor(190, 190, 190, 255) ) );
        m_hdividers.append(item);
    }
}

void MultiTrackView::setViewSize(int width, int height)
{
    m_scene->setSceneRect(0, 0, width, height);
    setSceneRect(0, 0, width, height);
    m_header->setWidth(width);
    updateTracksDividers();
}

void MultiTrackView::updateViewSize()
{
    quint32 gWidth = 0;
    foreach (SequenceItem *item, m_sequences)
    {
        if (item->x() + item->getWidth() > gWidth)
            gWidth = item->x() + item->getWidth();
    }
    if (gWidth > VIEW_DEFAULT_WIDTH)
        setViewSize(gWidth + 1000, VIEW_DEFAULT_HEIGHT);
}

void MultiTrackView::resetView()
{
    for (int t = 0; t < m_tracks.count(); t++)
        m_scene->removeItem(m_tracks.at(t));
    m_tracks.clear();

    for (int i = 0; i < m_sequences.count(); i++)
        m_scene->removeItem(m_sequences.at(i));
    m_sequences.clear();

    for (int i = 0; i < m_audio.count(); i++)
        m_scene->removeItem(m_audio.at(i));
    m_audio.clear();

    m_cursor->setPos(150, 0);
    this->horizontalScrollBar()->setSliderPosition(0);
    this->verticalScrollBar()->setSliderPosition(0);
    m_scene->update();
}

void MultiTrackView::addTrack(Track *track)
{
    TrackItem *trackItem = new TrackItem(track, m_tracks.count() + 1);
    trackItem->setName(track->name());
    trackItem->setPos(0, HEADER_HEIGHT + (TRACK_HEIGHT * m_tracks.count()));
    m_scene->addItem(trackItem);
    m_tracks.append(trackItem);
    activateTrack(track);
    connect(trackItem, SIGNAL(itemClicked(TrackItem*)), this, SLOT(slotTrackClicked(TrackItem*)));
    connect(trackItem, SIGNAL(itemSoloFlagChanged(TrackItem*,bool)), this, SLOT(slotTrackSoloFlagChanged(TrackItem*,bool)));
    connect(trackItem, SIGNAL(itemMuteFlagChanged(TrackItem*,bool)), this, SLOT(slotTrackMuteFlagChanged(TrackItem*,bool)));
}

void MultiTrackView::addSequence(Chaser *chaser)
{
    SequenceItem *item = new SequenceItem(chaser);
    int trackNum = getActiveTrack();
    if (trackNum < 0)
        trackNum = 0;
    item->setTrackIndex(trackNum);

    int timeScale = m_timeSlider->value();
    //m_header->setTimeScale(timeScale);

    if (chaser->getStartTime() == UINT_MAX)
    {
        quint32 s_time = getTimeFromPosition();
        chaser->setStartTime(s_time);
        item->setPos(m_cursor->x() + 2, 36 + (trackNum * TRACK_HEIGHT));
        item->setToolTip(QString(tr("Start time: %1msec\n%2"))
                         .arg(s_time).arg(tr("Click to move this sequence across the timeline")));
    }
    else
    {
        item->setPos(getPositionFromTime(chaser->getStartTime()) + 2, 36 + (trackNum * TRACK_HEIGHT));
    }
    item->setTimeScale(timeScale);
    qDebug() << Q_FUNC_INFO << "sequence start time: " << chaser->getStartTime() << "msec";

    connect(item, SIGNAL(itemDropped(QGraphicsSceneMouseEvent *, SequenceItem *)),
            this, SLOT(slotSequenceMoved(QGraphicsSceneMouseEvent *, SequenceItem *)));
    m_scene->addItem(item);
    m_sequences.append(item);
    int new_scene_width = item->x() + item->getWidth();
    if (new_scene_width > VIEW_DEFAULT_WIDTH && new_scene_width > m_scene->width())
        setViewSize(new_scene_width + 500, VIEW_DEFAULT_HEIGHT);
}

void MultiTrackView::addAudio(Audio *audio)
{
    AudioItem *item = new AudioItem(audio);
    int trackNum = getActiveTrack();
    if (trackNum < 0)
        trackNum = 0;
    item->setTrackIndex(trackNum);
    int timeScale = m_timeSlider->value();
    //m_header->setTimeScale(timeScale);

    if (audio->getStartTime() == UINT_MAX)
    {
        quint32 s_time = getTimeFromPosition();
        audio->setStartTime(s_time);
        item->setPos(m_cursor->x() + 2, 36 + (trackNum * TRACK_HEIGHT));
        item->setToolTip(QString(tr("Start time: %1msec\n%2"))
                         .arg(s_time).arg(tr("Click to move this audio across the timeline")));
    }
    else
    {
        item->setPos(getPositionFromTime(audio->getStartTime()) + 2, 36 + (trackNum * TRACK_HEIGHT));
    }
    item->setTimeScale(timeScale);
    qDebug() << Q_FUNC_INFO << "audio start time: " << audio->getStartTime() << "msec";

    connect(item, SIGNAL(itemDropped(QGraphicsSceneMouseEvent *, AudioItem *)),
            this, SLOT(slotSequenceMoved(QGraphicsSceneMouseEvent *, AudioItem *)));
    connect(audio, SIGNAL(totalTimeChanged(qint64)), item, SLOT(updateDuration()));
    m_scene->addItem(item);
    m_audio.append(item);
    int new_scene_width = item->x() + item->getWidth();
    if (new_scene_width > VIEW_DEFAULT_WIDTH && new_scene_width > m_scene->width())
        setViewSize(new_scene_width + 500, VIEW_DEFAULT_HEIGHT);
}

quint32 MultiTrackView::deleteSelectedFunction()
{
    int i = 0;

    foreach(SequenceItem *item, m_sequences)
    {
        if (item->isSelected() == true)
        {
            QString msg = tr("Do you want to DELETE sequence:") + QString("\n\n") + item->getChaser()->name();

            // Ask for user's confirmation
            if (QMessageBox::question(this, tr("Delete Functions"), msg,
                                      QMessageBox::Yes, QMessageBox::No)
                                      == QMessageBox::Yes)
            {
                quint32 fID = item->getChaser()->id();
                m_scene->removeItem(item);
                m_sequences.removeAt(i);
                return fID;
            }
        }
        i++;
    }
    i = 0;
    foreach(AudioItem *item, m_audio)
    {
        if (item->isSelected() == true)
        {
            QString msg = tr("Do you want to DELETE audio (the source file will NOT be removed):") +
                          QString("\n\n") + item->getAudio()->name();

            // Ask for user's confirmation
            if (QMessageBox::question(this, tr("Delete Functions"), msg,
                                  QMessageBox::Yes, QMessageBox::No)
                                  == QMessageBox::Yes)
            {
                quint32 fID = item->getAudio()->id();
                m_scene->removeItem(item);
                m_audio.removeAt(i);
                return fID;
            }
        }
        i++;
    }

    i = 0;
    foreach(TrackItem *item, m_tracks)
    {
        if (item->isActive() == true)
        {
            Track *track = item->getTrack();
            quint32 fID = track->getSceneID();
            QList <quint32> ids = track->functionsID();
            QString msg = tr("Do you want to DELETE scene:") + QString("\n\n") + track->name();
            if (ids.count() > 0)
            {
                msg += QString("\n\n") + tr("This operation will also DELETE:" ) + QString("\n\n");
                foreach (SequenceItem *item, m_sequences)
                {
                    Chaser *chaser = item->getChaser();
                    if (chaser->getBoundedSceneID() == fID)
                        msg += chaser->name() + QString("\n");
                }
            }

            // Ask for user's confirmation
            if (QMessageBox::question(this, tr("Delete Functions"), msg,
                                  QMessageBox::Yes, QMessageBox::No)
                                  == QMessageBox::Yes)
            {
                m_scene->removeItem(item);
                m_tracks.removeAt(i);
                return fID;
            }
        }
        i++;
    }

    return Function::invalidId();
}

void MultiTrackView::moveCursor(quint32 timePos)
{
    int newPos = getPositionFromTime(timePos);
    m_cursor->setPos(newPos, 0);
}

void MultiTrackView::rewindCursor()
{
    m_cursor->setPos(TRACK_WIDTH, 0);
}

void MultiTrackView::activateTrack(Track *track)
{
    foreach(TrackItem *item, m_tracks)
    {
        if (item->getTrack()->id() == track->id())
            item->setActive(true);
        else
            item->setActive(false);
    }
}

SequenceItem *MultiTrackView::getSelectedSequence()
{
    foreach(SequenceItem *item, m_sequences)
    {
        if (item->isSelected() == true)
            return item;
    }
    return NULL;
}

AudioItem *MultiTrackView::getSelectedAudio()
{
    foreach(AudioItem *item, m_audio)
    {
        if (item->isSelected() == true)
            return item;
    }
    return NULL;
}


quint32 MultiTrackView::getTimeFromPosition()
{
    quint32 s_time = (double)(m_cursor->x() - TRACK_WIDTH) * (m_header->getTimeScale() * 1000) /
                     (double)(m_header->getHalfSecondWidth() * 2);
    return s_time;
}

quint32 MultiTrackView::getPositionFromTime(quint32 time)
{
    if (time == 0)
        return TRACK_WIDTH;
    quint32 xPos = ((double)time / 500) * ((double)m_header->getHalfSecondWidth() /
                                           (double)m_header->getTimeScale());
    return TRACK_WIDTH + xPos;
}

int MultiTrackView::getActiveTrack()
{
    int index = 0;
    foreach (TrackItem *track, m_tracks)
    {
        if (track->isActive())
            return index;
        index++;
    }

    return -1;
}

void MultiTrackView::setHeaderType(int type)
{
    m_header->setTimeDivisionType((SceneHeaderItem::TimeDivision)type);
}

void MultiTrackView::setBPMValue(int value)
{
    m_header->setBPMValue(value);
}

void MultiTrackView::mouseReleaseEvent(QMouseEvent * e)
{
    emit viewClicked(e);

    QGraphicsView::mouseReleaseEvent(e);
    qDebug() << Q_FUNC_INFO << "View clicked at pos: " << e->pos().x() << e->pos().y();

}

void MultiTrackView::slotMoveCursor(QGraphicsSceneMouseEvent *event)
{
    //qDebug() << Q_FUNC_INFO << "event - <" << event->pos().toPoint().x() << "> - <" << event->pos().toPoint().y() << ">";
    m_cursor->setPos(TRACK_WIDTH +  event->pos().toPoint().x(), 0);
    m_cursor->setTime(getTimeFromPosition());
    emit timeChanged(getTimeFromPosition());
}

void MultiTrackView::slotTimeScaleChanged(int val)
{
    //int oldScale = m_header->getTimeScale();
    m_header->setTimeScale(val);
    foreach(SequenceItem *item, m_sequences)
    {
        quint32 newXpos = getPositionFromTime(item->getChaser()->getStartTime());
        item->setPos(newXpos + 2, item->y());
        item->setTimeScale(val);
    }
    foreach(AudioItem *item, m_audio)
    {
        quint32 newXpos = getPositionFromTime(item->getAudio()->getStartTime());
        item->setPos(newXpos + 2, item->y());
        item->setTimeScale(val);
    }
    int newCursorPos = getPositionFromTime(m_cursor->getTime());
    m_cursor->setPos(newCursorPos + 2, m_cursor->y());
    updateViewSize();
}

void MultiTrackView::slotTrackClicked(TrackItem *track)
{
    foreach(TrackItem *item, m_tracks)
    {
        if (item == track)
            item->setActive(true);
        else
            item->setActive(false);
    }
    emit trackClicked(track->getTrack());
}

void MultiTrackView::slotTrackSoloFlagChanged(TrackItem* track, bool solo)
{
    foreach(TrackItem *item, m_tracks)
    {
        if (item != track)
            item->setFlags(false, solo);
        Track *trk = item->getTrack();
        if (trk != NULL)
            trk->setMute(item->isMute());
    }
}

void MultiTrackView::slotTrackMuteFlagChanged(TrackItem* item, bool mute)
{
    Track *trk = item->getTrack();
    if (trk != NULL)
        trk->setMute(mute);
}

void MultiTrackView::slotViewScrolled(int)
{
    //qDebug() << Q_FUNC_INFO << "Percentage: " << value;
}

void MultiTrackView::slotSequenceMoved(QGraphicsSceneMouseEvent *, SequenceItem *item)
{
    //qDebug() << Q_FUNC_INFO << "event - <" << event->pos().toPoint().x() << "> - <" << event->pos().toPoint().y() << ">";
    // align to the appropriate track
    int trackNum = item->getTrackIndex();
    int ypos = HEADER_HEIGHT + 1 + (trackNum * TRACK_HEIGHT);

    if (item->x() < TRACK_WIDTH + 2)
        item->setPos(TRACK_WIDTH + 2, ypos); // avoid moving a sequence too early...
    else
        item->setPos(item->x(), ypos);
    quint32 s_time = (item->x() - TRACK_WIDTH) * (double)(m_header->getTimeScale() * 1000) /
                                                 (double)(m_header->getHalfSecondWidth() * 2);
    item->getChaser()->setStartTime(s_time);
    item->setToolTip(QString(tr("Start time: %1\n%2"))
                     .arg(Function::speedToString(s_time)).arg(tr("Click to move this sequence across the timeline")));

    m_scene->update();
    emit sequenceMoved(item);
}

void MultiTrackView::slotSequenceMoved(QGraphicsSceneMouseEvent *, AudioItem *item)
{
    //qDebug() << Q_FUNC_INFO << "event - <" << event->pos().toPoint().x() << "> - <" << event->pos().toPoint().y() << ">";
    // align to the appropriate track
    int trackNum = item->getTrackIndex();
    int ypos = HEADER_HEIGHT + 1 + (trackNum * TRACK_HEIGHT);

    if (item->x() < TRACK_WIDTH + 2)
        item->setPos(TRACK_WIDTH + 2, ypos); // avoid moving a sequence too early...
    else
        item->setPos(item->x(), ypos);
    quint32 s_time = (double)(item->x() - TRACK_WIDTH) * (double)(m_header->getTimeScale() * 1000) /
                     (double)(m_header->getHalfSecondWidth() * 2);
    item->getAudio()->setStartTime(s_time);
    item->setToolTip(QString(tr("Start time: %1\n%2"))
                     .arg(Function::speedToString(s_time)).arg(tr("Click to move this sequence across the timeline")));

    m_scene->update();
    emit audioMoved(item);
}
