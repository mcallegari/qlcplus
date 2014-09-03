/*
  Q Light Controller Plus
  showitems.cpp

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
#include <QtGui>
#include <QMenu>

#include "audiodecoder.h"
#include "headeritems.h"
#include "trackitem.h"
#include "showitems.h"
#include "chaserstep.h"

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
    , m_locked(false)
    , m_pressed(false)
    , m_alignToCursor(NULL)
    , m_lockAction(NULL)
{
    Q_ASSERT(seq != NULL);
    setToolTip(QString(tr("Name: %1\nStart time: %2\nDuration: %3\n%4"))
              .arg(m_chaser->name())
              .arg(Function::speedToString(m_chaser->getStartTime()))
              .arg(Function::speedToString(m_chaser->getDuration()))
              .arg(tr("Click to move this sequence across the timeline")));

    setCursor(Qt::OpenHandCursor);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    m_color = m_chaser->getColor();
    m_locked = m_chaser->isLocked();
    if (m_locked == true)
        setFlag(QGraphicsItem::ItemIsMovable, false);
    else
        setFlag(QGraphicsItem::ItemIsMovable, true);
    calculateWidth();
    m_font = qApp->font();
    m_font.setBold(true);
    m_font.setPixelSize(12);
    connect(m_chaser, SIGNAL(changed(quint32)),
            this, SLOT(slotSequenceChanged(quint32)));

    m_alignToCursor = new QAction(tr("Align to cursor"), this);
    connect(m_alignToCursor, SIGNAL(triggered()),
            this, SLOT(slotAlignToCursorClicked()));
    m_lockAction = new QAction(tr("Lock item"), this);
    connect(m_lockAction, SIGNAL(triggered()),
            this, SLOT(slotLockItemClicked()));
}

void SequenceItem::calculateWidth()
{
    int newWidth = 0;
    unsigned long seq_duration = 0;

    foreach (ChaserStep step, m_chaser->steps())
    {
        if (m_chaser->durationMode() == Chaser::Common)
            seq_duration += m_chaser->duration();
        else
            seq_duration += step.duration;
    }

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

    // draw chaser background
    painter->setBrush(QBrush(m_color));
    painter->drawRect(0, 0, m_width, 77);

    foreach (ChaserStep step, m_chaser->steps())
    {
        uint stepFadeIn = step.fadeIn;
        uint stepFadeOut = step.fadeOut;
        uint stepDuration = step.duration;
        if (m_chaser->fadeInMode() == Chaser::Common)
            stepFadeIn = m_chaser->fadeInSpeed();
        if (m_chaser->fadeOutMode() == Chaser::Common)
            stepFadeOut = m_chaser->fadeOutSpeed();
        if (m_chaser->durationMode() == Chaser::Common)
            stepDuration = m_chaser->duration();

        // draw fade in line
        if (stepFadeIn > 0)
        {
            int fadeXpos = xpos + ((timeScale * (float)stepFadeIn) / 1000);
            // doesn't even draw it if too small
            if (fadeXpos - xpos > 5)
            {
                painter->setPen(QPen(Qt::gray, 1));
                painter->drawLine(xpos, TRACK_HEIGHT - 4, fadeXpos, 1);
            }
        }
        float stepWidth = ((timeScale * (float)stepDuration) / 1000);
        // draw selected step
        if (stepIdx == m_selectedStep)
        {
            painter->setPen(QPen(Qt::yellow, 2));
            painter->setBrush(QBrush(Qt::NoBrush));
            painter->drawRect(xpos, 0, stepWidth, TRACK_HEIGHT - 3);
        }
        xpos += stepWidth;

        // draw step vertical delimiter
        painter->setPen(QPen(Qt::white, 1));
        painter->drawLine(xpos, 1, xpos, TRACK_HEIGHT - 5);

        // draw fade out line
        if (stepFadeOut > 0)
        {
            int fadeXpos = xpos + ((timeScale * (float)stepFadeOut) / 1000);
            // doesn't even draw it if too small
            if (fadeXpos - xpos > 5)
            {
                painter->setPen(QPen(Qt::gray, 1));
                painter->drawLine(xpos, 1, fadeXpos, TRACK_HEIGHT - 4);
            }
        }
        stepIdx++;
    }

    painter->setFont(m_font);
    // draw shadow
    painter->setPen(QPen(QColor(10, 10, 10, 150), 2));
    painter->drawText(QRect(4, 6, m_width - 6, 71), Qt::AlignLeft | Qt::TextWordWrap, m_chaser->name());

    // draw sequence name
    painter->setPen(QPen(QColor(220, 220, 220, 255), 2));
    painter->drawText(QRect(3, 5, m_width - 5, 72), Qt::AlignLeft | Qt::TextWordWrap, m_chaser->name());

    if (m_pressed)
    {
        quint32 s_time = 0;
        if (x() > TRACK_WIDTH)
            s_time = (double)(x() - TRACK_WIDTH) * (m_timeScale * 500) /
                     (double)(HALF_SECOND_WIDTH);
        painter->setFont(m_font);
        painter->drawText(3, TRACK_HEIGHT - 10, Function::speedToString(s_time));
    }

    if (m_locked)
        painter->drawPixmap(3, TRACK_HEIGHT >> 1, 24, 24, QIcon(":/lock.png").pixmap(24, 24));
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

void SequenceItem::setLocked(bool locked)
{
    m_locked = locked;
    m_chaser->setLocked(locked);
    setFlag(QGraphicsItem::ItemIsMovable, !locked);
}

bool SequenceItem::isLocked()
{
    return m_locked;
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

void SequenceItem::slotLockItemClicked()
{
    setLocked(!isLocked());
    //update();
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
    QFont menuFont = qApp->font();
    menuFont.setPixelSize(14);
    menu.setFont(menuFont);

    menu.addAction(m_alignToCursor);
    if (isLocked())
    {
        m_lockAction->setText(tr("Unlock item"));
        m_lockAction->setIcon(QIcon(":/unlock.png"));
    }
    else
    {
        m_lockAction->setText(tr("Lock item"));
        m_lockAction->setIcon(QIcon(":/lock.png"));
    }
    menu.addAction(m_lockAction);
    menu.exec(QCursor::pos());
}

/*********************************************************************
 *
 * Audio item
 *
 *********************************************************************/

AudioItem::AudioItem(Audio *aud)
    : m_color(100, 100, 100)
    , m_locked(false)
    , m_audio(aud)
    , m_width(50)
    , m_timeScale(3)
    , m_trackIdx(-1)
    , m_previewLeftAction(NULL)
    , m_previewRightAction(NULL)
    , m_previewStereoAction(NULL)
    , m_alignToCursor(NULL)
    , m_lockAction(NULL)
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
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    m_color = m_audio->getColor();
    m_locked = m_audio->isLocked();
    if (m_locked == true)
        setFlag(QGraphicsItem::ItemIsMovable, false);
    else
        setFlag(QGraphicsItem::ItemIsMovable, true);
    m_font = qApp->font();
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
            this, SLOT(slotAudioPreviewStereo(bool)));

    m_alignToCursor = new QAction(tr("Align to cursor"), this);
    connect(m_alignToCursor, SIGNAL(triggered()),
            this, SLOT(slotAlignToCursorClicked()));
    m_lockAction = new QAction(tr("Lock item"), this);
    connect(m_lockAction, SIGNAL(triggered()),
            this, SLOT(slotLockItemClicked()));
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

    float timeScale = 50/(float)m_timeScale;

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

    if (m_audio->fadeInSpeed() != 0)
    {
        int fadeXpos = (timeScale * (float)m_audio->fadeInSpeed()) / 1000;
        painter->setPen(QPen(Qt::gray, 1));
        painter->drawLine(1, TRACK_HEIGHT - 4, fadeXpos, 2);
    }

    if (m_audio->fadeOutSpeed() != 0)
    {
        int fadeXpos = (timeScale * (float)m_audio->fadeOutSpeed()) / 1000;
        painter->setPen(QPen(Qt::gray, 1));
        painter->drawLine(m_width - fadeXpos, 2, m_width - 1, TRACK_HEIGHT - 4);
    }

    // draw shadow
    painter->setPen(QPen(QColor(10, 10, 10, 150), 2));
    painter->drawText(QRect(4, 6, m_width - 6, 71), Qt::AlignLeft | Qt::TextWordWrap, m_audio->name());

    // draw audio name
    painter->setPen(QPen(QColor(220, 220, 220, 255), 2));
    painter->drawText(QRect(3, 5, m_width - 5, 72), Qt::AlignLeft | Qt::TextWordWrap, m_audio->name());

    if (m_pressed)
    {
        quint32 s_time = (double)(x() - TRACK_WIDTH - 2) * (m_timeScale * 500) /
                         (double)(HALF_SECOND_WIDTH);
        painter->drawText(3, TRACK_HEIGHT - 10, Function::speedToString(s_time));
    }

    if (m_locked)
        painter->drawPixmap(3, TRACK_HEIGHT >> 1, 24, 24, QIcon(":/lock.png").pixmap(24, 24));
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

void AudioItem::setLocked(bool locked)
{
    m_locked = locked;
    m_audio->setLocked(locked);
    setFlag(QGraphicsItem::ItemIsMovable, !locked);
}

bool AudioItem::isLocked()
{
    return m_locked;
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

void AudioItem::slotAudioPreviewStereo(bool active)
{
    m_previewLeftAction->setChecked(false);
    m_previewRightAction->setChecked(false);
    createWaveform(active, active);
}

void AudioItem::slotAlignToCursorClicked()
{
    emit alignToCursor(this);
}

void AudioItem::slotLockItemClicked()
{
    setLocked(!isLocked());
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
    QFont menuFont = qApp->font();
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
    if (isLocked())
    {
        m_lockAction->setText(tr("Unlock item"));
        m_lockAction->setIcon(QIcon(":/unlock.png"));
    }
    else
    {
        m_lockAction->setText(tr("Lock item"));
        m_lockAction->setIcon(QIcon(":/lock.png"));
    }
    menu.addAction(m_lockAction);

    menu.exec(QCursor::pos());
}

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
/*********************************************************************
 *
 * Video item
 *
 *********************************************************************/

VideoItem::VideoItem(Video *vid)
    : m_color(100, 100, 100)
    , m_locked(false)
    , m_video(vid)
    , m_width(50)
    , m_timeScale(3)
    , m_trackIdx(-1)
    , m_alignToCursor(NULL)
    , m_lockAction(NULL)
    , m_fullscreenAction(NULL)
    , m_pressed(false)
{
    Q_ASSERT(vid != NULL);
    setToolTip(QString(tr("Name: %1\nStart time: %2\nDuration: %3\n%4"))
              .arg(m_video->name())
              .arg(Function::speedToString(m_video->getStartTime()))
              .arg(Function::speedToString(m_video->getDuration()))
              .arg(tr("Click to move this video across the timeline")));

    setCursor(Qt::OpenHandCursor);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    m_color = m_video->getColor();
    m_locked = m_video->isLocked();
    if (m_locked == true)
        setFlag(QGraphicsItem::ItemIsMovable, false);
    else
        setFlag(QGraphicsItem::ItemIsMovable, true);
    m_font = qApp->font();
    m_font.setBold(true);
    m_font.setPixelSize(12);

    calculateWidth();
    connect(m_video, SIGNAL(changed(quint32)), this, SLOT(slotVideoChanged(quint32)));

    m_fullscreenAction = new QAction(tr("Fullscreen"), this);
    m_fullscreenAction->setCheckable(true);
    if (m_video->fullscreen() == true)
        m_fullscreenAction->setChecked(true);
    connect(m_fullscreenAction, SIGNAL(toggled(bool)),
            this, SLOT(slotFullscreenToggled(bool)));

    m_alignToCursor = new QAction(tr("Align to cursor"), this);
    connect(m_alignToCursor, SIGNAL(triggered()),
            this, SLOT(slotAlignToCursorClicked()));
    m_lockAction = new QAction(tr("Lock item"), this);
    connect(m_lockAction, SIGNAL(triggered()),
            this, SLOT(slotLockItemClicked()));
}

void VideoItem::calculateWidth()
{
    int newWidth = 0;
    qint64 video_duration = m_video->getDuration();

    if (video_duration != 0)
        newWidth = ((50/(float)m_timeScale) * (float)video_duration) / 1000;
    else
        newWidth = 100;

    if (newWidth < (50 / m_timeScale))
        newWidth = 50 / m_timeScale;
    m_width = newWidth;
}

int VideoItem::getWidth()
{
    return m_width;
}

QPointF VideoItem::getDraggingPos()
{
    return m_pos;
}

QRectF VideoItem::boundingRect() const
{
    return QRectF(0, 0, m_width, TRACK_HEIGHT - 3);
}

void VideoItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    float timeScale = 50/(float)m_timeScale;

    if (this->isSelected() == true)
        painter->setPen(QPen(Qt::white, 3));
    else
        painter->setPen(QPen(Qt::white, 1));
    painter->setBrush(QBrush(m_color));

    painter->drawRect(0, 0, m_width, TRACK_HEIGHT - 3);

    painter->setFont(m_font);

    if (m_video->fadeInSpeed() != 0)
    {
        int fadeXpos = (timeScale * (float)m_video->fadeInSpeed()) / 1000;
        painter->setPen(QPen(Qt::gray, 1));
        painter->drawLine(1, TRACK_HEIGHT - 4, fadeXpos, 2);
    }

    if (m_video->fadeOutSpeed() != 0)
    {
        int fadeXpos = (timeScale * (float)m_video->fadeOutSpeed()) / 1000;
        painter->setPen(QPen(Qt::gray, 1));
        painter->drawLine(m_width - fadeXpos, 2, m_width - 1, TRACK_HEIGHT - 4);
    }

    // draw shadow
    painter->setPen(QPen(QColor(10, 10, 10, 150), 2));
    painter->drawText(QRect(4, 6, m_width - 6, 71), Qt::AlignLeft | Qt::TextWordWrap, m_video->name());

    // draw audio name
    painter->setPen(QPen(QColor(220, 220, 220, 255), 2));
    painter->drawText(QRect(3, 5, m_width - 5, 72), Qt::AlignLeft | Qt::TextWordWrap, m_video->name());

    if (m_pressed)
    {
        quint32 s_time = (double)(x() - TRACK_WIDTH - 2) * (m_timeScale * 500) /
                         (double)(HALF_SECOND_WIDTH);
        painter->drawText(3, TRACK_HEIGHT - 10, Function::speedToString(s_time));
    }

    if (m_locked)
        painter->drawPixmap(3, TRACK_HEIGHT >> 1, 24, 24, QIcon(":/lock.png").pixmap(24, 24));
}

void VideoItem::updateDuration()
{
    setToolTip(QString(tr("Name: %1\nStart time: %2\nDuration: %3\n%4"))
              .arg(m_video->name())
              .arg(Function::speedToString(m_video->getStartTime()))
              .arg(Function::speedToString(m_video->getDuration()))
              .arg(tr("Click to move this video across the timeline")));
    prepareGeometryChange();
    calculateWidth();
}

void VideoItem::setTimeScale(int val)
{
    prepareGeometryChange();
    m_timeScale = val;
    calculateWidth();
}

void VideoItem::setTrackIndex(int idx)
{
    m_trackIdx = idx;
}

int VideoItem::getTrackIndex()
{
    return m_trackIdx;
}

void VideoItem::setColor(QColor col)
{
    m_color = col;
    update();
}

QColor VideoItem::getColor()
{
    return m_color;
}

void VideoItem::setLocked(bool locked)
{
    m_locked = locked;
    m_video->setLocked(locked);
    setFlag(QGraphicsItem::ItemIsMovable, !locked);
}

bool VideoItem::isLocked()
{
    return m_locked;
}

Video *VideoItem::getVideo()
{
    return m_video;
}

void VideoItem::slotVideoChanged(quint32)
{
    prepareGeometryChange();
    calculateWidth();
}

void VideoItem::slotAlignToCursorClicked()
{
    emit alignToCursor(this);
}

void VideoItem::slotLockItemClicked()
{
    setLocked(!isLocked());
}

void VideoItem::slotScreenChanged()
{
    QAction *action = (QAction *)sender();
    int scrIdx = action->data().toInt();

    m_video->setScreen(scrIdx);
}

void VideoItem::slotFullscreenToggled(bool toggle)
{
    m_video->setFullscreen(toggle);
}

void VideoItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsItem::mousePressEvent(event);
    m_pos = this->pos();
    if(event->button() == Qt::LeftButton)
        m_pressed = true;
    this->setSelected(true);
}

void VideoItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsItem::mouseReleaseEvent(event);
    qDebug() << Q_FUNC_INFO << "mouse RELEASE event - <" << event->pos().toPoint().x() << "> - <" << event->pos().toPoint().y() << ">";
    setCursor(Qt::OpenHandCursor);
    m_pressed = false;
    emit itemDropped(event, this);
}

void VideoItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *)
{
    QMenu menu;
    QFont menuFont = qApp->font();
    menuFont.setPixelSize(14);
    menu.setFont(menuFont);

    int screenCount = m_video->getScreenCount();
    if (screenCount > 0)
    {
        for (int i = 0; i < screenCount; i++)
        {
            QAction *scrAction = new QAction(tr("Screen %1").arg(i + 1), this);
            scrAction->setCheckable(true);
            if (m_video->screen() == i)
                scrAction->setChecked(true);
            scrAction->setData(i);
            connect(scrAction, SIGNAL(triggered()),
                    this, SLOT(slotScreenChanged()));
            menu.addAction(scrAction);
        }
    }
    menu.addAction(m_fullscreenAction);
    menu.addAction(m_alignToCursor);
    if (isLocked())
    {
        m_lockAction->setText(tr("Unlock item"));
        m_lockAction->setIcon(QIcon(":/unlock.png"));
    }
    else
    {
        m_lockAction->setText(tr("Lock item"));
        m_lockAction->setIcon(QIcon(":/lock.png"));
    }
    menu.addAction(m_lockAction);

    menu.exec(QCursor::pos());
}
#endif
