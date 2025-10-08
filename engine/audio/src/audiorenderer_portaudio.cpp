/*
  Q Light Controller Plus
  audiorenderer_portaudio.cpp

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

#include <QMutexLocker>
#include <QSettings>
#include <QString>
#include <QDebug>

#include "audiodecoder.h"
#include "audiorenderer_portaudio.h"

AudioRendererPortAudio::AudioRendererPortAudio(QString device, QObject * parent)
    : AudioRenderer(parent)
    , m_paStream(NULL)
    , m_device(device)
    , m_channels(0)
    , m_frameSize(0)
{
}

AudioRendererPortAudio::~AudioRendererPortAudio()
{
    reset();
    QMutexLocker locker(&m_paMutex);

    PaError err;
    err = Pa_Terminate();
    if (err != paNoError)
        qDebug() << "PortAudio error: " << Pa_GetErrorText(err);
}

int AudioRendererPortAudio::dataCallback(const void *, void *outputBuffer,
                                         unsigned long frameCount,
                                         const PaStreamCallbackTimeInfo*,
                                         PaStreamCallbackFlags ,
                                         void *userData)
{
    AudioRendererPortAudio *PAobj = (AudioRendererPortAudio *)userData;

    unsigned long requestedData = frameCount * PAobj->m_frameSize * PAobj->m_channels;

    QMutexLocker locker(&PAobj->m_paMutex);
    // user stop requested ? Prevent this callback to be called again
    if (PAobj->m_userStop == true)
        return paAbort;

    // not enough data ? Wait for another writeAudio to add more
    if (requestedData > (unsigned long)PAobj->m_buffer.size())
    {
        Pa_Sleep(10);
        return paContinue;
    }

    memcpy(outputBuffer, PAobj->m_buffer.data(), requestedData);
    PAobj->m_buffer.remove(0, requestedData);
    qDebug() << "PortAudio dataCallback. RequestedData: " << requestedData << "buffer size:" << PAobj->m_buffer.size();

    return paContinue;
}

bool AudioRendererPortAudio::initialize(quint32 freq, int chan, AudioFormat format)
{
    PaError err;
    PaStreamParameters outputParameters;
    PaStreamFlags flags = paNoFlag;

    err = Pa_Initialize();
    if (err != paNoError)
        return false;

    if (m_device.isEmpty())
    {
        QSettings settings;
        QVariant var = settings.value(SETTINGS_AUDIO_OUTPUT_DEVICE);
        if (var.isValid() == true)
            outputParameters.device = QString(var.toString()).toInt();
        else
            outputParameters.device = Pa_GetDefaultOutputDevice(); /* default output device */
    }
    else
        outputParameters.device = m_device.toInt();

    if (outputParameters.device == paNoDevice)
    {
        qDebug() << "Error: No default output device";
        return false;
    }

    m_channels = chan;

    outputParameters.channelCount = chan;       /* stereo output */
    outputParameters.suggestedLatency = Pa_GetDeviceInfo(outputParameters.device)->defaultLowOutputLatency;
    outputParameters.hostApiSpecificStreamInfo = NULL;

    switch (format)
    {
    case PCM_S8:
        outputParameters.sampleFormat = paInt8; /* 8 bit signed output */
        m_frameSize = 1;
        break;
    case PCM_S16LE:
        outputParameters.sampleFormat = paInt16; /* 16 bit signed output */
        m_frameSize = 2;
        break;
    case PCM_S24LE:
        outputParameters.sampleFormat = paInt24; /* 24 bit signed output */
        m_frameSize = 3;
        break;
    case PCM_S32LE:
        outputParameters.sampleFormat = paFloat32; /* 32 bit floating point output */
        m_frameSize = 4;
        break;
    default:
        qWarning("AudioRendererPortAudio: unsupported format detected");
        return false;
    }

    err = Pa_OpenStream(&m_paStream, NULL, &outputParameters,
              freq, paFramesPerBufferUnspecified, flags, dataCallback, this);

    if (err != paNoError)
        return false;

    err = Pa_StartStream(m_paStream);

    if (err != paNoError)
        return false;

    return true;
}

qint64 AudioRendererPortAudio::latency()
{
    return 0;
}

QList<AudioDeviceInfo> AudioRendererPortAudio::getDevicesInfo()
{
    QList<AudioDeviceInfo> devList;

    int numDevices, err, i;

    err = Pa_Initialize();
    if (err != paNoError)
        return devList;

    numDevices = Pa_GetDeviceCount();
    if (numDevices < 0)
    {
        qWarning("ERROR: Pa_CountDevices returned 0x%x\n", numDevices);
        return devList;
    }

    for (i = 0; i < numDevices; i++)
    {
        const PaDeviceInfo *deviceInfo = Pa_GetDeviceInfo(i);
        if (deviceInfo != NULL)
        {
            AudioDeviceInfo info;
            info.deviceName = QString(deviceInfo->name);
            info.privateName = QString::number(i);
            info.capabilities = 0;
            if (deviceInfo->maxInputChannels > 0)
                info.capabilities |= AUDIO_CAP_INPUT;
            if (deviceInfo->maxOutputChannels > 0)
                info.capabilities |= AUDIO_CAP_OUTPUT;
            devList.append(info);
        }
    }

    err = Pa_Terminate();
    if (err != paNoError)
        qDebug() << "PortAudio error: " << Pa_GetErrorText(err);

    return devList;
}

qint64 AudioRendererPortAudio::writeAudio(unsigned char *data, qint64 maxSize)
{
    if (m_buffer.size() > (8192 * 4))
        return 0;

    qDebug() << "writeAudio called !! - " << maxSize;
    QMutexLocker locker(&m_paMutex);
    m_buffer.append((const char *)data, maxSize);

    return maxSize;
}

void AudioRendererPortAudio::drain()
{
    m_buffer.clear();
}

void AudioRendererPortAudio::reset()
{
    QMutexLocker locker(&m_paMutex);
    if (m_paStream == NULL)
        return;

    PaError err;
    err = Pa_StopStream(m_paStream);
    if (err != paNoError)
        qDebug() << "PortAudio Error: Stop stream failed!";

    err = Pa_CloseStream(m_paStream);
    if (err != paNoError)
        qDebug() << "PortAudio Error: Close stream failed!";
    m_buffer.clear();
    m_paStream = NULL;
}

void AudioRendererPortAudio::suspend()
{
}

void AudioRendererPortAudio::resume()
{
}
