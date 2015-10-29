/*
  Q Light Controller
  alsamidiinputdevice.h

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

#ifndef ALSAMIDIINPUTDEVICE_H
#define ALSAMIDIINPUTDEVICE_H

#include "midiinputdevice.h"

struct _snd_seq;
typedef _snd_seq snd_seq_t;

struct snd_seq_addr;
typedef snd_seq_addr snd_seq_addr_t;
typedef unsigned char snd_seq_event_type_t;

class AlsaMidiInputThread;

class AlsaMidiInputDevice : public MidiInputDevice
{
public:
    AlsaMidiInputDevice(const QVariant& uid, const QString& name,
                        const snd_seq_addr_t* address, snd_seq_t* alsa,
                        AlsaMidiInputThread* thread, QObject* parent);
    virtual ~AlsaMidiInputDevice();

    bool open();
    void close();
    bool isOpen() const;

    const snd_seq_addr_t* address() const;

    bool processMBC(snd_seq_event_type_t type);

private:
    snd_seq_t* m_alsa;
    snd_seq_addr_t* m_address;
    AlsaMidiInputThread* m_thread;
    bool m_open;
    uint m_mbc_counter;
};

#endif
