/*
  Q Light Controller Plus
  audiodecoder.cpp

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

#include <QIODevice>

#include "audiodecoder.h"

void AudioDecoder::configure(quint32 srate, int chan, AudioFormat format)
{
    m_parameters = AudioParameters(srate, chan, format);
}

AudioParameters AudioDecoder::audioParameters() const
{
    return m_parameters;
}
