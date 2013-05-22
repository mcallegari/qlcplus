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

AudioCaptureWaveIn::AudioCaptureWaveIn(QObject * parent)
    : AudioCapture(parent)
{

}

AudioCaptureWaveIn::~AudioCaptureWaveIn()
{

}

bool AudioCaptureWaveIn::initialize(quint32 sampleRate, quint8 channels, quint16 bufferSize)
{


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


    qDebug() << "[WAVEIN readAudio] " << maxSize << "bytes read";

    return true;
}











