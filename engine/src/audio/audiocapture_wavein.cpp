/*
  Q Light Controller Plus
  audiocapture_wavein.cpp

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

#include <QDebug>
#include <windows.h>

#include "audiocapture_wavein.h"

static HWAVEIN deviceHandle;

AudioCaptureWaveIn::AudioCaptureWaveIn(QObject * parent)
    : AudioCapture(parent)
{

}

AudioCaptureWaveIn::~AudioCaptureWaveIn()
{
    waveInStop(deviceHandle);
    waveInClose(deviceHandle);
}

bool AudioCaptureWaveIn::initialize(quint32 sampleRate, quint8 channels, quint16 bufferSize)
{
    MMRESULT result = 0;
    WAVEFORMATEX format;

    format.wFormatTag = WAVE_FORMAT_PCM; // simple, uncompressed format
    format.wBitsPerSample = 16;
    format.nChannels = 1;
    format.nSamplesPerSec = sampleRate;
    format.nAvgBytesPerSec = format.nSamplesPerSec *
                             format.nChannels *
                             (format.wBitsPerSample / 8);   // = nSamplesPerSec * n.Channels * wBitsPerSample/8
    format.nBlockAlign=format.nChannels*format.wBitsPerSample/8;
                                            // = n.Channels * wBitsPerSample/8
    format.cbSize=0;

    result = waveInOpen(&deviceHandle, WAVE_MAPPER, &format, 0L, 0L, CALLBACK_NULL);
    switch (result)
    {
    case MMSYSERR_ALLOCATED:
        qWarning("AudioRendererWaveOut: Device is already open.");
        return false;
    case MMSYSERR_BADDEVICEID:
        qWarning("AudioRendererWaveOut: The specified device is out of range.");
        return false;
    case MMSYSERR_NODRIVER:
        qWarning("AudioRendererWaveOut: There is no audio driver in this system.");
        return false;
    case MMSYSERR_NOMEM:
        qWarning("AudioRendererWaveOut: Unable to allocate sound memory.");
        return false;
    case WAVERR_BADFORMAT:
        qWarning("AudioRendererWaveOut: This audio format is not supported.");
        return false;
    default:
        qWarning("AudioRendererWaveOut: Unknown media error.");
        return false;
    case MMSYSERR_NOERROR:
        break;
    }

    return AudioCapture::initialize(sampleRate, channels, bufferSize);
}

qint64 AudioCaptureWaveIn::latency()
{
    return 0; // TODO
}

void AudioCaptureWaveIn::suspend()
{
}

void AudioCaptureWaveIn::resume()
{
}

bool AudioCaptureWaveIn::readAudio(int maxSize)
{
    WAVEHDR waveHeader;

    // Set up and prepare header for input
    waveHeader.lpData = (LPSTR)m_audioBuffer;
    waveHeader.dwBufferLength = maxSize;
    waveHeader.dwBytesRecorded = 0;
    waveHeader.dwUser = 0L;
    waveHeader.dwFlags = 0L;
    waveHeader.dwLoops = 0L;

    if (waveInPrepareHeader(deviceHandle, &waveHeader, sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
    {
        qWarning("[WAVEIN readAudio] PrepareHeader failed");
        return false;
    }
    // Insert a wave input buffer
    if (waveInAddBuffer(deviceHandle, &waveHeader, sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
    {
        qWarning("[WAVEIN readAudio] AddBuffer failed");
        return false;
    }

    if (waveInStart(deviceHandle) != MMSYSERR_NOERROR)
    {
        qWarning("[WAVEIN readAudio] WaveStart failed");
        return false;
    }

    while ( (waveHeader.dwFlags & WHDR_DONE) == 0) usleep(1000);

    //waveInReset(deviceHandle);
    if (waveInUnprepareHeader(deviceHandle, &waveHeader, sizeof(WAVEHDR) != MMSYSERR_NOERROR)
    {
        qWarning("[WAVEIN readAudio] UnprepareHeader failed");
        return false;
    }

    qDebug() << "[WAVEIN readAudio] " << maxSize << "bytes read";

    return true;
}











