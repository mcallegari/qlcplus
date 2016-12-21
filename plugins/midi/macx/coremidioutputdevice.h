/*
  Q Light Controller
  coremidioutputdevice.h

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

#ifndef COREMIDIOUTPUTDEVICE_H
#define COREMIDIOUTPUTDEVICE_H

#include <CoreFoundation/CoreFoundation.h>
#include <CoreMIDI/CoreMIDI.h>

#include "midioutputdevice.h"

class CoreMidiOutputDevice : public MidiOutputDevice
{
public:
    CoreMidiOutputDevice(const QVariant& uid, const QString& name,
                         MIDIEndpointRef destination, MIDIClientRef client,
                         QObject* parent);
    virtual ~CoreMidiOutputDevice();

    bool open();
    void close();
    bool isOpen() const;

    void writeChannel(ushort channel, uchar value);
    void writeUniverse(const QByteArray& universe);
    void writeFeedback(uchar cmd, uchar data1, uchar data2);
    void writeSysEx(QByteArray message);

private:
    MIDIClientRef m_client;
    MIDIPortRef m_outPort;
    MIDIEndpointRef m_destination;
    QByteArray m_universe;
};

#endif
