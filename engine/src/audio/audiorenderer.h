/*
  Q Light Controller Plus
  audiorenderer.h

  Copyright (c) Massimo Callegari

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

#ifndef AUDIORENDERER_H
#define AUDIORENDERER_H

#include <QThread>
#include <QMutex>

#include "audiodecoder.h"

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

    /*********************************************************************
     * Thread functions
     *********************************************************************/
    /** @reimpl */
    void run(); //thread run function

    void stop();

private:

    bool m_userStop, m_pause;
    qreal m_intensity;

protected:
    /*!
     * Writes up to \b maxSize bytes from \b data to the output interface device.
     * Returns the number of bytes written, or -1 if an error occurred.
     * Subclass should reimplement this function.
     */
    virtual qint64 writeAudio(unsigned char *data, qint64 maxSize) = 0;

private:
    /** Reference to the decoder to be used as data source */
    AudioDecoder *m_adec;
    QMutex m_mutex;

    /** Data buffer for audio */
    unsigned char audioData[8 * 1024];
    qint64 audioDataRead;
    qint64 pendingAudioBytes;
};

#endif
