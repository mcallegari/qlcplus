/*
  Q Light Controller Plus
  audioparameters.cpp

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

// Copyright (c) 2000-2001 Brad Hughes <bhughes@trolltech.com>
//
// Use, modification and distribution is allowed without limitation,
// warranty, or liability of any kind.
//

#include "audioparameters.h"

AudioParameters::AudioParameters()
{
    m_srate = 0;
    m_chan = 0;
    m_format = PCM_S16LE;
}

AudioParameters::AudioParameters(const AudioParameters &other)
{
    m_srate = other.sampleRate();
    m_chan = other.channels();
    m_format = other.format();
}

AudioParameters::AudioParameters(quint32 srate, int chan, AudioFormat  format)
{
    m_srate = srate;
    m_chan = chan;
    m_format = format;
}

void AudioParameters::operator=(const AudioParameters &p)
{
    m_srate = p.sampleRate();
    m_chan = p.channels();
    m_format = p.format();
}

bool AudioParameters::operator==(const AudioParameters &p) const
{
    return m_srate == p.sampleRate() && m_chan == p.channels() && m_format == p.format();
}

bool AudioParameters::operator!=(const AudioParameters &p) const
{
    return !operator==(p);
}

quint32 AudioParameters::sampleRate() const
{
    return m_srate;
}

int AudioParameters::channels() const
{
    return m_chan;
}

AudioFormat AudioParameters::format() const
{
    return m_format;
}

int AudioParameters::sampleSize() const
{
    return sampleSize(m_format);
}

int AudioParameters::sampleSize(AudioFormat format)
{
    switch(format)
    {
    case PCM_S8:
        return 1;
    case PCM_S16LE:
    case PCM_UNKNOWN:
        return 2;
    case PCM_S24LE:
    case PCM_S32LE:
        return 4;
    }
    return 2;
}
