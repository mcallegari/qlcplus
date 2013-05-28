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
#include <memory.h>

#include "audiocapture_wavein.h"

static HWAVEIN deviceHandle = NULL;
static WAVEHDR waveHeaders[HEADERS_NUMBER];

AudioCaptureWaveIn::AudioCaptureWaveIn(QObject * parent)
    : AudioCapture(parent)
    , m_started(false)
    , m_currentBufferIndex(0)
{

}

AudioCaptureWaveIn::~AudioCaptureWaveIn()
{
    m_mutex.lock();
    if (deviceHandle)
    {
        waveInReset(deviceHandle);
        //waveInStop(deviceHandle);
        waveInClose(deviceHandle);
    }
    m_mutex.unlock();
}

bool AudioCaptureWaveIn::initialize(unsigned int sampleRate, quint8 channels, quint16 bufferSize)
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
    if (m_started == false)
    {
        for (int i = 0; i < HEADERS_NUMBER; i++)
        {
            m_internalBuffers[i] = new qint16[m_captureSize];
            // Set up and prepare header for input
            waveHeaders[i].lpData = (LPSTR)m_internalBuffers[i];
            waveHeaders[i].dwBufferLength = maxSize;
            waveHeaders[i].dwBytesRecorded = 0;
            waveHeaders[i].dwUser = 0L;
            waveHeaders[i].dwFlags = 0L;
            waveHeaders[i].dwLoops = 0L;

            if (waveInPrepareHeader(deviceHandle, &waveHeaders[i], sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
            {
                qWarning("[WAVEIN readAudio] PrepareHeader failed");
                return false;
            }
            // Insert the first input buffer
            if (waveInAddBuffer(deviceHandle, &waveHeaders[i], sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
            {
                qWarning("[WAVEIN readAudio] AddBuffer failed");
                return false;
            }
        }

        if (waveInStart(deviceHandle) != MMSYSERR_NOERROR)
        {
            qWarning("[WAVEIN readAudio] WaveStart failed");
            return false;
        }
        m_started = true;
    }

    m_mutex.lock();

    while ( (waveHeaders[m_currentBufferIndex].dwFlags & WHDR_DONE) == 0)
        usleep(1000);

    memcpy(m_audioBuffer, m_internalBuffers[m_currentBufferIndex], maxSize);
/*
    if (waveInUnprepareHeader(deviceHandle, &waveHeaders[m_currentBufferIndex], sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
    {
        qWarning("[WAVEIN readAudio] UnprepareHeader failed");
        m_mutex.unlock();
        return false;
    }
*/
    // requeue the buffer for another use
    if (waveInAddBuffer(deviceHandle, &waveHeaders[m_currentBufferIndex], sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
    {
        qWarning("[WAVEIN readAudio] AddBuffer failed");
        m_mutex.unlock();
        return false;
    }

    m_currentBufferIndex++;
    if (m_currentBufferIndex == HEADERS_NUMBER)
        m_currentBufferIndex = 0;
/*
    if (waveInPrepareHeader(deviceHandle, &waveHeaders[m_currentBufferIndex], sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
    {
        qWarning("[WAVEIN readAudio] PrepareHeader failed");
        m_mutex.unlock();
        return false;
    }

    if (waveInAddBuffer(deviceHandle, &waveHeaders[m_currentBufferIndex], sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
    {
        qWarning("[WAVEIN readAudio] AddBuffer failed");
        m_mutex.unlock();
        return false;
    }
*/
    m_mutex.unlock();

    qDebug() << "[WAVEIN readAudio] " << maxSize << "bytes read";

    return true;
}











