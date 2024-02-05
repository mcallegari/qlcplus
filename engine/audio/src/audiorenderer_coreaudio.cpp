/*
  Q Light Controller Plus
  audiorenderer_coreaudio.cpp

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

#include <QString>
#include <QDebug>

#include "audiodecoder.h"
#include "audiorenderer_coreaudio.h"

AudioRendererCoreAudio::AudioRendererCoreAudio(QObject * parent)
    : AudioRenderer(parent)
{
    m_buffersFilled = 0;
    m_bufferIndex = 0;
}

AudioRendererCoreAudio::~AudioRendererCoreAudio()
{
}

void AudioRendererCoreAudio::inCallback(void *inUserData, AudioQueueRef, AudioQueueBufferRef)
{
    AudioRendererCoreAudio *CAobj = (AudioRendererCoreAudio *)inUserData;
    qDebug() << "inCallback called !!";
    CAobj->m_buffersFilled--;
}

bool AudioRendererCoreAudio::initialize(quint32 freq, int chan, AudioFormat format)
{
    OSStatus status;
    AudioStreamBasicDescription fmt;

    fmt.mFormatID = kAudioFormatLinearPCM;
    fmt.mFormatFlags = kAudioFormatFlagIsSignedInteger | kAudioFormatFlagIsPacked;
    fmt.mSampleRate = freq;
    fmt.mFramesPerPacket = 1;
    fmt.mChannelsPerFrame = chan;
    switch (format)
    {
    case PCM_S8:
        fmt.mBitsPerChannel = 8;
        break;
    case PCM_S16LE:
        fmt.mBitsPerChannel = 16;
        break;
    case PCM_S24LE:
        fmt.mBitsPerChannel = 24;
        break;
    case PCM_S32LE:
        fmt.mBitsPerChannel = 32;
        break;
    default:
        qWarning("AudioRendererCoreAudio: unsupported format detected");
        return false;
    }
    fmt.mBytesPerFrame = fmt.mChannelsPerFrame * fmt.mBitsPerChannel/8;
    fmt.mBytesPerPacket = fmt.mBytesPerFrame * fmt.mFramesPerPacket;

    status = AudioQueueNewOutput(&fmt, AudioRendererCoreAudio::inCallback, this, CFRunLoopGetMain(),
                    kCFRunLoopCommonModes, 0, &m_queue);

    if (status == kAudioFormatUnsupportedDataFormatError)
    {
        qDebug() << Q_FUNC_INFO << "Fatal: Unsupported data format";
        return false;
    }
    else
        qDebug() << Q_FUNC_INFO << "initialize status: " << status;

    for (int i = 0; i < AUDIO_BUFFERS_NUM; i++)
    {
        status = AudioQueueAllocateBuffer (m_queue, AUDIO_BUFFER_SIZE, &m_buffer[i]);
        qDebug() << Q_FUNC_INFO << "Buffer #" << i << " allocate status: " << status;
    }

    status = AudioQueueStart(m_queue, NULL);
    if (status)
    {
        qDebug() << Q_FUNC_INFO << "Cannot start Audio Queue!";
        return false;
    }

    return true;
}

qint64 AudioRendererCoreAudio::latency()
{
    return 0;
}

qint64 AudioRendererCoreAudio::writeAudio(unsigned char *data, qint64 maxSize)
{
    if (m_buffersFilled == AUDIO_BUFFERS_NUM)
        return 0;

    qint64 size = maxSize;
    if (maxSize > AUDIO_BUFFER_SIZE)
        size = AUDIO_BUFFER_SIZE;

    m_buffer[m_bufferIndex]->mAudioDataByteSize = size;
    memcpy(m_buffer[m_bufferIndex]->mAudioData, data, size);

    AudioQueueEnqueueBuffer(m_queue, m_buffer[m_bufferIndex], 0, 0);
    m_bufferIndex++;
    if (m_bufferIndex == AUDIO_BUFFERS_NUM)
        m_bufferIndex = 0;
    m_buffersFilled++;

    return size;
}

void AudioRendererCoreAudio::drain()
{
}

void AudioRendererCoreAudio::reset()
{
    OSStatus status;

    AudioQueueStop(m_queue, true);
    m_bufferIndex = 0;
    m_buffersFilled = 0;
    for (int i = 0; i < AUDIO_BUFFERS_NUM; i++)
    {
        status = AudioQueueFreeBuffer(m_queue, m_buffer[i]);
        if (status)
            qDebug() << Q_FUNC_INFO << "Failed to free buffer #" << i;
    }
}

void AudioRendererCoreAudio::suspend()
{
    OSStatus status;

    status = AudioQueuePause(m_queue);
    if (status)
        qDebug() << Q_FUNC_INFO << "Failed to pause Audio Queue !!";
}

void AudioRendererCoreAudio::resume()
{
    OSStatus status;

    status = AudioQueueStart(m_queue, NULL);
    if (status)
        qDebug() << Q_FUNC_INFO << "Failed to resume Audio Queue !!";
}
