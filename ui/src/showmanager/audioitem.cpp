/*
  Q Light Controller Plus
  audioitem.cpp

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

#include <QApplication>
#include <QPainter>
#include <qmath.h>
#include <QDebug>
#include <QMenu>

#include "audioitem.h"
#include "trackitem.h"
#include "audiodecoder.h"
#include "audioplugincache.h"

AudioItem::AudioItem(Audio *aud, ShowFunction *func)
    : ShowItem(func)
    , m_audio(aud)
    , m_previewLeftAction(NULL)
    , m_previewRightAction(NULL)
    , m_previewStereoAction(NULL)
    , m_preview(NULL)
{
    Q_ASSERT(aud != NULL);

    if (func->color().isValid())
        setColor(func->color());
    else
        setColor(ShowFunction::defaultColor(Function::AudioType));

    if (func->duration() == 0)
        func->setDuration(aud->totalDuration());

    calculateWidth();
    connect(m_audio, SIGNAL(changed(quint32)),
            this, SLOT(slotAudioChanged(quint32)));

    /* Preview actions */
    m_previewLeftAction = new QAction(tr("Preview Left Channel"), this);
    m_previewLeftAction->setCheckable(true);
    connect(m_previewLeftAction, SIGNAL(triggered(bool)),
            this, SLOT(slotAudioPreviewLeft()));
    m_previewRightAction = new QAction(tr("Preview Right Channel"), this);
    m_previewRightAction->setCheckable(true);
    connect(m_previewRightAction, SIGNAL(triggered(bool)),
            this, SLOT(slotAudioPreviewRight()));
    m_previewStereoAction = new QAction(tr("Preview Stereo Channels"), this);
    m_previewStereoAction->setCheckable(true);
    connect(m_previewStereoAction, SIGNAL(triggered(bool)),
            this, SLOT(slotAudioPreviewStereo()));
}

void AudioItem::calculateWidth()
{
    int newWidth = 0;
    qint64 audio_duration = m_audio->totalDuration();

    if (audio_duration != 0)
        newWidth = ((50/(float)getTimeScale()) * (float)audio_duration) / 1000;
    else
        newWidth = 100;

    if (newWidth < (50 / m_timeScale))
        newWidth = 50 / m_timeScale;
    setWidth(newWidth);
}

void AudioItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    float timeScale = 50/(float)m_timeScale;

    ShowItem::paint(painter, option, widget);

    if (m_preview != NULL)
    {
        // show preview here
        painter->drawPixmap(0, 0, m_preview->scaled(m_width, TRACK_HEIGHT - 4));
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

    ShowItem::postPaint(painter);
}

void AudioItem::setTimeScale(int val)
{
    ShowItem::setTimeScale(val);
    calculateWidth();
}

void AudioItem::setDuration(quint32 msec, bool stretch)
{
    Q_UNUSED(msec)
    Q_UNUSED(stretch)
    // nothing to do
}

QString AudioItem::functionName()
{
    if (m_audio)
        return m_audio->name();
    return QString();
}

Audio *AudioItem::getAudio()
{
    return m_audio;
}

void AudioItem::updateWaveformPreview()
{
    PreviewThread *waveformThread = new PreviewThread;
    waveformThread->setAudioItem(this);
    connect(waveformThread, SIGNAL(finished()), waveformThread, SLOT(deleteLater()));
    waveformThread->start();
}

void AudioItem::slotAudioChanged(quint32)
{
    updateWaveformPreview();
    prepareGeometryChange();
    calculateWidth();
    if (m_function)
        m_function->setDuration(m_audio->totalDuration());
}

void AudioItem::slotAudioPreviewLeft()
{
    m_previewRightAction->setChecked(false);
    m_previewStereoAction->setChecked(false);
    updateWaveformPreview();
}

void AudioItem::slotAudioPreviewRight()
{
    m_previewLeftAction->setChecked(false);
    m_previewStereoAction->setChecked(false);
    updateWaveformPreview();
}

void AudioItem::slotAudioPreviewStereo()
{
    m_previewLeftAction->setChecked(false);
    m_previewRightAction->setChecked(false);
    updateWaveformPreview();
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

    foreach (QAction *action, getDefaultActions())
        menu.addAction(action);

    menu.exec(QCursor::pos());
}

void PreviewThread::setAudioItem(AudioItem *item)
{
    m_item = item;
}

qint32 PreviewThread::getSample(unsigned char *data, quint32 idx, int sampleSize)
{
    qint32 value = 0;
    if (sampleSize == 1)
    {
        value = (qint32)data[idx];
    }
    else if (sampleSize == 2)
    {
        qint16 *array = (qint16 *)data;
        value = array[idx / 2];
    }
    else if (sampleSize == 3 || sampleSize == 4)
    {
        qint32 *array = (qint32 *)data;
        value = array[idx / 4] >> 16;
    }
    //qDebug() << "sampleValue:" << value;
    return value;
}

void PreviewThread::run()
{
    bool left = m_item->m_previewLeftAction->isChecked() || m_item->m_previewStereoAction->isChecked();
    bool right = m_item->m_previewRightAction->isChecked() || m_item->m_previewStereoAction->isChecked();

    if ((left || right) && m_item->m_audio->getAudioDecoder() != NULL)
    {
        AudioDecoder *ad = m_item->m_audio->doc()->audioPluginCache()->getDecoderForFile(m_item->m_audio->getSourceFileName());
        AudioParameters ap = ad->audioParameters();
        ad->seek(0);
        // 1- find out how many samples have to be represented on a single pixel on a 1:1 time scale
        int sampleSize = ap.sampleSize();
        int channels = ap.channels();
        int oneSecondSamples = ap.sampleRate() * channels;
        int onePixelSamples = oneSecondSamples / 50;

        qint32 maxValue = 0;
        // 24 and 32 bit samples would produce a RMS too high, so let's
        // work on 16bit values
        if (sampleSize > 2)
            sampleSize = 2;

        if (left && right)
            maxValue = 0x7F << (8 * (sampleSize - 1));
        else
            maxValue = 0x3F << (8 * (sampleSize - 1));

        quint32 onePixelReadLen = onePixelSamples * sampleSize;

        // 2- decode the whole file and fill a QPixmap with a sample block RMS value for each pixel
        qint64 dataRead = 1;
        unsigned char audioData[onePixelReadLen * 4];
        quint32 audioDataOffset = 0;
        QPixmap *preview = new QPixmap((50 * m_item->m_audio->totalDuration()) / 1000, 76);
        preview->fill(Qt::transparent);
        QPainter p(preview);
        int xpos = 0;

        qDebug() << "Audio duration:" << m_item->m_audio->totalDuration() <<
                    ", channels:" << channels << ", pixmap width:" << preview->width() <<
                    ", maxValue:" << maxValue << ", samples:" << sampleSize;
        qDebug() << "Samples per second:" << oneSecondSamples << ", for one pixel:" << onePixelSamples <<
                    ", onePixelReadLen:" << onePixelReadLen;

        delete m_item->m_preview;
        m_item->m_preview = NULL;
        m_item->update();

        while (dataRead)
        {
            quint32 tmpExceedData = 0;
            if (audioDataOffset < onePixelReadLen)
            {
                dataRead = ad->read((char *)audioData + audioDataOffset, onePixelReadLen * 2);
                if (dataRead > 0)
                {
                    if ((quint32)dataRead + audioDataOffset >= onePixelReadLen)
                    {
                        tmpExceedData = (dataRead + audioDataOffset) - onePixelReadLen;
                        dataRead = onePixelReadLen;
                    }
                    else
                    {
                        qDebug() << "Not enough data. Requested:" << onePixelReadLen << "got:" << dataRead;
                        audioDataOffset = dataRead;
                        continue;
                    }
                }
            }
            else
            {
                dataRead = onePixelReadLen;
                tmpExceedData = audioDataOffset - onePixelReadLen;
            }

            if (dataRead == onePixelReadLen)
            {
                quint32 i = 0;
                // calculate the RMS value (peak) for this data block
                qint64 rmsLeft = 0;
                qint64 rmsRight = 0;
                bool done = false;
                while (!done)
                {
                    if (left)
                    {
                        qint32 sampleVal = getSample(audioData, i, sampleSize);
                        rmsLeft += (sampleVal * sampleVal);
                    }
                    i += sampleSize;

                    if (channels == 2)
                    {
                        if (right)
                        {
                            qint32 sampleVal = getSample(audioData, i, sampleSize);
                            rmsRight += (sampleVal * sampleVal);
                        }
                        i += sampleSize;
                    }

                    if (i >= dataRead)
                    {
                        //qDebug() << "i:" << i << "xpos:" << xpos;
                        done = true;
                    }
                }

                if (left)
                    rmsLeft = sqrt(rmsLeft / onePixelSamples);
                if (right)
                    rmsRight = sqrt(rmsRight / onePixelSamples);
                //qDebug() << "sample" << i << "RMS right:" << rmsRight << ", RMS left:" << rmsLeft;

                // 3- Draw the actual waveform
                unsigned short lineHeightLeft = 0, lineHeightRight = 0;

                if (left)
                    lineHeightLeft = (76 * rmsLeft) / maxValue;
                if (right)
                    lineHeightRight = (76 * rmsRight) / maxValue;

                if (left && right)
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
                    unsigned short lineHeight = left ? lineHeightLeft : lineHeightRight;

                    if (lineHeight > 1)
                        p.drawLine(xpos, 38 - (lineHeight / 2), xpos, 38 + (lineHeight / 2));
                    else
                        p.drawLine(xpos, 38, xpos + 1, 38);
                    //qDebug() << "Data read: " << dataRead << ", rms: " << rmsRight << ", line height: " << lineHeight << ", xpos = " << xpos;
                }
                xpos++;

                if (tmpExceedData > 0)
                {
                    //qDebug() << "Exceed data found: " << tmpExceedData;
                    memmove(audioData, audioData + onePixelReadLen, tmpExceedData);
                    audioDataOffset = tmpExceedData;
                }
                else
                    audioDataOffset = 0;
            }
        }
        //qDebug() << "Iterations done: " << xpos;
        delete ad;
        m_item->m_preview = preview;
    }
    else // no preview selected. Delete pixmap
    {
        delete m_item->m_preview;
        m_item->m_preview = NULL;
    }

    m_item->update();
}
