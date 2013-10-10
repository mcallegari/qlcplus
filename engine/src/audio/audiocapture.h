/*
  Q Light Controller Plus
  audiocapture.h

  Copyright (c) Massimo Callegari
  based on libbeat code by Maximilian Güntner

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

#ifndef AUDIOCAPTURE_H
#define AUDIOCAPTURE_H

#include <QThread>
#include <QMutex>

#define SETTINGS_AUDIO_INPUT_DEVICE  "audio/input"

#define FREQ_SUBBANDS_MAX_NUMBER        32
#define FREQ_SUBBANDS_DEFAULT_NUMBER    16
#define SPECTRUM_MAX_FREQUENCY          5000

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

    void setBandsNumber(int number);
    int bandsNumber();

    bool isInitialized();

    static int maxFrequency() { return SPECTRUM_MAX_FREQUENCY; }

    /*!
     * Prepares object for usage and setups required audio parameters.
     * Subclass should reimplement this function.
     * @param sampleSize Sample rate.
     * @param channels Number of channels.
     * @param bufferSize Audio dat buffer size\
     * @return initialization result (\b true - success, \b false - failure)
     */
    virtual bool initialize(unsigned int sampleRate, quint8 channels, quint16 bufferSize);

    /*!
     * Returns input interface latency in milliseconds.
     */
    virtual qint64 latency() = 0;

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

    void stop();

private:
    void processData();

    bool m_userStop, m_pause;

signals:
    void dataProcessed(double *spectrumBands, double maxMagnitude, quint32 power);

protected:
    /*!
     * Reads up to \b maxSize bytes from \b the input interface device.
     * Returns an array of the bytes read, or an empty array if an error occurred.
     * Subclass should reimplement this function.
     */
    virtual bool readAudio(int maxSize) = 0;

    QMutex m_mutex;
    bool m_isInitialized;

    unsigned int m_captureSize, m_sampleRate, m_channels;

    /** Data buffer for audio data coming from the sound card */
    int16_t *m_audioBuffer;

    quint32 m_signalPower;
    double m_maxMagnitude;
    int m_subBandsNumber;

    /** **************** FFT variables ********************** */
    double *m_fftInputBuffer;
    void *m_fftOutputBuffer;
    double m_fftMagnitudeBuffer[FREQ_SUBBANDS_MAX_NUMBER];
};

#endif // AUDIOCAPTURE_H
