/*
  Q Light Controller Plus
  audiorenderer_coreaudio.cpp

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

    status = AudioQueueNewOutput(&fmt, AudioRendererCoreAudio::inCallback, this, CFRunLoopGetCurrent(),
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
        qDebug() << Q_FUNC_INFO << "Cannot start Audio Queue !";
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
    if(m_buffersFilled == AUDIO_BUFFERS_NUM)
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
