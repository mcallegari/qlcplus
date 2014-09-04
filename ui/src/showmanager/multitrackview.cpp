/*
  Q Light Controller Plus
  multitrackview.cpp

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
#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QMouseEvent>
#include <QMessageBox>
#include <QScrollBar>
#include <QSlider>
#include <QDebug>

#include "multitrackview.h"
#include "track.h"

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

    m_header = new ShowHeaderItem(m_scene->width());
    m_header->setPos(TRACK_WIDTH, 0);
    connect(m_header, SIGNAL(itemClicked(QGraphicsSceneMouseEvent *)),
            this, SLOT(slotHeaderClicked(QGraphicsSceneMouseEvent *)));
    m_scene->addItem(m_header);
    m_snapToGrid = false;

    m_cursor = new ShowCursorItem(m_scene->height());
    m_cursor->setPos(TRACK_WIDTH, 0);
    m_cursor->setZValue(999); // make sure the cursor is always on top of everything else
    m_scene->addItem(m_cursor);
    connect(horizontalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(slotViewScrolled(int)));

    m_vdivider = NULL;
    // draw horizontal and vertical lines for tracks
    updateTracksDividers();
}

void MultiTrackView::updateTracksDividers()
{
    if (m_hdividers.count() > 0)
    {
        int hdivCount = m_hdividers.count();
        for (int c = 0; c < hdivCount; c++)
            m_scene->removeItem(m_hdividers.takeLast());
        m_hdividers.clear();
    }
    if (m_vdivider != NULL)
        m_scene->removeItem(m_vdivider);

    int ypos = 35 + TRACK_HEIGHT;
    int hDivNum = 6;
    if (m_tracks.count() > 5)
        hDivNum = m_tracks.count();
    for (int j = 0; j < hDivNum; j++)
    {
        QGraphicsItem *item = m_scene->addRect(0, ypos + (j * TRACK_HEIGHT),
                                               m_scene->width(), 1,
                                               QPen( QColor(150, 150, 150, 255) ),
                                               QBrush( QColor(190, 190, 190, 255) ) );
        item->setZValue(-1);
        m_hdividers.append(item);
    }
    m_vdivider = m_scene->addRect(TRACK_WIDTH - 3, 0, 3, m_scene->height(),
                        QPen( QColor(150, 150, 150, 255) ),
                        QBrush( QColor(190, 190, 190, 255) ) );
}

void MultiTrackView::setViewSize(int width, int height)
{
    m_scene->setSceneRect(0, 0, width, height);
    setSceneRect(0, 0, width, height);
    m_header->setWidth(width);
    if (m_snapToGrid == true)
        m_header->setHeight(height);
    else
        m_header->setHeight(HEADER_HEIGHT);
    updateTracksDividers();
}

void MultiTrackView::updateViewSize()
{
    quint32 gWidth = VIEW_DEFAULT_WIDTH;
    quint32 gHeight = VIEW_DEFAULT_HEIGHT;

    // find leftmost sequence item
    foreach (SequenceItem *item, m_sequences)
    {
        if (item->x() + item->getWidth() > gWidth)
            gWidth = item->x() + item->getWidth();
    }
    // find leftmost audio item
    foreach (AudioItem *item, m_audio)
    {
        if (item->x() + item->getWidth() > gWidth)
            gWidth = item->x() + item->getWidth();
    }
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    // find leftmost video item
    foreach (VideoItem *item, m_videos)
    {
        if (item->x() + item->getWidth() > gWidth)
            gWidth = item->x() + item->getWidth();
    }
#endif

    if ((m_tracks.count() * TRACK_HEIGHT) + HEADER_HEIGHT > VIEW_DEFAULT_HEIGHT)
    {
        gHeight = (m_tracks.count() * TRACK_HEIGHT) + HEADER_HEIGHT;
        m_cursor->setHeight(gHeight);
    }

    if (gWidth > VIEW_DEFAULT_WIDTH || gHeight > VIEW_DEFAULT_HEIGHT)
        setViewSize(gWidth + 1000, gHeight);
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

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    for (int i = 0; i < m_videos.count(); i++)
        m_scene->removeItem(m_videos.at(i));
    m_videos.clear();
#endif
    m_cursor->setPos(150, 0);
    this->horizontalScrollBar()->setSliderPosition(0);
    this->verticalScrollBar()->setSliderPosition(0);
    m_scene->update();
}

void MultiTrackView::addTrack(Track *track)
{
    TrackItem *trackItem = new TrackItem(track, m_tracks.count());
    trackItem->setName(track->name());
    trackItem->setPos(0, HEADER_HEIGHT + (TRACK_HEIGHT * m_tracks.count()));
    m_scene->addItem(trackItem);
    m_tracks.append(trackItem);
    activateTrack(track);
    connect(trackItem, SIGNAL(itemClicked(TrackItem*)),
            this, SLOT(slotTrackClicked(TrackItem*)));
    connect(trackItem, SIGNAL(itemDoubleClicked(TrackItem*)),
            this, SLOT(slotTrackDoubleClicked(TrackItem*)));
    connect(trackItem, SIGNAL(itemSoloFlagChanged(TrackItem*,bool)),
            this, SLOT(slotTrackSoloFlagChanged(TrackItem*,bool)));
    connect(trackItem, SIGNAL(itemMuteFlagChanged(TrackItem*,bool)),
            this, SLOT(slotTrackMuteFlagChanged(TrackItem*,bool)));
    connect(trackItem, SIGNAL(itemMoveUpDown(Track*,int)),
            this, SIGNAL(trackMoved(Track*,int)));
    connect(trackItem, SIGNAL(itemRequestDelete(Track*)),
            this, SIGNAL(trackDelete(Track*)));
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
        item->setStartTime(getTimeFromCursor());
        item->setPos(m_cursor->x() + 2, 36 + (trackNum * TRACK_HEIGHT));
    }
    else
    {
        item->setPos(getPositionFromTime(chaser->getStartTime()) + 2, 36 + (trackNum * TRACK_HEIGHT));
    }
    item->setTimeScale(timeScale);
    qDebug() << Q_FUNC_INFO << "sequence start time: " << chaser->getStartTime() << "msec";

    connect(item, SIGNAL(itemDropped(QGraphicsSceneMouseEvent *, ShowItem *)),
            this, SLOT(slotItemMoved(QGraphicsSceneMouseEvent *, ShowItem *)));
    connect(item, SIGNAL(alignToCursor(ShowItem*)),
            this, SLOT(slotAlignToCursor(ShowItem*)));
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
        item->setStartTime(getTimeFromCursor());
        item->setPos(m_cursor->x() + 2, 36 + (trackNum * TRACK_HEIGHT));
    }
    else
    {
        item->setPos(getPositionFromTime(audio->getStartTime()) + 2, 36 + (trackNum * TRACK_HEIGHT));
    }
    item->setTimeScale(timeScale);
    qDebug() << Q_FUNC_INFO << "audio start time: " << audio->getStartTime() << "msec";

    connect(item, SIGNAL(itemDropped(QGraphicsSceneMouseEvent *, ShowItem *)),
            this, SLOT(slotItemMoved(QGraphicsSceneMouseEvent *, ShowItem *)));
    connect(item, SIGNAL(alignToCursor(ShowItem*)),
            this, SLOT(slotAlignToCursor(ShowItem*)));
    connect(audio, SIGNAL(totalTimeChanged(qint64)), item, SLOT(updateDuration()));
    m_scene->addItem(item);
    m_audio.append(item);
    int new_scene_width = item->x() + item->getWidth();
    if (new_scene_width > VIEW_DEFAULT_WIDTH && new_scene_width > m_scene->width())
        setViewSize(new_scene_width + 500, VIEW_DEFAULT_HEIGHT);
}

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
void MultiTrackView::addVideo(Video *video)
{
    VideoItem *item = new VideoItem(video);
    int trackNum = getActiveTrack();
    if (trackNum < 0)
        trackNum = 0;
    item->setTrackIndex(trackNum);
    int timeScale = m_timeSlider->value();
    //m_header->setTimeScale(timeScale);

    if (video->getStartTime() == UINT_MAX)
    {
        item->setStartTime(getTimeFromCursor());
        item->setPos(m_cursor->x() + 2, 36 + (trackNum * TRACK_HEIGHT));
    }
    else
    {
        item->setPos(getPositionFromTime(video->getStartTime()) + 2, 36 + (trackNum * TRACK_HEIGHT));
    }
    item->setTimeScale(timeScale);
    qDebug() << Q_FUNC_INFO << "video start time: " << video->getStartTime() << "msec";

    connect(item, SIGNAL(itemDropped(QGraphicsSceneMouseEvent *, ShowItem *)),
            this, SLOT(slotItemMoved(QGraphicsSceneMouseEvent *, ShowItem *)));
    connect(item, SIGNAL(alignToCursor(ShowItem*)),
            this, SLOT(slotAlignToCursor(ShowItem*)));
    connect(video, SIGNAL(totalTimeChanged(qint64)), item, SLOT(updateDuration()));
    m_scene->addItem(item);
    m_videos.append(item);
    int new_scene_width = item->x() + item->getWidth();
    if (new_scene_width > VIEW_DEFAULT_WIDTH && new_scene_width > m_scene->width())
        setViewSize(new_scene_width + 500, VIEW_DEFAULT_HEIGHT);
}
#endif

quint32 MultiTrackView::deleteSelectedFunction()
{
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
                m_sequences.removeOne(item);
                return fID;
            }
        }
    }

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
                m_audio.removeOne(item);
                return fID;
            }
            return Function::invalidId();
        }
    }

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    foreach(VideoItem *item, m_videos)
    {
        if (item->isSelected() == true)
        {
            QString msg = tr("Do you want to DELETE video (the source file will NOT be removed):") +
                          QString("\n\n") + item->getVideo()->name();

            // Ask for user's confirmation
            if (QMessageBox::question(this, tr("Delete Functions"), msg,
                                  QMessageBox::Yes, QMessageBox::No)
                                  == QMessageBox::Yes)
            {
                quint32 fID = item->getVideo()->id();
                m_scene->removeItem(item);
                m_videos.removeOne(item);
                return fID;
            }
            return Function::invalidId();
        }
    }
#endif

    foreach(TrackItem *item, m_tracks)
    {
        if (item->isActive() == true)
        {
            Track *track = item->getTrack();
            quint32 sceneID = track->getSceneID();
            quint32 trkID = track->id();
            QList <quint32> ids = track->functionsID();
            QString msg = tr("Do you want to DELETE track:") + QString("\n\n") + track->name();
            if (ids.count() > 0)
            {
                msg += QString("\n\n") + tr("This operation will also DELETE:" ) + QString("\n\n");
                foreach (SequenceItem *item, m_sequences)
                {
                    Chaser *chaser = item->getChaser();
                    if (chaser->getBoundSceneID() == sceneID)
                        msg += chaser->name() + QString("\n");
                }
            }

            // Ask for user's confirmation
            if (QMessageBox::question(this, tr("Delete Functions"), msg,
                                  QMessageBox::Yes, QMessageBox::No)
                                  == QMessageBox::Yes)
            {
                m_scene->removeItem(item);
                m_tracks.removeOne(item);
                return trkID;
            }
            return Function::invalidId();
        }
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

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
VideoItem *MultiTrackView::getSelectedVideo()
{
    foreach(VideoItem *item, m_videos)
    {
        if (item->isSelected() == true)
            return item;
    }
    return NULL;
}
#endif

quint32 MultiTrackView::getTimeFromCursor()
{
    quint32 s_time = (double)(m_cursor->x() - TRACK_WIDTH) *
                     (m_header->getTimeScale() * 1000) /
                     (double)(m_header->getHalfSecondWidth() * 2);
    return s_time;
}

quint32 MultiTrackView::getTimeFromPosition(qreal pos)
{
    return ((double)(pos - TRACK_WIDTH) *
            (double)(m_header->getTimeScale() * 1000) /
            (double)(m_header->getHalfSecondWidth() * 2));
}

quint32 MultiTrackView::getPositionFromTime(quint32 time)
{
    if (time == 0)
        return TRACK_WIDTH;
    quint32 xPos = ((double)time / 500) *
                    ((double)m_header->getHalfSecondWidth() /
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

void MultiTrackView::setHeaderType(ShowHeaderItem::TimeDivision type)
{
    m_header->setTimeDivisionType(type);
}

ShowHeaderItem::TimeDivision MultiTrackView::getHeaderType()
{
    return m_header->getTimeDivisionType();
}

void MultiTrackView::setBPMValue(int value)
{
    m_header->setBPMValue(value);
}

void MultiTrackView::setSnapToGrid(bool enable)
{
    m_snapToGrid = enable;
    if (enable == true)
        m_header->setHeight(m_scene->height());
    else
        m_header->setHeight(HEADER_HEIGHT);
}

void MultiTrackView::mouseReleaseEvent(QMouseEvent * e)
{
    if (getSelectedSequence() == NULL && getSelectedAudio() == NULL
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
        && getSelectedVideo() == NULL
#endif
       )
    {
        quint32 xpos = mapToScene(e->pos()).x();
        if (xpos > TRACK_WIDTH)
        {
            m_cursor->setPos(xpos, 0);
            m_cursor->setTime(getTimeFromCursor());
            emit timeChanged(m_cursor->getTime());
        }
        emit viewClicked(e);
    }

    QGraphicsView::mouseReleaseEvent(e);
    //qDebug() << Q_FUNC_INFO << "View clicked at pos: " << e->pos().x() << e->pos().y();
}

void MultiTrackView::slotHeaderClicked(QGraphicsSceneMouseEvent *event)
{
    m_cursor->setPos(TRACK_WIDTH +  event->pos().toPoint().x(), 0);
    m_cursor->setTime(getTimeFromCursor());
    qDebug() << Q_FUNC_INFO << "Cursor moved to time:" << m_cursor->getTime();
    emit timeChanged(m_cursor->getTime());
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
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    foreach(VideoItem *item, m_videos)
    {
        quint32 newXpos = getPositionFromTime(item->getVideo()->getStartTime());
        item->setPos(newXpos + 2, item->y());
        item->setTimeScale(val);
    }
#endif
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

void MultiTrackView::slotTrackDoubleClicked(TrackItem *track)
{
    emit trackDoubleClicked(track->getTrack());
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

void MultiTrackView::slotItemMoved(QGraphicsSceneMouseEvent *event, ShowItem *item)
{
    qDebug() << Q_FUNC_INFO << "event - <" << event->pos().toPoint().x() << "> - <" << event->pos().toPoint().y() << ">";
    // align to the appropriate track
    bool moved = true;
    quint32 s_time = 0;
    int trackNum = item->getTrackIndex();
    int ypos = HEADER_HEIGHT + 1 + (trackNum * TRACK_HEIGHT);
    int shift = qAbs(item->getDraggingPos().x() - item->x());

    if (item->x() < TRACK_WIDTH + 2)
    {
        item->setPos(TRACK_WIDTH + 2, ypos); // avoid moving a sequence too early...
    }
    else if (shift < 3) // a drag of less than 3 pixel doesn't move the item
    {
        qDebug() << "Drag too short (" << shift << "px) not allowed !";
        item->setPos(item->getDraggingPos());
        s_time = item->getStartTime();
        moved = false;
    }
    else if (m_snapToGrid == true)
    {
        float step = m_header->getTimeDivisionStep();
        float gridPos = ((int)(item->x() / step) * step);
        item->setPos(gridPos, ypos);
        s_time = getTimeFromPosition(gridPos);
    }
    else
    {
        item->setPos(item->x(), ypos);
        s_time = getTimeFromPosition(item->x());
    }

    item->setStartTime(s_time);

    m_scene->update();
    emit showItemMoved(item, getTimeFromPosition(item->x() + event->pos().toPoint().x()), moved);
}

void MultiTrackView::slotAlignToCursor(ShowItem *item)
{
    item->setX(m_cursor->x());
    item->setStartTime(getTimeFromPosition(item->x()));
    m_scene->update();
}



