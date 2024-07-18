/*
  Q Light Controller Plus
  audiocapture.h

  Copyright (c) Massimo Callegari
  based on libbeat code by Maximilian Güntner

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

#ifndef AUDIOCAPTURE_H
#define AUDIOCAPTURE_H

#include <stdint.h>
#include <QThread>
#include <QVector>
#include <QMutex>
#include <QMap>

#define SETTINGS_AUDIO_INPUT_DEVICE   "audio/input"
#define SETTINGS_AUDIO_INPUT_SRATE    "audio/samplerate"
#define SETTINGS_AUDIO_INPUT_CHANNELS "audio/channels"

#define AUDIO_DEFAULT_SAMPLE_RATE     44100
#define AUDIO_DEFAULT_CHANNELS        1
#define AUDIO_DEFAULT_BUFFER_SIZE     2048 // bytes per channel

#define FREQ_SUBBANDS_MAX_NUMBER        32
#define FREQ_SUBBANDS_DEFAULT_NUMBER    16
#define SPECTRUM_MAX_FREQUENCY          5000

/** @addtogroup engine_audio Audio
 * @{
 */

struct BandsData
{
    int m_registerCounter;
    QVector<double> m_fftMagnitudeBuffer;
};

class AudioCapture : public QThread
{
    Q_OBJECT
public:
    /*!
     * Object contsructor.
     * @param parent Parent object.
     */
    AudioCapture(QObject* parent = 0);

    ~AudioCapture();

    int defaultBarsNumber();

    /**
     * Request the given number of frequency bands to the
     * audiocapture engine
     */
    void registerBandsNumber(int number);

    /**
     * Cancel a previous request of bars
     */
    void unregisterBandsNumber(int number);
    //int bandsNumber();

    static int maxFrequency() { return SPECTRUM_MAX_FREQUENCY; }

    protected:
    /*!
     * Prepares object for usage and setups required audio parameters.
     * Subclass should reimplement this function.
     * @param sampleSize Sample rate.
     * @param channels Number of channels.
     * @param bufferSize Audio dat buffer size\
     * @return initialization result (\b true - success, \b false - failure)
     */
    virtual bool initialize() = 0;

    virtual void uninitialize() = 0;

public:
    /*!
     * Returns input interface latency in milliseconds.
     */
    virtual qint64 latency() = 0;

    /*!
     *  Adjusts the audio output volume
     */
    virtual void setVolume(qreal volume) = 0;

    /*!
     * Stops processing audio data, preserving buffered audio data.
     */
    virtual void suspend() = 0;

    /*!
     * Resumes processing audio data.
     */
    virtual void resume() = 0;

    /*********************************************************************
     * Thread functions
     *********************************************************************/
    /** @reimpl */
    void run(); //thread run function

protected:
    void stop();

private:
    /** This is called at every processData to fill a single BandsData structure */
    double fillBandsData(int number);

    /** This is the method where captured audio data is processed in this order
     *  1) calculates the signal power, which will be the volume bar
     *  2) perform the FFT
     *  3) retrieve the signal magnitude for each registered number of bands
     */
    void processData();

    bool m_userStop, m_pause;

signals:
    void dataProcessed(double *spectrumBands, int size, double maxMagnitude, quint32 power);
    void volumeChanged(int volume);

protected:
    /*!
     * Reads up to \b maxSize uint16 from \b the input interface device.
     * Returns an array of the bytes read, or an empty array if an error occurred.
     * Subclass should reimplement this function.
     */
    virtual bool readAudio(int maxSize) = 0;

    QMutex m_mutex;

    unsigned int bufferSize, m_captureSize, m_sampleRate, m_channels;

    /** Data buffer for audio data coming from the sound card */
    int16_t *m_audioBuffer;
    int16_t *m_audioMixdown;

    quint32 m_signalPower;

    /** **************** FFT variables ********************** */
    double *m_fftInputBuffer;
    void *m_fftOutputBuffer;

    /** Map of the registered clients (key is the number of bands) */
    QMap <int, BandsData> m_fftMagnitudeMap;
};

/** @} */

#endif // AUDIOCAPTURE_H
