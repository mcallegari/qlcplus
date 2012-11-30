/*
  Q Light Controller Plus
  audiorenderer_null.h

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

#ifndef AUDIORENDERER_NULL_H
#define AUDIORENDERER_NULL_H

#include "audiorenderer.h"
#include "audiodecoder.h"

class AudioRendererNull : public AudioRenderer
{
    Q_OBJECT
public:
    AudioRendererNull(QObject * parent = 0);

    ~AudioRendererNull() { }

    bool initialize(quint32, int, AudioFormat) { return true; }
    qint64 latency() { return 0; } // not bad for a null device huh ? ;)

    //output api
    qint64 writeAudio(unsigned char *, qint64 maxSize) { return maxSize; }

};

#endif
