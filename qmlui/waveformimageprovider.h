/*
  Q Light Controller Plus
  waveformimageprovider.h

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

#ifndef WAVEFORMIMAGEPROVIDER_H
#define WAVEFORMIMAGEPROVIDER_H

#include <QQuickImageProvider>
#include <QThread>
#include <QMutex>
#include <QImage>
#include <QHash>
#include <QSize>

class Doc;

/*********************************************************************
 * Worker that actually decodes the audio and builds the QImage
 *********************************************************************/
class WaveformWorker : public QObject
{
    Q_OBJECT
public:
    explicit WaveformWorker(Doc *doc, QObject *parent = nullptr);

public slots:
    void setPixelDensity(qreal density);
    void processWaveform(quint32 functionId, QSize requestedSize);

signals:
    void waveformReady(quint32 functionId, QImage image);

private:
    QImage generateWaveform(quint32 functionId, const QSize &requestedSize) const;
    static qint32 sampleAt(const unsigned char *data, quint32 idx, int sampleSize);

private:
    Doc *m_doc;
    qreal m_pixelDensity;
};

/*********************************************************************
 * Image provider: single interface used by QML
 * - QML calls requestImage()
 * - On cache miss, provider emits generateWaveform()
 * - Worker (in its own thread) emits waveformReady()
 * - Provider caches the image and emits waveformUpdated()
 *********************************************************************/
class WaveformImageProvider : public QQuickImageProvider
{
    Q_OBJECT
public:
    explicit WaveformImageProvider(Doc *doc);
    ~WaveformImageProvider() override;

    /** @reimp */
    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize) override;

    /** Clear the whole cache (e.g. when project changes) */
    void clearCache();

    void setPixelDensity(qreal density);
    qreal pixelDensity() const;

signals:
    /** Triggered when QML asks for an image that is not cached yet */
    void generateWaveform(quint32 functionId, QSize requestedSize);

    /** Emitted when a waveform is ready and stored in cache */
    void waveformUpdated(quint32 functionId);

public slots:
    /** Called by the worker when a waveform is ready */
    void handleWaveformReady(quint32 functionId, const QImage &image);

private:
    void ensureWorker();

private:
    Doc *m_doc;

    QThread        m_thread;
    WaveformWorker *m_worker;

    qreal m_pixelDensity;

    // mutable because requestImage is logically const
    mutable QMutex m_mutex;
    mutable QHash<quint32, QImage> m_cache;
};

#endif // WAVEFORMIMAGEPROVIDER_H
