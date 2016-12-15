/*
  Q Light Controller
  coremidiinputdevice.h

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

#ifndef COREMIDIINPUTDEVICE_H
#define COREMIDIINPUTDEVICE_H

#include <CoreFoundation/CoreFoundation.h>
#include <CoreMIDI/CoreMIDI.h>

#include "midiinputdevice.h"

class CoreMidiInputDevice : public MidiInputDevice
{
public:
    CoreMidiInputDevice(const QVariant& uid, const QString& name,
                        MIDIEndpointRef source, MIDIClientRef client, QObject* parent);
    virtual ~CoreMidiInputDevice();

    bool open();
    void close();
    bool isOpen() const;

    bool processMBC(int type);

private:
    MIDIClientRef m_client;
    MIDIPortRef m_inPort;
    MIDIEndpointRef m_source;
    uint m_mbc_counter;
};

#endif
