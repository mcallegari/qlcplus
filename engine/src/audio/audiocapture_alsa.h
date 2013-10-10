/*
  Q Light Controller Plus
  audiocapture_alsa.h

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

#ifndef AUDIOCAPTURE_ALSA_H
#define AUDIOCAPTURE_ALSA_H

#include "audiocapture.h"

extern "C"
{
#include <alsa/asoundlib.h>
}

class AudioCaptureAlsa : public AudioCapture
{
    Q_OBJECT
public:
    AudioCaptureAlsa(QObject * parent = 0);
    ~AudioCaptureAlsa();

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
    snd_pcm_t *m_captureHandle;
    char *pcm_name;
};

#endif // AUDIOCAPTURE_ALSA_H
