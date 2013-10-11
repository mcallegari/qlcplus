/*
  Q Light Controller Plus
  audiocapture_portaudio.cpp

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

#include <QSettings>
#include <QDebug>

#include <portaudio.h>

#include "audiocapture_portaudio.h"

// Since only one instance of this class is allowed, I can
// afford to do this
static PaStream *stream = NULL;

AudioCapturePortAudio::AudioCapturePortAudio(QObject * parent)
    : AudioCapture(parent)
{

}

AudioCapturePortAudio::~AudioCapturePortAudio()
{
    PaError err;

    /* -- Now we stop the stream -- */
    err = Pa_StopStream( stream );
    if( err != paNoError )
        qDebug() << "PortAudio error: " << Pa_GetErrorText( err );

    /* -- don't forget to cleanup! -- */
    err = Pa_CloseStream( stream );
    if( err != paNoError )
        qDebug() << "PortAudio error: " << Pa_GetErrorText( err );

    err = Pa_Terminate();
    if( err != paNoError )
        qDebug() << "PortAudio error: " << Pa_GetErrorText( err );
}

bool AudioCapturePortAudio::initialize(unsigned int sampleRate, quint8 channels, quint16 bufferSize)
{
    PaError err;
    PaStreamParameters inputParameters;

    err = Pa_Initialize();
    if( err != paNoError )
        return false;

    QSettings settings;
    QVariant var = settings.value(SETTINGS_AUDIO_INPUT_DEVICE);
    if (var.isValid() == true)
        inputParameters.device = QString(var.toString()).toInt();
    else
        inputParameters.device = Pa_GetDefaultInputDevice(); /* default input device */

    if (inputParameters.device == paNoDevice)
    {
        qWarning("Error: No default input device found.\n");
        Pa_Terminate();
        return false;
    }

    inputParameters.channelCount = channels;
    inputParameters.sampleFormat = paInt16;
    inputParameters.suggestedLatency = Pa_GetDeviceInfo( inputParameters.device )->defaultLowInputLatency;
    inputParameters.hostApiSpecificStreamInfo = NULL;

    /* -- setup stream -- */
    err = Pa_OpenStream( &stream, &inputParameters, NULL, sampleRate, paFramesPerBufferUnspecified,
              paClipOff, /* we won't output out of range samples so don't bother clipping them */
              NULL, /* no callback, use blocking API */
              NULL ); /* no callback, so no callback userData */
    if( err != paNoError )
    {
        qWarning("Cannot open audio input stream (%s)\n",  Pa_GetErrorText(err));
        Pa_Terminate();
        return false;
    }

    /* -- start capture -- */
    err = Pa_StartStream( stream );
    if( err != paNoError )
    {
        qWarning("Cannot start stream capture (%s)\n",  Pa_GetErrorText(err));
        Pa_Terminate();
        return false;
    }

    return AudioCapture::initialize(sampleRate, channels, bufferSize);
}

qint64 AudioCapturePortAudio::latency()
{
    return 0; // TODO
}

void AudioCapturePortAudio::suspend()
{
}

void AudioCapturePortAudio::resume()
{
}

bool AudioCapturePortAudio::readAudio(int maxSize)
{
    int err;
    err = Pa_ReadStream( stream, m_audioBuffer, maxSize / 2 );
    if( err )
    {
        qWarning("read from audio interface failed (%s)\n", Pa_GetErrorText (err));
        return false;
    }

    qDebug() << "[PORTAUDIO readAudio] " << maxSize << "bytes read";

    return true;
}











