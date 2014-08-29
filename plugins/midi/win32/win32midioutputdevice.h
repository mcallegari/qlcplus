/*
  Q Light Controller
  win32midioutputdevice.h

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

#ifndef WIN32MIDIOUTPUTDEVICE_H
#define WIN32MIDIOUTPUTDEVICE_H

#include <Windows.h>
#include <QObject>

#include "midioutputdevice.h"

class Win32MidiOutputDevice : public MidiOutputDevice
{
    Q_OBJECT

public:
    Win32MidiOutputDevice(const QVariant& uid, const QString& name, UINT id, QObject* parent = 0);
    ~Win32MidiOutputDevice();

    bool open();
    void close();
    bool isOpen() const;

    void writeChannel(ushort channel, uchar value);
    void writeUniverse(const QByteArray& universe);
    void writeFeedback(uchar cmd, uchar data1, uchar data2);
    void writeSysEx(QByteArray message);

private:
    void sendData(BYTE command, BYTE channel, BYTE value);

private:
    UINT m_id;
    HMIDIOUT m_handle;
    QByteArray m_universe;
};

#endif
