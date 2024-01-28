/*
  Q Light Controller Plus
  audiocapture_wavein.cpp

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

#include <QDebug>
#include <windows.h>
#include <memory.h>

#include "audiocapture_wavein.h"

static HWAVEIN deviceHandle = NULL;
static WAVEHDR waveHeaders[HEADERS_NUMBER];

AudioCaptureWaveIn::AudioCaptureWaveIn(QObject * parent)
    : AudioCapture(parent)
    , m_currentBufferIndex(0)
{

}

AudioCaptureWaveIn::~AudioCaptureWaveIn()
{
    stop();
}

bool AudioCaptureWaveIn::initialize()
{
    MMRESULT result = 0;
    WAVEFORMATEX format;

    format.wFormatTag = WAVE_FORMAT_PCM; // simple, uncompressed format
    format.wBitsPerSample = 16;
    format.nChannels = m_channels;
    format.nSamplesPerSec = m_sampleRate;
    format.nAvgBytesPerSec = format.nSamplesPerSec *
                             format.nChannels *
                             (format.wBitsPerSample / 8);
    format.nBlockAlign = format.nChannels*format.wBitsPerSample/8;
    format.cbSize=0;

    result = waveInOpen(&deviceHandle, WAVE_MAPPER, &format, 0L, 0L, CALLBACK_NULL|WAVE_FORMAT_DIRECT);
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

    for (int i = 0; i < HEADERS_NUMBER; i++)
    {
        m_internalBuffers[i] = new char[m_captureSize * 2];
        // Set up and prepare header for input
        waveHeaders[i].lpData = (LPSTR)m_internalBuffers[i];
        waveHeaders[i].dwBufferLength = m_captureSize * 2; // multiply by 2 cause they're 16bit samples
        waveHeaders[i].dwBytesRecorded = 0;
        waveHeaders[i].dwUser = 0L;
        waveHeaders[i].dwFlags = 0L;
        waveHeaders[i].dwLoops = 0L;

        if (waveInPrepareHeader(deviceHandle, &waveHeaders[i], sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
        {
            qWarning("[WAVEIN readAudio] PrepareHeader failed");
            return false;
        }
    }
    // Insert the first input buffer
    if (waveInAddBuffer(deviceHandle, &waveHeaders[0], sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
    {
        qWarning("[WAVEIN readAudio] AddBuffer failed");
        return false;
    }

    if (waveInStart(deviceHandle) != MMSYSERR_NOERROR)
    {
        qWarning("[WAVEIN readAudio] WaveStart failed");
        return false;
    }

    return true;
}

void AudioCaptureWaveIn::uninitialize()
{
    if (deviceHandle)
    {
        waveInStop(deviceHandle);
        for (int i = 0; i < HEADERS_NUMBER; i++)
        {
            if (waveInUnprepareHeader(deviceHandle, &waveHeaders[i], sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
                qWarning("[WAVEIN readAudio] UnprepareHeader failed");
        }

        waveInReset(deviceHandle);
        waveInClose(deviceHandle);
    }
    deviceHandle = NULL;
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
    int newBufferIndex = m_currentBufferIndex + 1;
    if (newBufferIndex == HEADERS_NUMBER)
        newBufferIndex = 0;

    // queue the next buffer to avoid losing data
    if (waveInAddBuffer(deviceHandle, &waveHeaders[newBufferIndex], sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
    {
        qWarning("[WAVEIN readAudio] AddBuffer failed");
        return false;
    }

    while ((waveHeaders[m_currentBufferIndex].dwFlags & WHDR_DONE) == 0)
        usleep(100);

    memcpy(m_audioBuffer, m_internalBuffers[m_currentBufferIndex], maxSize * 2);

    m_currentBufferIndex = newBufferIndex;

    qDebug() << "[WAVEIN readAudio] " << maxSize << "bytes read";

    return true;
}
