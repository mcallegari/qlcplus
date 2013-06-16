/*
  Q Light Controller Plus
  audiocapture_wavein.h

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

#ifndef AUDIOCAPTURE_WAVEIN_H
#define AUDIOCAPTURE_WAVEIN_H

#include "audiocapture.h"

#include <QMutex>

#define HEADERS_NUMBER   2

class AudioCaptureWaveIn : public AudioCapture
{
    Q_OBJECT
public:
    AudioCaptureWaveIn(QObject * parent = 0);
    ~AudioCaptureWaveIn();

    /** @reimpl */
    bool initialize(unsigned int sampleRate, quint8 channels, quint16 bufferSize);

    /** @reimpl */
    qint64 latency();

protected:
    /** @reimpl */
    void suspend();

    /** @reimpl */
    void resume();

    /** @reimpl */
    bool readAudio(int maxSize);

private:
    bool m_started;
    QMutex m_mutex;
    int m_currentBufferIndex;
    char *m_internalBuffers[HEADERS_NUMBER];
};

#endif // AUDIOCAPTURE_WAVEIN_H
