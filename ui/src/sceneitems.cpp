/*
  Q Light Controller Plus
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
    , m_height(HEADER_HEIGHT)
    , m_timeStep(HALF_SECOND_WIDTH)
    , m_timeHit(2)
    , m_timeScale(3)
    , m_BPMValue(120)
    , m_type(Time)
{
}

void SceneHeaderItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsItem::mousePressEvent(event);
    emit itemClicked(event);
}

QRectF SceneHeaderItem::boundingRect() const
{
    return QRectF(0, 0, m_width, m_height);
}

void SceneHeaderItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    // draw base background
    painter->setPen(QPen(QColor(100, 100, 100, 255), 1));
    painter->setBrush(QBrush(QColor(150, 150, 150, 255)));
    painter->drawRect(0, 0, m_width, 35);

    if (m_type > Time)
        m_timeStep = ((float)(120 * HALF_SECOND_WIDTH) / (float)m_BPMValue) / (float)m_timeScale;

    // draw vertical timing lines and time labels
    int tmpSec = 0;
    for (int i = 0; i < m_width / m_timeStep; i++)
    {
        float xpos = ((float)i * m_timeStep) + 1;
        painter->setPen(QPen( QColor(250, 250, 250, 255), 1));
        if (i%m_timeHit == 0)
        {
            painter->drawLine(xpos, 20, xpos, 34);
            if (m_height > HEADER_HEIGHT)
            {
                painter->setPen(QPen(QColor(105, 105, 105, 255), 1));
                painter->drawLine(xpos, HEADER_HEIGHT, xpos, m_height);
            }
            painter->setPen(QPen( Qt::black, 1));
            if (m_type == Time)
            {
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
            {
                tmpSec++;
                painter->drawText(xpos - 4, 15, QString("%1").arg(tmpSec));
            }
        }
        else
        {
            if (m_timeStep > 5)
            {
                painter->drawLine(xpos, 25, xpos, 34);
                if (m_height > HEADER_HEIGHT)
                {
                    painter->setPen(QPen(QColor(105, 105, 105, 255), 1));
                    painter->drawLine(xpos, HEADER_HEIGHT, xpos, m_height);
                }
            }
        }
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

void SceneHeaderItem::setTimeDivisionType(SceneHeaderItem::TimeDivision type)
{
    if (type >= Invalid)
        return;

    m_type = type;
    if (m_type == Time)
    {
        m_timeStep = HALF_SECOND_WIDTH;
        m_timeHit = 2;
    }
    else
    {
        if (m_type == BPM_4_4)
            m_timeHit = 4;
        else if (m_type == BPM_3_4)
            m_timeHit = 3;
        else if (m_type == BPM_2_4)
            m_timeHit = 2;
    }
    update();
}

SceneHeaderItem::TimeDivision SceneHeaderItem::getTimeDivisionType()
{
    return m_type;
}

void SceneHeaderItem::setBPMValue(int value)
{
    if (value > 1)
    {
        m_BPMValue = value;
    }
    update();
}

int SceneHeaderItem::getHalfSecondWidth()
{
    return HALF_SECOND_WIDTH;
}

float SceneHeaderItem::getTimeDivisionStep()
{
    if (m_type > Time && m_timeStep <= 5)
        return m_timeStep * m_timeHit;
    return m_timeStep;
}

void SceneHeaderItem::setWidth(int w)
{
    prepareGeometryChange();
    m_width = w;
}

void SceneHeaderItem::setHeight(int h)
{
    prepareGeometryChange();
    m_height = h;
}

QString SceneHeaderItem::tempoToString(SceneHeaderItem::TimeDivision type)
{
    switch(type)
    {
        case Time: return QString("Time"); break;
        case BPM_4_4: return QString("BPM_4_4"); break;
        case BPM_3_4: return QString("BPM_3_4"); break;
        case BPM_2_4: return QString("BPM_2_4"); break;
        case Invalid:
        default:
            return QString("Invalid"); break;
    }
    return QString();
}

SceneHeaderItem::TimeDivision SceneHeaderItem::stringToTempo(QString tempo)
{
    if (tempo == "Time")
        return Time;
    else if (tempo == "BPM_4_4")
        return BPM_4_4;
    else if (tempo == "BPM_3_4")
        return BPM_3_4;
    else if (tempo == "BPM_2_4")
        return BPM_2_4;
    else
        return Invalid;
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

void SceneCursorItem::setHeight(int height)
{
    prepareGeometryChange();
    m_height = height;
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
    return QRectF(0, 0, TRACK_WIDTH - 4, TRACK_HEIGHT);
}

void TrackItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    // draw background gradient
    QLinearGradient linearGrad(QPointF(0, 0), QPointF(0, TRACK_HEIGHT));
    linearGrad.setColorAt(0, QColor(50, 64, 75, 255));
    //linearGrad.setColorAt(1, QColor(99, 127, 148, 255));
    linearGrad.setColorAt(1, QColor(76, 98, 115, 255));
    painter->setBrush(linearGrad);
    painter->drawRect(0, 0, TRACK_WIDTH - 4, TRACK_HEIGHT - 1);

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
    , m_selectedStep(-1)
    , m_pressed(false)
    , m_alignToCursor(NULL)
{
    Q_ASSERT(seq != NULL);
    setToolTip(QString(tr("Name: %1\nStart time: %2\nDuration: %3\n%4"))
              .arg(m_chaser->name())
              .arg(Function::speedToString(m_chaser->getStartTime()))
              .arg(Function::speedToString(m_chaser->getDuration()))
              .arg(tr("Click to move this sequence across the timeline")));

    setCursor(Qt::OpenHandCursor);
    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    m_color = m_chaser->getColor();
    calculateWidth();
    m_timeFont = QApplication::font();
    m_timeFont.setBold(true);
    m_timeFont.setPixelSize(12);
    connect(m_chaser, SIGNAL(changed(quint32)), this, SLOT(slotSequenceChanged(quint32)));

    m_alignToCursor = new QAction(tr("Align to cursor"), this);
    connect(m_alignToCursor, SIGNAL(triggered()),
            this, SLOT(slotAlignToCursorClicked()));

}

void SequenceItem::calculateWidth()
{
    int newWidth = 0;
    unsigned long seq_duration = 0;

    foreach (ChaserStep step, m_chaser->steps())
        seq_duration += step.duration;

    if (seq_duration != 0)
        newWidth = ((50/(float)m_timeScale) * (float)seq_duration) / 1000;

    if (newWidth < (50 / m_timeScale))
        newWidth = 50 / m_timeScale;
    m_width = newWidth;
}

int SequenceItem::getWidth()
{
    return m_width;
}

QPointF SequenceItem::getDraggingPos()
{
    return m_pos;
}

QRectF SequenceItem::boundingRect() const
{
    return QRectF(0, 0, m_width, TRACK_HEIGHT - 3);
}

void SequenceItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    float xpos = 0;
    float timeScale = 50/(float)m_timeScale;
    int stepIdx = 0;

    if (this->isSelected() == true)
    {
        painter->setPen(QPen(Qt::white, 3));
    }
    else
    {
        painter->setPen(QPen(Qt::white, 1));
        m_selectedStep = -1;
    }

    painter->setBrush(QBrush(m_color));
    painter->drawRect(0, 0, m_width, 77);

    /* draw vertical lines to divide the chaser's steps */
    foreach (ChaserStep step, m_chaser->steps())
    {
        if (step.fadeIn > 0)
        {
            int fadeXpos = xpos + ((timeScale * (float)step.fadeIn) / 1000);
            // doesn't even draw it if too small
            if (fadeXpos - xpos > 5)
            {
                painter->setPen(QPen(Qt::gray, 1));
                painter->drawLine(xpos, TRACK_HEIGHT - 4, fadeXpos, 1);
            }
        }
        float stepWidth = ((timeScale * (float)step.duration) / 1000);
        if (stepIdx == m_selectedStep)
        {
            painter->setPen(QPen(Qt::yellow, 2));
            painter->setBrush(QBrush(Qt::NoBrush));
            painter->drawRect(xpos, 0, stepWidth, TRACK_HEIGHT - 3);
        }
        xpos += stepWidth;

        painter->setPen(QPen(Qt::white, 1));
        painter->drawLine(xpos, 1, xpos, TRACK_HEIGHT - 5);
        if (step.fadeOut > 0)
        {
            int fadeXpos = xpos - ((timeScale * (float)step.fadeOut) / 1000);
            // doesn't even draw it if too small
            if (xpos - fadeXpos > 5)
            {
                painter->setPen(QPen(Qt::gray, 1));
                painter->drawLine(fadeXpos, 1, xpos, TRACK_HEIGHT - 4);
            }
        }
        stepIdx++;
    }

    if (m_pressed)
    {
        quint32 s_time = (double)(x() - TRACK_WIDTH - 2) * (m_timeScale * 500) /
                         (double)(HALF_SECOND_WIDTH);
        painter->setFont(m_timeFont);
        painter->drawText(5, TRACK_HEIGHT - 10, Function::speedToString(s_time));
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

void SequenceItem::setSelectedStep(int idx)
{
    m_selectedStep = idx;
    update();
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

void SequenceItem::slotAlignToCursorClicked()
{
    emit alignToCursor(this);
}

void SequenceItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsItem::mousePressEvent(event);
    m_pos = this->pos();
    if(event->button() == Qt::LeftButton)
        m_pressed = true;
    this->setSelected(true);
}

void SequenceItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsItem::mouseReleaseEvent(event);
    qDebug() << Q_FUNC_INFO << "mouse RELEASE event - <" << event->pos().toPoint().x() << "> - <" << event->pos().toPoint().y() << ">";
    setCursor(Qt::OpenHandCursor);
    m_pressed = false;
    emit itemDropped(event, this);
}

void SequenceItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *)
{
    QMenu menu;
    QFont menuFont = QApplication::font();
    menuFont.setPixelSize(14);
    menu.setFont(menuFont);

    menu.addAction(m_alignToCursor);
    menu.exec(QCursor::pos());
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
    , m_previewLeftAction(NULL)
    , m_previewRightAction(NULL)
    , m_previewStereoAction(NULL)
    , m_alignToCursor(NULL)
    , m_preview(NULL)
    , m_pressed(false)
{
    Q_ASSERT(aud != NULL);
    setToolTip(QString(tr("Name: %1\nStart time: %2\nDuration: %3\n%4"))
              .arg(m_audio->name())
              .arg(Function::speedToString(m_audio->getStartTime()))
              .arg(Function::speedToString(m_audio->getDuration()))
              .arg(tr("Click to move this audio across the timeline")));

    setCursor(Qt::OpenHandCursor);
    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    m_color = m_audio->getColor();

    m_font = QApplication::font();
    m_font.setBold(true);
    m_font.setPixelSize(12);

    calculateWidth();
    connect(m_audio, SIGNAL(changed(quint32)), this, SLOT(slotAudioChanged(quint32)));

    /* Preview actions */
    m_previewLeftAction = new QAction(tr("Preview Left Channel"), this);
    m_previewLeftAction->setCheckable(true);
    connect(m_previewLeftAction, SIGNAL(toggled(bool)),
            this, SLOT(slotAudioPreviewLeft(bool)));
    m_previewRightAction = new QAction(tr("Preview Right Channel"), this);
    m_previewRightAction->setCheckable(true);
    connect(m_previewRightAction, SIGNAL(toggled(bool)),
            this, SLOT(slotAudioPreviewRight(bool)));
    m_previewStereoAction = new QAction(tr("Preview Stereo Channels"), this);
    m_previewStereoAction->setCheckable(true);
    connect(m_previewStereoAction, SIGNAL(toggled(bool)),
            this, SLOT(slotAudioPreviewStero(bool)));

    m_alignToCursor = new QAction(tr("Align to cursor"), this);
    connect(m_alignToCursor, SIGNAL(triggered()),
            this, SLOT(slotAlignToCursorClicked()));
}

void AudioItem::calculateWidth()
{
    int newWidth = 0;
    qint64 audio_duration = m_audio->getDuration();

    if (audio_duration != 0)
        newWidth = ((50/(float)m_timeScale) * (float)audio_duration) / 1000;
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

QPointF AudioItem::getDraggingPos()
{
    return m_pos;
}

QRectF AudioItem::boundingRect() const
{
    return QRectF(0, 0, m_width, TRACK_HEIGHT - 3);
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

    painter->drawRect(0, 0, m_width, TRACK_HEIGHT - 3);

    painter->setFont(m_font);
    if (m_preview != NULL)
    {
        // show preview here
        QPixmap waveform = m_preview->scaled(m_width, TRACK_HEIGHT - 4);
        painter->drawPixmap(0, 0, waveform);
    }
    // draw shadow
    painter->setPen(QPen(QColor(10, 10, 10, 150), 2));
    painter->drawText(6, 16, m_audio->name());
    // draw track name
    painter->setPen(QPen(QColor(220, 220, 220, 255), 2));
    painter->drawText(5, 15, m_audio->name());

    if (m_pressed)
    {
        quint32 s_time = (double)(x() - TRACK_WIDTH - 2) * (m_timeScale * 500) /
                         (double)(HALF_SECOND_WIDTH);
        painter->drawText(5, TRACK_HEIGHT - 10, Function::speedToString(s_time));
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

void AudioItem::slotAudioPreviewLeft(bool active)
{
    m_previewRightAction->setChecked(false);
    m_previewStereoAction->setChecked(false);
    createWaveform(active, false);
}

void AudioItem::slotAudioPreviewRight(bool active)
{
    m_previewLeftAction->setChecked(false);
    m_previewStereoAction->setChecked(false);
    createWaveform(false, active);
}

void AudioItem::slotAudioPreviewStero(bool active)
{
    m_previewLeftAction->setChecked(false);
    m_previewRightAction->setChecked(false);
    createWaveform(active, active);
}

void AudioItem::slotAlignToCursorClicked()
{
    emit alignToCursor(this);
}

void AudioItem::createWaveform(bool left, bool right)
{
    if ((left == true || right == true) && m_audio->getAudioDecoder() != NULL)
    {
        AudioDecoder *ad = m_audio->getAudioDecoder();
        AudioParameters ap = ad->audioParameters();
        // 1- find out how many samples have to be represented on a single pixel on a 1:1 time scale
        int sampleSize = ap.sampleSize();
        int channels = ap.channels();
        int oneSecondSamples = ap.sampleRate() * channels;
        int onePixelSamples = oneSecondSamples / 50;
        //qint32 maxValue = qPow(0xFF, sampleSize);
        qint32 maxValue = 0;
        if (left == true && right == true)
            maxValue = 0x7F << (8 * (sampleSize - 1));
        else
            maxValue = 0x3F << (8 * (sampleSize - 1));
        quint32 defaultDataLen = onePixelSamples * sampleSize;

        // 2- decode the whole file and fill a QPixmap with a sample block RMS value for each pixel
        qint64 dataRead = 1;
        unsigned char audioData[defaultDataLen * 4];
        quint32 audioDataOffset = 0;
        m_preview = new QPixmap((50 * m_audio->getDuration()) / 1000, 76);
        m_preview->fill(Qt::transparent);
        QPainter p(m_preview);
        int xpos = 0;

        qDebug() << "Audio duration: " << m_audio->getDuration() <<
                    ", pixmap width: " << ((50 * m_audio->getDuration()) / 1000) <<
                    ", maxValue: " << maxValue;
        qDebug() << "Samples per second: " << oneSecondSamples << ", for one pixel: " << onePixelSamples;

        while (dataRead)
        {
            quint32 tmpExceedData = 0;
            if (audioDataOffset < defaultDataLen)
            {
                dataRead = ad->read((char *)audioData + audioDataOffset, defaultDataLen * 2);
                if (dataRead > 0)
                {
                    if(dataRead + audioDataOffset >= defaultDataLen)
                    {
                        tmpExceedData = dataRead + audioDataOffset - defaultDataLen;
                        dataRead = defaultDataLen;
                    }
                    else
                    {
                        audioDataOffset = dataRead;
                        continue;
                    }
                }
            }
            else
            {
                dataRead = defaultDataLen;
                tmpExceedData = audioDataOffset - defaultDataLen;
            }

            if (dataRead > 0)
            {
                quint32 i = 0;
                // calculate the RMS value (peak) for this data block
                double rmsLeft = 0;
                double rmsRight = 0;
                bool done = false;
                while (!done)
                {
                    if (left == true)
                    {
                        qint32 sampleVal = getSample(audioData, &i, sampleSize);
                        rmsLeft += (sampleVal * sampleVal);
                    }
                    if (channels == 2)
                    {
                        if (right == true)
                        {
                            qint32 sampleVal = getSample(audioData, &i, sampleSize);
                            rmsRight += (sampleVal * sampleVal);
                        }
                        else
                            getSample(audioData, &i, sampleSize); // got to read it anyway and discard data
                    }

                    if (i >= dataRead / sampleSize)
                        done = true;
                }
                quint32 divisor = (dataRead / sampleSize) / channels;
                if (left == true)
                    rmsLeft = sqrt(rmsLeft / divisor);
                if (right == true)
                    rmsRight = sqrt(rmsRight / divisor);

                // 3- Draw the actual waveform
                unsigned short lineHeightLeft = 0, lineHeightRight = 0;

                if (left == true)
                    lineHeightLeft = (76 * rmsLeft) / maxValue;
                if (right == true)
                    lineHeightRight = (76 * rmsRight) / maxValue;

                if (left == true && right == true)
                {
                    if (lineHeightLeft > 1)
                        p.drawLine(xpos, 19 - (lineHeightLeft / 2), xpos, 19 + (lineHeightLeft / 2));
                    else
                        p.drawLine(xpos, 19, xpos + 1, 19);

                    if (lineHeightRight > 1)
                        p.drawLine(xpos, 51 - (lineHeightRight / 2), xpos, 51 + (lineHeightRight / 2));
                    else
                        p.drawLine(xpos, 51, xpos + 1, 51);
                }
                else
                {
                    unsigned short lineHeight = 0;
                    if (left == true)
                        lineHeight = lineHeightLeft;
                    else
                        lineHeight = lineHeightRight;
                    if (lineHeight > 1)
                        p.drawLine(xpos, 38 - (lineHeight / 2), xpos, 38 + (lineHeight / 2));
                    else
                        p.drawLine(xpos, 38, xpos + 1, 38);
                    //qDebug() << "Data read: " << dataRead << ", rms: " << rms << ", line height: " << lineHeight << ", xpos = " << xpos;
                }
                xpos++;

                if (tmpExceedData > 0)
                {
                    //qDebug() << "Exceed data found: " << tmpExceedData;
                    memmove(audioData, audioData + defaultDataLen, tmpExceedData);
                    audioDataOffset = tmpExceedData;
                }
                else
                    audioDataOffset = 0;
            }
        }
        //qDebug() << "Iterations done: " << xpos;
        ad->seek(0);
    }
    else // no preview selected. Delete pixmap
    {
        delete m_preview;
        m_preview = NULL;
    }

    update();
}

void AudioItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsItem::mousePressEvent(event);
    m_pos = this->pos();
    if(event->button() == Qt::LeftButton)
        m_pressed = true;
    this->setSelected(true);
}

void AudioItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsItem::mouseReleaseEvent(event);
    qDebug() << Q_FUNC_INFO << "mouse RELEASE event - <" << event->pos().toPoint().x() << "> - <" << event->pos().toPoint().y() << ">";
    setCursor(Qt::OpenHandCursor);
    m_pressed = false;
    emit itemDropped(event, this);
}

void AudioItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *)
{
    QMenu menu;
    QFont menuFont = QApplication::font();
    menuFont.setPixelSize(14);
    menu.setFont(menuFont);

    if (m_audio->getAudioDecoder() != NULL)
    {
        AudioDecoder *ad = m_audio->getAudioDecoder();
        AudioParameters ap = ad->audioParameters();

        if (ap.channels() == 1)
            m_previewLeftAction->setText(tr("Preview Mono"));
        menu.addAction(m_previewLeftAction);
        if (ap.channels() == 2)
        {
            m_previewLeftAction->setText(tr("Preview Left Channel"));
            menu.addAction(m_previewRightAction);
            menu.addAction(m_previewStereoAction);
        }
        menu.addSeparator();
    }
    menu.addAction(m_alignToCursor);
    menu.exec(QCursor::pos());
}
