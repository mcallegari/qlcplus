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

#include <QtGui>

#include "sceneitems.h"
#include "chaserstep.h"

/****************************************************************************
 * Header item
 ****************************************************************************/
SceneHeaderItem::SceneHeaderItem(int w)
    : m_width(w)
    , m_timeStep(25)
    , m_timeScale(1)
{
}

void SceneHeaderItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsItem::mousePressEvent(event);
    emit itemClicked(event);
}

QRectF SceneHeaderItem::boundingRect() const
{
    return QRectF(0, 0, m_width, 35);
}

void SceneHeaderItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    // draw base background
    painter->setPen(QPen(QColor(100, 100, 100, 255), 1));
    painter->setBrush(QBrush(QColor(150, 150, 150, 255)));
    painter->drawRect(0, 0, m_width, 35);

    // draw vertical timing lines and time labels
    int tmpSec = 0;
    for (int i = 0; i < m_width / m_timeStep; i++)
    {
        int xpos = (i * m_timeStep) + 1;
        painter->setPen(QPen( QColor(250, 250, 250, 255), 1));
        if (i%2 == 0)
        {
            painter->drawLine(xpos, 20, xpos, 34);
            painter->setPen(QPen( Qt::black, 1));
            tmpSec = (i/2) * m_timeScale;
            if (tmpSec < 60)
                painter->drawText(xpos - 4, 15, QString("%1s").arg(tmpSec));
            else
            {
                int tmpMin = tmpSec / 60;
                tmpSec = tmpSec - (tmpMin * 60);
                painter->drawText(xpos - 4, 15, QString("%1m%2s").arg(tmpMin).arg(tmpSec));
            }
        }
        else
            painter->drawLine(xpos, 25, xpos, 34);
    }

}

void SceneHeaderItem::setTimeScale(int val)
{
    m_timeScale = val;
    update();
}

int SceneHeaderItem::getTimeScale()
{
    return m_timeScale;
}

int SceneHeaderItem::getTimeStep()
{
    return m_timeStep;
}

/****************************************************************************
 * Cursor item
 ****************************************************************************/
SceneCursorItem::SceneCursorItem(int h)
    : m_height(h)
    , m_time(0)
{
}

void SceneCursorItem::setTime(quint32 t)
{
    m_time = t;
}

quint32 SceneCursorItem::getTime()
{
    return m_time;
}


QRectF SceneCursorItem::boundingRect() const
{
    return QRectF(-5, 0, 10, m_height);
}

void SceneCursorItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    painter->setBrush(QBrush(Qt::yellow, Qt::SolidPattern));
    painter->setPen(QPen(Qt::yellow, 1));
    QPolygonF CursorHead;
    CursorHead.append(QPointF(-5.0, 22.0));
    CursorHead.append(QPointF(5.0, 22.0));
    CursorHead.append(QPointF(5.0, 25.0));
    CursorHead.append(QPointF(0.0, 35.0));
    CursorHead.append(QPointF(-5.0, 25.0));
    CursorHead.append(QPointF(-5.0, 22.0));
    painter->drawPolygon(CursorHead);
    painter->setPen(Qt::NoPen);
    painter->drawRect(0, 35, 1, m_height - 35);
}

/****************************************************************************
 * Track item
 ****************************************************************************/
TrackItem::TrackItem(Track *track, int number)
    : m_number(number)
    , m_isActive(false)
    , m_track(track)
    , m_isMute(false)
    , m_isSolo(false)
{
    m_font = QApplication::font();
    m_font.setBold(true);
    m_font.setPixelSize(12);

    m_btnFont = QApplication::font();
    m_btnFont.setBold(true);
    m_btnFont.setPixelSize(12);

    if (track != NULL)
    {
        m_name = m_track->name();
        m_isMute = m_track->isMute();
        connect(m_track, SIGNAL(changed(quint32)), this, SLOT(slotTrackChanged(quint32)));
    }
    else
        m_name = QString("Track %1").arg(m_number);

    m_soloRegion = new QRectF(15.0, 10.0, 20.0, 20.0);
    m_muteRegion = new QRectF(40.0, 10.0, 20.0, 20.0);
}

Track *TrackItem::getTrack()
{
    return m_track;
}

int TrackItem::getTrackNumber()
{
    return m_number;
}

void TrackItem::setName(QString name)
{
    if (!name.isEmpty())
        m_name = name;
    update();
}

void TrackItem::setActive(bool flag)
{
    m_isActive = flag;
    update();
}

bool TrackItem::isActive()
{
    return m_isActive;
}

void TrackItem::setFlags(bool solo, bool mute)
{
    m_isSolo = solo;
    m_isMute = mute;
    update();
}

bool TrackItem::isMute()
{
    return m_isMute;
}

void TrackItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    m_isActive = true;
    QGraphicsItem::mousePressEvent(event);
    if (m_soloRegion->contains(event->pos().toPoint()))
    {
        m_isSolo = !m_isSolo;
        emit itemSoloFlagChanged(this, m_isSolo);
    }
    if (m_muteRegion->contains(event->pos().toPoint()))
    {
        m_isMute = !m_isMute;
        emit itemMuteFlagChanged(this, m_isMute);
    }
    emit itemClicked(this);
}

QRectF TrackItem::boundingRect() const
{
    return QRectF(0, 0, 146, 80);
}

void TrackItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    // draw background gradient
    QLinearGradient linearGrad(QPointF(0, 0), QPointF(0, 80));
    linearGrad.setColorAt(0, QColor(200, 200, 200, 255));
    linearGrad.setColorAt(1, QColor(120, 120, 120, 255));
    painter->setBrush(linearGrad);
    painter->drawRect(0, 0, 146, 79);

    // Draw left bar that shows if the track is active or not
    painter->setPen(QPen(QColor(10, 10, 10, 150), 1));
    if (m_isActive == true)
        painter->setBrush(QBrush(QColor(0, 255, 0, 255)));
    else
        painter->setBrush(QBrush(QColor(180, 180, 180, 255)));
    painter->drawRect(1, 1, 7, 40);

    // draw solo button
    painter->setPen(QPen(QColor(200, 200, 200, 255), 2));
    if (m_isSolo)
        painter->setBrush(QBrush(QColor(255, 255, 0, 255)));
    else
        painter->setBrush(QBrush(QColor(128, 128, 0, 255)));
    painter->drawRoundedRect(m_soloRegion->toRect(), 5.0, 5.0);
    painter->setFont(m_btnFont);
    painter->drawText(21, 25, "S");

    // draw mute button
    if (m_isMute)
        painter->setBrush(QBrush(QColor(255, 0, 0, 255)));
    else
        painter->setBrush(QBrush(QColor(128, 0, 0, 255)));
    painter->drawRoundedRect(m_muteRegion->toRect(), 5.0, 5.0);
    painter->drawText(44, 25, "M");

    painter->setFont(m_font);
    // draw shadow
    painter->setPen(QPen(QColor(10, 10, 10, 150), 2));
    painter->drawText(6, 71, m_name);
    // draw track name
    painter->setPen(QPen(QColor(200, 200, 200, 255), 2));
    painter->drawText(5, 70, m_name);
}

void TrackItem::slotTrackChanged(quint32 id)
{
    Q_UNUSED(id);

    m_name = m_track->name();
    update();
}


/*********************************************************************
 * Sequence item
 *********************************************************************/

SequenceItem::SequenceItem(Chaser *seq)
    : color(qrand() % 256, qrand() % 256, qrand() % 256)
    , m_chaser(seq)
    , m_width(50)
    , m_timeScale(1)
    , m_trackIdx(-1)
{
    Q_ASSERT(seq != NULL);
    setToolTip(QString("Start time: %1msec\n%2")
              .arg(seq->getStartTime()).arg("Click to move this sequence across the timeline"));
    setCursor(Qt::OpenHandCursor);
    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    calculateWidth();
    connect(m_chaser, SIGNAL(changed(quint32)), this, SLOT(slotSequenceChanged(quint32)));
}

void SequenceItem::calculateWidth()
{
    int newWidth = 0;
    unsigned long seq_duration = 0;

    foreach (ChaserStep step, m_chaser->steps())
        seq_duration += step.duration;

    if (seq_duration != 0)
        newWidth = ((50/m_timeScale) * seq_duration) / 1000;

    if (newWidth < (50 / m_timeScale))
        newWidth = 50 / m_timeScale;
    m_width = newWidth;
}

QRectF SequenceItem::boundingRect() const
{
    return QRectF(0, 0, m_width, 77);
}

void SequenceItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    if (this->isSelected() == true)
        painter->setPen(QPen(Qt::white, 3));
    else
        painter->setPen(QPen(Qt::white, 1));
    painter->setBrush(QBrush(color));

    painter->drawRect(0, 0, m_width, 77);
    /* draw vertical lines to show the chaser's steps */
    int xpos = 0;
    painter->setPen(QPen(Qt::white, 1));
    foreach (ChaserStep step, m_chaser->steps())
    {
        xpos += (((50/m_timeScale) * step.duration) / 1000);
        painter->drawLine(xpos, 1, xpos, 75);
    }
}

void SequenceItem::setTimeScale(int val)
{
    prepareGeometryChange();
    m_timeScale = val;
    calculateWidth();
}

void SequenceItem::setTrackIndex(int idx)
{
    m_trackIdx = idx;
}

int SequenceItem::getTrackIndex()
{
    return m_trackIdx;
}

Chaser *SequenceItem::getChaser()
{
    return m_chaser;
}

void SequenceItem::slotSequenceChanged(quint32)
{
    prepareGeometryChange();
    calculateWidth();
}

void SequenceItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsItem::mousePressEvent(event);
    this->setSelected(true);
}

void SequenceItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsItem::mouseReleaseEvent(event);
    qDebug() << Q_FUNC_INFO << "mouse RELEASE event - <" << event->pos().toPoint().x() << "> - <" << event->pos().toPoint().y() << ">";
    setCursor(Qt::OpenHandCursor);
    emit itemDropped(event, this);
}


