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

/* *************************************************************************************** */

SceneHeaderItem::SceneHeaderItem(int w)
    : width(w)
{
    timeScale = 1;
    timeStep = 25;
}

void SceneHeaderItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsItem::mousePressEvent(event);
    emit itemClicked(event);
}

QRectF SceneHeaderItem::boundingRect() const
{
    return QRectF(0, 0, width, 35);
}

void SceneHeaderItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    // draw base background
    painter->setPen(QPen(QColor(100, 100, 100, 255), 1));
    painter->setBrush(QBrush(QColor(150, 150, 150, 255)));
    painter->drawRect(0, 0, width, 35);

    // draw vertical timing lines and time labels
    int tmpSec = 0;
    for (int i = 0; i < width / timeStep; i++)
    {
        int xpos = (i * timeStep) + 1;
        painter->setPen(QPen( QColor(250, 250, 250, 255), 1));
        if (i%2 == 0)
        {
            painter->drawLine(xpos, 20, xpos, 34);
            painter->setPen(QPen( Qt::black, 1));
            tmpSec = (i/2) * timeScale;
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
    timeScale = val;
    update();
}

int SceneHeaderItem::getTimeScale()
{
    return timeScale;
}

int SceneHeaderItem::getTimeStep()
{
    return timeStep;
}

/**********************************************************************************************************/

SceneCursorItem::SceneCursorItem(int h)
    : height(h)
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
    return QRectF(-5, 0, 10, height);
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
    painter->drawRect(0, 35, 1, height - 35);
}

/**********************************************************************************************************/

TrackItem::TrackItem(int number)
    : trackNumber(number)
{
    m_font = QApplication::font();
    m_font.setBold(true);
    m_font.setPixelSize(18);
    trackName = QString("Track %1").arg(trackNumber);
}

int TrackItem::getTrackNumber()
{
    return trackNumber;
}

QRectF TrackItem::boundingRect() const
{
    return QRectF(0, 0, 146, 80);
}

void TrackItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    QLinearGradient linearGrad(QPointF(0, 0), QPointF(0, 80));
    linearGrad.setColorAt(0, QColor(200, 200, 200, 255));
    linearGrad.setColorAt(1, QColor(100, 100, 100, 255));

    painter->setBrush(linearGrad);
    painter->drawRect(0, 0, 146, 80);
    painter->setPen(QPen(QColor(200, 200, 200, 255), 2));

    painter->setFont(m_font);
    painter->drawText(5, 70, trackName);
}


/**********************************************************************************************************/

SequenceItem::SequenceItem(Chaser *seq)
    : color(qrand() % 256, qrand() % 256, qrand() % 256)
    , chaser(seq)
{
    setToolTip(QString("Start time: %1msec\n%2")
              .arg(seq->getStartTime()).arg("Click to move this sequence across the timeline"));
    setCursor(Qt::OpenHandCursor);
    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    timeScale = 1;
    seq_width = 50;
    connect(chaser, SIGNAL(changed(quint32)), this, SLOT(slotSequenceChanged(quint32)));
}

QRectF SequenceItem::boundingRect() const
{
    return QRectF(0, 0, seq_width, 77);
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
    unsigned long seq_duration = 0;
    foreach (ChaserStep step, chaser->steps())
        seq_duration += step.duration;

    if (seq_duration != 0)
    {
        seq_width = ((50/timeScale) * seq_duration) / 1000;
        if (seq_width < (50 / timeScale))
            seq_width = 50 / timeScale;
    }
    painter->drawRect(0, 0, seq_width, 77);
    /* draw vertical lines to show the chaser's steps */
    int xpos = 0;
    painter->setPen(QPen(Qt::white, 1));
    foreach (ChaserStep step, chaser->steps())
    {
        xpos += (((50/timeScale) * step.duration) / 1000);
        painter->drawLine(xpos, 1, xpos, 75);
    }

}

void SequenceItem::setTimeScale(int val)
{
    timeScale = val;
    update();
}

Chaser *SequenceItem::getChaser()
{
    return chaser;
}

void SequenceItem::slotSequenceChanged(quint32)
{
    //qDebug() << Q_FUNC_INFO << " step added !!!";
    update();
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
/*
void SequenceItem::focusOutEvent ( QFocusEvent * event )
{
    emit itemSelected(false);
}
*/
#if 0
void SequenceItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() != Qt::LeftButton) {
        event->ignore();
        return;
    }

    setCursor(Qt::ClosedHandCursor);
}

void SequenceItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (QLineF(event->screenPos(), event->buttonDownScreenPos(Qt::LeftButton))
        .length() < QApplication::startDragDistance()) {
        return;
    }

    QDrag *drag = new QDrag(event->widget());
    QMimeData *mime = new QMimeData;
    drag->setMimeData(mime);
/*
    mime->setColorData(color);
    mime->setText(QString("#%1%2%3")
                    .arg(color.red(), 2, 16, QLatin1Char('0'))
                    .arg(color.green(), 2, 16, QLatin1Char('0'))
                    .arg(color.blue(), 2, 16, QLatin1Char('0')));

    QPixmap pixmap(34, 34);
    pixmap.fill(Qt::white);

    QPainter painter(&pixmap);
    painter.translate(15, 15);
    painter.setRenderHint(QPainter::Antialiasing);
    paint(&painter, 0, 0);
    painter.end();

    pixmap.setMask(pixmap.createHeuristicMask());

    drag->setPixmap(pixmap);
    drag->setHotSpot(QPoint(15, 20));
*/
    drag->exec( Qt::CopyAction );
    setCursor(Qt::OpenHandCursor);
}
#endif

