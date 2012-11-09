/*
  Q Light Controller
  sceneitems.cpp

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
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QMouseEvent>
#include <QMessageBox>
#include <QScrollBar>
#include <QSlider>
#include <QDebug>

#include "multitrackview.h"
#include "sceneitems.h"

#define TRACK_HEIGHT 80
#define TRACK_WIDTH 150

MultiTrackView::MultiTrackView(QWidget *parent) :
        QGraphicsView(parent)
{
    m_scene = new QGraphicsScene();
    m_scene->setSceneRect(0, 0, 2000, 600);
    setSceneRect(0, 0, 2000, 600);
    setScene(m_scene);
	
    QSlider *slider = new QSlider(Qt::Horizontal);
    slider->setRange(1, 15);
    slider->setSingleStep(1);
    slider->setFixedSize(TRACK_WIDTH - 4, 35);

    slider->setStyleSheet("QSlider { background-color: #969696; }"
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
    connect(slider, SIGNAL(valueChanged(int)), this, SLOT(slotTimeScaleChanged(int)));
    m_scene->addWidget(slider);

    // draw vertical "splitter" between tracks and sequences
    m_scene->addRect(TRACK_WIDTH - 3, 0, 3, m_scene->height(),
                        QPen( QColor(150, 150, 150, 255) ),
                        QBrush( QColor(190, 190, 190, 255) ) );
    // draw horizontal lines for tracks
    int ypos = 35 + TRACK_HEIGHT;
    for (int j = 0; j < 5; j++)
    m_scene->addRect(0, ypos + (j * TRACK_HEIGHT), m_scene->width(), 3,
                         QPen( QColor(150, 150, 150, 255) ),
                         QBrush( QColor(190, 190, 190, 255) ) );

    m_header = new SceneHeaderItem(m_scene->width());
    m_header->setPos(TRACK_WIDTH, 0);
    connect(m_header, SIGNAL(itemClicked(QGraphicsSceneMouseEvent *)),
            this, SLOT(slotMoveCursor(QGraphicsSceneMouseEvent *)));
    m_scene->addItem(m_header);

    m_cursor = new SceneCursorItem(m_scene->height());
    m_cursor->setPos(TRACK_WIDTH, 0);
    m_cursor->setZValue(99); // make sure the cursor is always on top of everything else
    m_scene->addItem(m_cursor);

    TrackItem *track = new TrackItem(1);
    track->setPos(0, 35);
    m_scene->addItem(track);
}

void MultiTrackView::resetView()
{
    if (m_sequences.count() > 0)
        for (int i = 0; i < m_sequences.count(); i++)
            m_scene->removeItem(m_sequences.at(i));
    m_sequences.clear();

    m_cursor->setPos(150, 0);
    this->horizontalScrollBar()->setSliderPosition(0);
    m_scene->update();
}

void MultiTrackView::addSequence(Chaser *chaser)
{
    SequenceItem *item = new SequenceItem(chaser);

    if (chaser->getStartTime() == UINT_MAX)
    {
        quint32 s_time = getTimeFromPosition();
        chaser->setStartTime(s_time);
        item->setPos(m_cursor->x() + 2, 36);
        item->setToolTip(QString(tr("Start time: %1msec\n%2"))
                         .arg(s_time).arg(tr("Click to move this sequence across the timeline")));
    }
    else
    {
        item->setPos(getPositionFromTime(chaser->getStartTime()) + 2, 36);
    }
    qDebug() << Q_FUNC_INFO << "sequence start time: " << chaser->getStartTime() << "msec";

    connect(item, SIGNAL(itemDropped(QGraphicsSceneMouseEvent *, SequenceItem *)),
            this, SLOT(slotSequenceMoved(QGraphicsSceneMouseEvent *, SequenceItem *)));
    m_scene->addItem(item);
    m_sequences.append(item);
    m_scene->update();
    this->update();
}

quint32 MultiTrackView::deleteSelectedSequence()
{
    int i = 0;
    SequenceItem *selItem = NULL;
    foreach(SequenceItem *item, m_sequences)
    {
        if (item->isSelected() == true)
        {
           selItem = item;
           break;
        }
        i++;
    }

    if (selItem != NULL)
    {
        QString msg = tr("Do you want to DELETE sequence:") + QString("\n\n") + selItem->getChaser()->name();

        // Ask for user's confirmation
        if (QMessageBox::question(this, tr("Delete Functions"), msg,
                                  QMessageBox::Yes, QMessageBox::No)
                                  == QMessageBox::Yes)
        {
            quint32 fID = selItem->getChaser()->id();
            m_scene->removeItem(selItem);
            m_sequences.removeAt(i);
            return fID;
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

quint32 MultiTrackView::getTimeFromPosition()
{
    quint32 s_time = (m_cursor->x() - TRACK_WIDTH) * (m_header->getTimeScale() * 1000) / (m_header->getTimeStep() * 2);
    return s_time;
}

quint32 MultiTrackView::getPositionFromTime(quint32 time)
{
    if (time == 0)
        return TRACK_WIDTH;
    int xPos = (time * (m_header->getTimeStep() / m_header->getTimeScale())) / 500;
    return TRACK_WIDTH + xPos;
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
    m_header->setTimeScale(val);
    foreach(SequenceItem *item, m_sequences)
    {
        int newXpos = getPositionFromTime(item->getChaser()->getStartTime());
        item->setPos(newXpos + 2, item->y());
        item->setTimeScale(val);
    }
    int newCursorPos = getPositionFromTime(m_cursor->getTime());
    m_cursor->setPos(newCursorPos + 2, m_cursor->y());
}
    
void MultiTrackView::slotSequenceMoved(QGraphicsSceneMouseEvent *, SequenceItem *item)
{
    //qDebug() << Q_FUNC_INFO << "event - <" << event->pos().toPoint().x() << "> - <" << event->pos().toPoint().y() << ">";
    // align to the closest track
    int ypos = item->y() - 36;
    ypos = (int)(ypos / (TRACK_HEIGHT + 1)) * (TRACK_HEIGHT + 1);
    int trackNum = (int)(ypos / (TRACK_HEIGHT + 1)) + 1;
    if (item->x() < TRACK_WIDTH + 2)
        item->setPos(TRACK_WIDTH + 2, ypos + 36); // avoid moving a sequence too early...
    else
        item->setPos(item->x(), ypos + 36);
    quint32 s_time = (item->x() - TRACK_WIDTH) * (m_header->getTimeScale() * 1000) / (m_header->getTimeStep() * 2);
    item->getChaser()->setStartTime(s_time);
    item->setToolTip(QString(tr("Start time: %1msec\n%2"))
                     .arg(s_time).arg(tr("Click to move this sequence across the timeline")));

    bool trackFound = false;
    foreach(TrackItem *trk, m_tracks)
    {
        if (trk->getTrackNumber() == trackNum)
        {
            trackFound = true;
            break;
        }
    }
    if (trackFound == false)
    {
        TrackItem *track = new TrackItem(trackNum);
        track->setPos(0, ypos + 36);
        m_scene->addItem(track);
    }

    m_scene->update();
    emit sequenceMoved(item);
}
