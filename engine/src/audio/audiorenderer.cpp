/*
  Q Light Controller Plus
  audiorenderer.cpp

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

#include "audiorenderer.h"
#include "qlcmacros.h"

AudioRenderer::AudioRenderer (QObject* parent)
    : QThread (parent)
    , m_userStop(true)
    , m_pause(false)
    , m_intensity(1.0)
    , audioDataRead(0)
    , pendingAudioBytes(0)
{
}

void AudioRenderer::setDecoder(AudioDecoder *adec)
{
    m_adec = adec;
}

void AudioRenderer::adjustIntensity(qreal fraction)
{
    m_intensity = CLAMP(fraction, 0.0, 1.0);
}

void AudioRenderer::stop()
{
    m_userStop = true;
    while (this->isRunning())
        usleep(10000);
    m_intensity = 1.0;
}

void AudioRenderer::run()
{
    m_userStop = false;
    audioDataRead = 0;

    while (!m_userStop)
    {
        m_mutex.lock();
        qint64 audioDataWritten = 0;
        if (m_pause == false)
        {
          //qDebug() << "Pending audio bytes: " << pendingAudioBytes;
          if (pendingAudioBytes == 0)
          {
            audioDataRead = m_adec->read((char *)audioData, 8192);
            if (audioDataRead == 0)
            {
                m_mutex.unlock();
                return;
            }
            if (m_intensity != 1.0)
            {
                for (int i = 0; i < audioDataRead; i++)
                    audioData[i] = (unsigned char)((char)audioData[i] * m_intensity);
            }
            audioDataWritten = writeAudio(audioData, audioDataRead);
            if (audioDataWritten < audioDataRead)
            {
                pendingAudioBytes = audioDataRead - audioDataWritten;
                usleep(15000);
            }
          }
          else
          {
            audioDataWritten = writeAudio(audioData + (audioDataRead - pendingAudioBytes), pendingAudioBytes);
            pendingAudioBytes -= audioDataWritten;
            if (audioDataWritten == 0)
                usleep(15000);
          }
          //qDebug() << "[Cycle] read: " << audioDataRead << ", written: " << audioDataWritten;
        }
        else
            usleep(15000);
        m_mutex.unlock();
    }

    reset();
}
