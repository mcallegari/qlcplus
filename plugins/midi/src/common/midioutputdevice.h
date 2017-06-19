/*
  Q Light Controller
  midioutputdevice.h

  Copyright (c) Heikki Junnila

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

#ifndef MIDIOUTPUTDEVICE_H
#define MIDIOUTPUTDEVICE_H

#include "mididevice.h"

class MidiOutputDevice : public MidiDevice
{
    Q_OBJECT

public:
    MidiOutputDevice(const QVariant& uid, const QString& name, QObject* parent = 0);
    virtual ~MidiOutputDevice();

public:
    virtual void writeChannel(ushort channel, uchar value) = 0;
    virtual void writeUniverse(const QByteArray& universe) = 0;
    virtual void writeFeedback(uchar cmd, uchar data1, uchar data2) = 0;
    virtual void writeSysEx(QByteArray message) = 0;
};

#endif
