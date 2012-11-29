/*
  Q Light Controller Plus
  audiodecoder.cpp

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

#include <QIODevice>

#include "audiodecoder.h"

AudioDecoder::AudioDecoder()
{

}

AudioDecoder::~AudioDecoder()
{}

void AudioDecoder::configure(quint32 srate, int chan, AudioFormat format)
{
    m_parameters = AudioParameters(srate, chan, format);
}

AudioParameters AudioDecoder::audioParameters() const
{
    return m_parameters;
}
