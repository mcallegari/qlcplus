/*
  Q Light Controller Plus
  audioparameters.cpp

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

// Copyright (c) 2000-2001 Brad Hughes <bhughes@trolltech.com>
//
// Use, modification and distribution is allowed without limitation,
// warranty, or liability of any kind.
//

#include "audioparameters.h"

AudioParameters::AudioParameters()
    : m_srate(0)
    , m_chan(0)
    , m_format(PCM_S16LE)
{
}

AudioParameters::AudioParameters(const AudioParameters &other)
    : m_srate(other.sampleRate())
    , m_chan(other.channels())
    , m_format(other.format())
{
}

AudioParameters::AudioParameters(quint32 srate, int chan, AudioFormat  format)
    : m_srate(srate)
    , m_chan(chan)
    , m_format(format)
{
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

