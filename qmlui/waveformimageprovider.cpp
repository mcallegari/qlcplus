/*
  Q Light Controller Plus
  waveformimageprovider.cpp

  Copyright (c) Massimo Callegari

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

/*
  Q Light Controller Plus
  waveformimageprovider.cpp
*/

#include <QPainter>
#include <QtMath>
#include <QDebug>

#include "waveformimageprovider.h"
#include "audioplugincache.h"
#include "audiodecoder.h"
#include "audio.h"
#include "doc.h"

/*********************************************************************
 * WaveformImageProvider
 *********************************************************************/

WaveformImageProvider::WaveformImageProvider(Doc *doc)
    : QQuickImageProvider(QQuickImageProvider::Image)
    , m_doc(doc)
    , m_worker(nullptr)
    , m_pixelDensity(1.0)
{
}

WaveformImageProvider::~WaveformImageProvider()
{
    if (m_thread.isRunning())
    {
        m_thread.quit();
        m_thread.wait();
    }
}

void WaveformImageProvider::ensureWorker()
{
    if (m_worker)
        return;

    m_worker = new WaveformWorker(m_doc);
    m_worker->moveToThread(&m_thread);

    connect(&m_thread, &QThread::finished,
            m_worker, &QObject::deleteLater);

    connect(this, &WaveformImageProvider::generateWaveform,
            m_worker, &WaveformWorker::processWaveform,
            Qt::QueuedConnection);

    connect(m_worker, &WaveformWorker::waveformReady,
            this, &WaveformImageProvider::handleWaveformReady,
            Qt::QueuedConnection);

    m_thread.start();

    // if density was already set before worker existed, push it now
    QMetaObject::invokeMethod(
        m_worker, "setPixelDensity",
        Qt::QueuedConnection,
        Q_ARG(qreal, m_pixelDensity));

    qDebug() << Q_FUNC_INFO << "worker/thread started. density:" << m_pixelDensity;
}

void WaveformImageProvider::setPixelDensity(qreal density)
{
    m_pixelDensity = density;

    // if worker is running, apply immediately
    if (m_worker)
    {
        QMetaObject::invokeMethod(
            m_worker, "setPixelDensity",
            Qt::QueuedConnection,
            Q_ARG(qreal, density));
    }
}

qreal WaveformImageProvider::pixelDensity() const
{
    return m_pixelDensity;
}

QImage WaveformImageProvider::requestImage(const QString &id,
                                           QSize *size,
                                           const QSize &requestedSize)
{
    //qDebug() << Q_FUNC_INFO << "requested id" << id << "requestedSize" << requestedSize;

    bool ok = false;
    quint32 fid = id.toUInt(&ok);
    if (!ok || m_doc == nullptr)
        return QImage();

    if (m_doc->function(fid)->type() != Function::AudioType)
        return QImage();

    // 1) Try cache first
    {
        QMutexLocker locker(&m_mutex);
        auto it = m_cache.constFind(fid);
        if (it != m_cache.constEnd())
        {
            //qDebug() << Q_FUNC_INFO << "cache hit for function" << fid << "image size" << it->size();

            if (size)
                *size = it->size();

            if (requestedSize.isValid() && !requestedSize.isEmpty())
                return it->scaled(requestedSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

            return *it;
        }
    }

    // 2) Not in cache: ask the internal worker to generate it
    ensureWorker();
    emit generateWaveform(fid, requestedSize);

    // 3) Return empty/placeholder image for now
    if (size)
        *size = requestedSize.isValid() ? requestedSize : QSize(1, 1);

    return QImage();
}

void WaveformImageProvider::handleWaveformReady(quint32 functionId,
                                                const QImage &image)
{
    qDebug() << Q_FUNC_INFO << "waveform ready for function" << functionId << "image size" << image.size();

    if (image.isNull())
        return;

    {
        QMutexLocker locker(&m_mutex);
        m_cache.insert(functionId, image);
    }

    emit waveformUpdated(functionId);
}

void WaveformImageProvider::clearCache()
{
    QMutexLocker locker(&m_mutex);
    m_cache.clear();
}

/*********************************************************************
 * WaveformWorker
 *********************************************************************/

WaveformWorker::WaveformWorker(Doc *doc, QObject *parent)
    : QObject(parent)
    , m_doc(doc)
    , m_pixelDensity(1.0)
{
}

void WaveformWorker::setPixelDensity(qreal density)
{
    m_pixelDensity = density;
}

void WaveformWorker::processWaveform(quint32 functionId, QSize requestedSize)
{
    if (m_doc == nullptr)
        return;

    QImage img = generateWaveform(functionId, requestedSize);

    if (!img.isNull())
        emit waveformReady(functionId, img);
}

qint32 WaveformWorker::sampleAt(const unsigned char *data,
                                quint32 idx,
                                int sampleSize)
{
    qint32 value = 0;
    if (sampleSize == 1)
    {
        value = static_cast<qint32>(data[idx]);
    }
    else if (sampleSize == 2)
    {
        const qint16 *array = reinterpret_cast<const qint16 *>(data);
        value = array[idx / 2];
    }
    else if (sampleSize == 3 || sampleSize == 4)
    {
        const qint32 *array = reinterpret_cast<const qint32 *>(data);
        value = array[idx / 4] >> 16;
    }
    return value;
}

QImage WaveformWorker::generateWaveform(quint32 functionId, const QSize &requestedSize) const
{
    Function *f = m_doc->function(functionId);
    if (f == nullptr || f->type() != Function::AudioType)
        return QImage();

    Audio *audio = qobject_cast<Audio *>(f);
    if (audio == nullptr || audio->getSourceFileName().isEmpty())
        return QImage();

    if (audio->getAudioDecoder() == nullptr)
        return QImage();

    AudioDecoder *ad = audio->doc()->audioPluginCache()->getDecoderForFile(audio->getSourceFileName());
    if (ad == nullptr)
        return QImage();

    ad->seek(0);

    AudioParameters ap = ad->audioParameters();
    int sampleSize = ap.sampleSize();
    int channels   = ap.channels();

    if (channels <= 0)
    {
        delete ad;
        return QImage();
    }

    const quint32 durationMs = audio->totalDuration();

    // Use DPI-based mapping:
    //  - 5 seconds = pixelDensity * 18 (Show manager tick)
    //  - height    = pixelDensity * 15 (same as ShowItem height)
    const qreal density = (m_pixelDensity > 0.0) ? m_pixelDensity : 1.0;
    const qreal pxPerSecond      = (density * 18.0) / 5.0;

    int width = qMax(1, qRound(pxPerSecond * (static_cast<qreal>(durationMs) / 1000.0)));

    int height;
    if (requestedSize.isValid() && requestedSize.height() > 0)
        height = requestedSize.height();
    else
        height = qMax(1, qRound(density * 15.0));

    const int oneSecondSamples = ap.sampleRate() * channels;
    int onePixelSamples = qMax(1.0, oneSecondSamples / pxPerSecond);

    if (sampleSize > 2)
        sampleSize = 2;

    qint32 maxValue = 0;
    if (channels == 2)
        maxValue = 0x7F << (8 * (sampleSize - 1));
    else
        maxValue = 0x3F << (8 * (sampleSize - 1));

    const quint32 onePixelReadLen = onePixelSamples * sampleSize;

    if (onePixelReadLen == 0)
    {
        delete ad;
        return QImage();
    }

    QImage img(width, height, QImage::Format_ARGB32_Premultiplied);
    img.fill(Qt::transparent);
    QPainter p(&img);
    p.setPen(QPen(Qt::black, 1));

    std::vector<unsigned char> audioData(onePixelReadLen * 4);
    quint32 audioDataOffset = 0;
    qint64 dataRead = 1;
    int xpos = 0;

    const int centerMono  = height / 2;
    const int centerLeft  = height / 4;
    const int centerRight = (3 * height) / 4;

    while (dataRead && xpos < width)
    {
        quint32 tmpExceedData = 0;

        if (audioDataOffset < onePixelReadLen)
        {
            dataRead = ad->read(reinterpret_cast<char *>(audioData.data()) + audioDataOffset, onePixelReadLen * 2);

            if (dataRead > 0)
            {
                if (static_cast<quint32>(dataRead) + audioDataOffset >= onePixelReadLen)
                {
                    tmpExceedData = (dataRead + audioDataOffset) - onePixelReadLen;
                    dataRead = onePixelReadLen;
                }
                else
                {
                    audioDataOffset += dataRead;
                    continue;
                }
            }
        }
        else
        {
            dataRead = onePixelReadLen;
            tmpExceedData = audioDataOffset - onePixelReadLen;
        }

        if (dataRead != static_cast<qint64>(onePixelReadLen))
            break;

        quint32 i = 0;
        qint64 rmsLeft = 0;
        qint64 rmsRight = 0;
        bool done = false;

        while (!done)
        {
            if (channels >= 1)
            {
                qint32 sampleVal = sampleAt(audioData.data(), i, sampleSize);
                rmsLeft += (sampleVal * sampleVal);
            }
            i += sampleSize;

            if (channels == 2)
            {
                qint32 sampleVal = sampleAt(audioData.data(), i, sampleSize);
                rmsRight += (sampleVal * sampleVal);
                i += sampleSize;
            }

            if (i >= static_cast<quint32>(dataRead))
                done = true;
        }

        if (channels >= 1 && onePixelSamples > 0)
            rmsLeft = qSqrt(static_cast<double>(rmsLeft) / onePixelSamples);
        if (channels == 2 && onePixelSamples > 0)
            rmsRight = qSqrt(static_cast<double>(rmsRight) / onePixelSamples);

        unsigned short lineHeightLeft  = 0;
        unsigned short lineHeightRight = 0;

        if (channels >= 1)
            lineHeightLeft = static_cast<unsigned short>((height * rmsLeft) / maxValue);
        if (channels == 2)
            lineHeightRight = static_cast<unsigned short>((height * rmsRight) / maxValue);

        if (channels == 2)
        {
            if (lineHeightLeft > 1)
                p.drawLine(xpos, centerLeft - (lineHeightLeft / 2),
                           xpos, centerLeft + (lineHeightLeft / 2));
            else
                p.drawLine(xpos, centerLeft, xpos + 1, centerLeft);

            if (lineHeightRight > 1)
                p.drawLine(xpos, centerRight - (lineHeightRight / 2),
                           xpos, centerRight + (lineHeightRight / 2));
            else
                p.drawLine(xpos, centerRight, xpos + 1, centerRight);
        }
        else
        {
            unsigned short lineHeight = lineHeightLeft;
            if (lineHeight > 1)
                p.drawLine(xpos, centerMono - (lineHeight / 2),
                           xpos, centerMono + (lineHeight / 2));
            else
                p.drawLine(xpos, centerMono, xpos + 1, centerMono);
        }

        xpos++;

        if (tmpExceedData > 0)
        {
            memmove(audioData.data(),
                    audioData.data() + onePixelReadLen,
                    tmpExceedData);
            audioDataOffset = tmpExceedData;
        }
        else
        {
            audioDataOffset = 0;
        }
    }

    delete ad;
    return img;
}
