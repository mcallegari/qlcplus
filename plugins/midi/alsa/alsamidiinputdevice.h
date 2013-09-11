/*
  Q Light Controller
  alsamidiinputdevice.h

  Copyright (c) Heikki Junnila

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

    void open();
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
