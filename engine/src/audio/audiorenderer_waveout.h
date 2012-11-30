/*
  Q Light Controller Plus
  audiorenderer_waveout.h

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
 
#ifndef AUDIORENDERER_WAVEOUT_H
#define AUDIORENDERER_WAVEOUT_H

#include "audiorenderer.h"
#include "audiodecoder.h"

#include <QObject>
#include <stdio.h>
#include <windows.h>

class AudioRendererWaveOut : public AudioRenderer
{
    Q_OBJECT
public:
    AudioRendererWaveOut(QObject * parent = 0);
    ~AudioRendererWaveOut();

    bool initialize(quint32, int, AudioFormat format);
    qint64 latency();

    //output api
    qint64 writeAudio(unsigned char *data, qint64 maxSize);

private:
    void drain();
    void suspend();
    void resume();
    void reset(); 

    // helper functions
    void status();
    void uninitialize();
};

#endif // AUDIORENDERER_WAVEOUT_H