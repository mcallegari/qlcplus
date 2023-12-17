/*
  Q Light Controller Plus
  audiorenderer.h

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

#ifndef AUDIORENDERER_H
#define AUDIORENDERER_H

#include <QThread>
#include <QMutex>

#include "audiodecoder.h"

/** @addtogroup engine_audio Audio
 * @{
 */

#define AUDIO_CAP_INPUT     1
#define AUDIO_CAP_OUTPUT    2

#define SETTINGS_AUDIO_OUTPUT_DEVICE "audio/output"

typedef struct
{
    QString deviceName;
    QString privateName;
    int capabilities;
} AudioDeviceInfo;

class AudioRenderer : public QThread
{
    Q_OBJECT
public:
    /*!
     * Object contsructor.
     * @param parent Parent object.
     */
    AudioRenderer(QObject * parent = 0);

    ~AudioRenderer() { }

    void setDecoder(AudioDecoder *adec);
    /*!
     * Prepares object for usage and setups required audio parameters.
     * Subclass should reimplement this function.
     * @param freq Sample rate.
     * @param chan Number of channels.
     * @param format Audio format
     * @return initialization result (\b true - success, \b false - failure)
     */
    virtual bool initialize(quint32 freq, int chan, AudioFormat format) = 0;

    /*!
     * Returns output interface latency in milliseconds.
     */
    virtual qint64 latency() = 0;

    /*!
     * Writes all remaining plugin's internal data to audio output device.
     * Subclass should reimplement this function.
     */
    virtual void drain() = 0;

    /*!
     * Drops all plugin's internal data, resets audio device
     * Subclass should reimplement this function.
     */
    virtual void reset() = 0;

    /*!
     * Stops processing audio data, preserving buffered audio data.
     */
    virtual void suspend() = 0;

    /*!
     * Resumes processing audio data.
     */
    virtual void resume() = 0;

    void adjustIntensity(qreal fraction);

    /* Get/Set the looping flag */
    bool isLooped();
    void setLooped(bool looped);

private:
    bool m_looped;

    /*********************************************************************
     * Fade sequences
     *********************************************************************/
public:
    void setFadeIn(uint fadeTime);

    void setFadeOut(uint fadeTime);

private:
    qreal m_fadeStep;

    /*********************************************************************
     * Thread functions
     *********************************************************************/
public:
    /** @reimpl */
    virtual void run(); //thread run function

    void stop();

protected:
    /** State machine variables */
    bool m_userStop, m_pause;

private:
    /** Local copy of the audio function intensity */
    qreal m_intensity;
    qreal m_currentIntensity;

protected:
    /*!
     * Writes up to \b maxSize bytes from \b data to the output interface device.
     * Returns the number of bytes written, or -1 if an error occurred.
     * Subclass should reimplement this function.
     */
    virtual qint64 writeAudio(unsigned char *data, qint64 maxSize) = 0;

signals:
    void endOfStreamReached();

private:
    /** Reference to the decoder to be used as data source */
    AudioDecoder *m_adec;
    QMutex m_mutex;

    /** Data buffer for audio */
    unsigned char audioData[8 * 1024];
    qint64 audioDataRead;
    qint64 pendingAudioBytes;
};

/** @} */

#endif
