/*
  Q Light Controller Plus
  audiorenderer_alsa.h

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

/******************************************************
 *   Based on qmmp project                            *
 *                                                    *
 *   Copyright (C) 2007-2009 by Ilya Kotov            *
 *   forkotov02@hotmail.ru                            *
 ******************************************************/

#ifndef AUDIORENDERER_ALSA_H
#define AUDIORENDERER_ALSA_H

#include "audiorenderer.h"
#include "audiodecoder.h"

extern "C"
{
#include <alsa/asoundlib.h>
}

class AudioRendererAlsa : public AudioRenderer
{
    Q_OBJECT
public:
    AudioRendererAlsa(QObject * parent = 0);
    ~AudioRendererAlsa();

    /** @reimpl */
    bool initialize(quint32, int, AudioFormat format);

    /** @reimpl */
    qint64 latency();

    static QList<AudioDeviceInfo> getDevicesInfo();

protected:
    /** @reimpl */
    qint64 writeAudio(unsigned char *data, qint64 maxSize);

    /** @reimpl */
    void drain();

    /** @reimpl */
    void reset();

    /** @reimpl */
    void suspend();

    /** @reimpl */
    void resume();

private:
    // helper functions
    long alsa_write(unsigned char *data, long size);

    void uninitialize();

    bool m_inited;
    bool m_use_mmap;

    // ALSA specific
    snd_pcm_t *pcm_handle;
    char *pcm_name;
    snd_pcm_uframes_t m_chunk_size;
    size_t m_bits_per_frame;
    //prebuffer
    uchar *m_prebuf;
    qint64 m_prebuf_size;
    qint64 m_prebuf_fill;
    bool m_can_pause;
};

#endif // AUDIORENDERER_ALSA_H
