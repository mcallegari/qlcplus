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
#include <QMenu>

#include "audiodecoder.h"
#include "sceneitems.h"
#include "chaserstep.h"

/****************************************************************************
 *
 * Header item
 *
 ****************************************************************************/
SceneHeaderItem::SceneHeaderItem(int w)
    : m_width(w)
    , m_timeStep(25)
    , m_timeScale(3)
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

void SceneHeaderItem::setWidth(int w)
{
    prepareGeometryChange();
    m_width = w;
}


/****************************************************************************
 *
 * Cursor item
 *
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
 *
 * Track item
 *
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

    m_soloRegion = new QRectF(15.0, 10.0, 25.0, 16.0);
    m_muteRegion = new QRectF(42.0, 10.0, 25.0, 16.0);
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
    linearGrad.setColorAt(0, QColor(50, 64, 75, 255));
    //linearGrad.setColorAt(1, QColor(99, 127, 148, 255));
    linearGrad.setColorAt(1, QColor(76, 98, 115, 255));
    painter->setBrush(linearGrad);
    painter->drawRect(0, 0, 146, 79);

    // Draw left bar that shows if the track is active or not
    painter->setPen(QPen(QColor(48, 61, 72, 255), 1));
    if (m_isActive == true)
        painter->setBrush(QBrush(QColor(0, 255, 0, 255)));
    else
        painter->setBrush(QBrush(QColor(129, 145, 160, 255)));
    painter->drawRect(1, 1, 7, 40);

    // draw solo button
    if (m_isSolo)
        painter->setBrush(QBrush(QColor(255, 255, 0, 255)));
    else
        painter->setBrush(QBrush(QColor(129, 145, 160, 255)));
    painter->drawRoundedRect(m_soloRegion->toRect(), 3.0, 3.0);
    painter->setFont(m_btnFont);
    painter->drawText(23, 23, "S");

    // draw mute button
    if (m_isMute)
        painter->setBrush(QBrush(QColor(255, 0, 0, 255)));
    else
        painter->setBrush(QBrush(QColor(129, 145, 160, 255)));
    painter->drawRoundedRect(m_muteRegion->toRect(), 3.0, 3.0);
    painter->drawText(48, 23, "M");

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
 *
 * Sequence item
 *
 *********************************************************************/

SequenceItem::SequenceItem(Chaser *seq)
    : m_color(100, 100, 100)
    , m_chaser(seq)
    , m_width(50)
    , m_timeScale(3)
    , m_trackIdx(-1)
{
    Q_ASSERT(seq != NULL);
    setToolTip(QString("Start time: %1\n%2")
              .arg(Function::speedToString(seq->getStartTime())).arg("Click to move this sequence across the timeline"));
    setCursor(Qt::OpenHandCursor);
    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    m_color = m_chaser->getColor();
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

int SequenceItem::getWidth()
{
    return m_width;
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
    painter->setBrush(QBrush(m_color));

    painter->drawRect(0, 0, m_width, 77);
    /* draw vertical lines to show the chaser's steps */
    int xpos = 0;

    foreach (ChaserStep step, m_chaser->steps())
    {

        if (step.fadeIn > 0)
        {
            int fadeXpos = xpos + (((50/m_timeScale) * step.fadeIn) / 1000);
            // doesn't even draw it if too small
            if (fadeXpos - xpos > 5)
            {
                painter->setPen(QPen(Qt::gray, 1));
                painter->drawLine(xpos, 76, fadeXpos, 1);
            }
        }
        painter->setPen(QPen(Qt::white, 1));
        xpos += (((50/m_timeScale) * step.duration) / 1000);
        painter->drawLine(xpos, 1, xpos, 75);
        if (step.fadeOut > 0)
        {
            int fadeXpos = xpos - (((50/m_timeScale) * step.fadeOut) / 1000);
            // doesn't even draw it if too small
            if (xpos - fadeXpos > 5)
            {
                painter->setPen(QPen(Qt::gray, 1));
                painter->drawLine(fadeXpos, 1, xpos, 76);
            }
        }
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

void SequenceItem::setColor(QColor col)
{
    m_color = col;
    update();
}

QColor SequenceItem::getColor()
{
    return m_color;
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


/*********************************************************************
 *
 * Audio item
 *
 *********************************************************************/

AudioItem::AudioItem(Audio *aud)
    : m_color(100, 100, 100)
    , m_audio(aud)
    , m_width(50)
    , m_timeScale(3)
    , m_trackIdx(-1)
    , m_previewAction(NULL)
    , m_preview(NULL)
{
    Q_ASSERT(aud != NULL);
    setToolTip(QString("Start time: %1\n%2")
              .arg(Function::speedToString(m_audio->getStartTime())).arg("Click to move this object across the timeline"));
    setCursor(Qt::OpenHandCursor);
    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    m_color = m_audio->getColor();

    m_font = QApplication::font();
    m_font.setBold(true);
    m_font.setPixelSize(12);

    calculateWidth();
    connect(m_audio, SIGNAL(changed(quint32)), this, SLOT(slotAudioChanged(quint32)));

    /* Edit actions */
    m_previewAction = new QAction(tr("&Preview"), this);
    m_previewAction->setCheckable(true);
    connect(m_previewAction, SIGNAL(toggled(bool)),
            this, SLOT(slotAudioPreview(bool)));
}

void AudioItem::calculateWidth()
{
    int newWidth = 0;
    qint64 audio_duration = m_audio->getDuration();

    if (audio_duration != 0)
        newWidth = ((50/m_timeScale) * audio_duration) / 1000;
    else
        newWidth = 100;

    if (newWidth < (50 / m_timeScale))
        newWidth = 50 / m_timeScale;
    m_width = newWidth;
}

int AudioItem::getWidth()
{
    return m_width;
}

QRectF AudioItem::boundingRect() const
{
    return QRectF(0, 0, m_width, 77);
}

void AudioItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    if (this->isSelected() == true)
        painter->setPen(QPen(Qt::white, 3));
    else
        painter->setPen(QPen(Qt::white, 1));
    painter->setBrush(QBrush(m_color));

    painter->drawRect(0, 0, m_width, 77);

    painter->setFont(m_font);
    // draw shadow
    painter->setPen(QPen(QColor(10, 10, 10, 150), 2));
    painter->drawText(6, 16, m_audio->name());
    // draw track name
    painter->setPen(QPen(QColor(220, 220, 220, 255), 2));
    painter->drawText(5, 15, m_audio->name());
    if (m_previewAction->isChecked() && m_preview != NULL)
    {
        // show preview here
        QPixmap waveform = m_preview->scaled(m_width, 76);
        painter->drawPixmap(0, 0, waveform);
    }
}

void AudioItem::updateDuration()
{
    prepareGeometryChange();
    calculateWidth();
}

void AudioItem::setTimeScale(int val)
{
    prepareGeometryChange();
    m_timeScale = val;
    calculateWidth();
}

void AudioItem::setTrackIndex(int idx)
{
    m_trackIdx = idx;
}

int AudioItem::getTrackIndex()
{
    return m_trackIdx;
}

void AudioItem::setColor(QColor col)
{
    m_color = col;
    update();
}

QColor AudioItem::getColor()
{
    return m_color;
}

Audio *AudioItem::getAudio()
{
    return m_audio;
}

void AudioItem::slotAudioChanged(quint32)
{
    prepareGeometryChange();
    calculateWidth();
}

qint32 AudioItem::getSample(unsigned char *data, quint32 *idx, int sampleSize)
{
    qint32 value = 0;
    if (sampleSize == 1)
        value = (qint32)data[(*idx)++];
    if (sampleSize == 2)
    {
        qint16 *tmpdata = (qint16 *)data;
        qint16 twobytes = tmpdata[*idx];
        value = twobytes;
        *idx+=2;
    }
    else if (sampleSize == 4)
    {
        value = (value + ((qint32)data[*idx] << 24) + ((qint32)data[*idx + 1] << 16) + ((qint32)data[*idx + 2] << 8));
        *idx+=3;
    }
    return value;
}

void AudioItem::slotAudioPreview(bool active)
{
    qDebug() << Q_FUNC_INFO << "Toggled: " << active;
    if (active == true && m_audio->getAudioDecoder() != NULL)
    {
        AudioDecoder *ad = m_audio->getAudioDecoder();
        AudioParameters ap = ad->audioParameters();
        // 1- find out how many samples have to be represented on a single pixel on a 1:1 time scale
        int sampleSize = ap.sampleSize();
        int channels = ap.channels();
        int oneSecondSamples = ap.sampleRate() * sampleSize * channels;
        int onePixelSamples = oneSecondSamples / 50;
        //qint32 maxValue = qPow(0xFF, sampleSize);
        qint32 maxValue = 0x7F << (8 * (sampleSize - 1));

        // 2- decode the whole file and fill a QPixmap with a sample block RMS value for each pixel
        qint64 dataRead = 1;
        unsigned char audioData[onePixelSamples * sampleSize];
        m_preview = new QPixmap((50 * m_audio->getDuration()) / 1000, 76);
        m_preview->fill(Qt::transparent);
        QPainter p(m_preview);
        int xpos = 0;

        qDebug() << "Samples per second: " << oneSecondSamples << ", for one pixel: " << onePixelSamples;

        while (dataRead)
        {
            dataRead = ad->read((char *)audioData, onePixelSamples * sampleSize);
            if (dataRead > 0)
            {
                quint32 i = 0;
                // calculate the RMS value (peak) for this data block
                double rms = 0;
                bool done = false;
                while (!done)
                {
                    qint32 sampleVal = getSample(audioData, &i, sampleSize);
                    //qDebug() << "sample #" << i << ": " << sampleVal;
                    if (channels == 2)
                    {
                        //qint32 evenSampleVal = getSample(audioData, &i, sampleSize);
                        getSample(audioData, &i, sampleSize);
                        //sampleVal = (sampleVal + evenSampleVal) / 2;
                    }
                    rms += (sampleVal * sampleVal);

                    if (i >= dataRead / sampleSize)
                        done = true;
                }
                rms = sqrt(rms/((dataRead / sampleSize) / channels));

                qint32 lineHeight = (76 * rms) / maxValue;
                if (lineHeight > 0)
                    p.drawLine(xpos, 38 - (lineHeight / 2), xpos, 38 + (lineHeight / 2));
                else
                    p.drawLine(xpos, 38, xpos + 1, 38);
                //qDebug() << "Data read: " << dataRead << ", rms: " << rms << ", line height: " << lineHeight << ", xpos = " << xpos;
                xpos++;
            }
        }


        ad->seek(0);
    }
    update();
}

void AudioItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsItem::mousePressEvent(event);
    this->setSelected(true);
}

void AudioItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsItem::mouseReleaseEvent(event);
    qDebug() << Q_FUNC_INFO << "mouse RELEASE event - <" << event->pos().toPoint().x() << "> - <" << event->pos().toPoint().y() << ">";
    setCursor(Qt::OpenHandCursor);
    emit itemDropped(event, this);
}

void AudioItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *)
{
    /*
    QMenu menu;
    menu.addAction(m_previewAction);

    menu.exec(QCursor::pos());
    */
}
