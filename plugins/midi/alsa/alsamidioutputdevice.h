/*
  Q Light Controller
  alsamidioutputdevice.h

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

#ifndef ALSAMIDIOUTPUTDEVICE_H
#define ALSAMIDIOUTPUTDEVICE_H

#include "midioutputdevice.h"

struct _snd_seq;
typedef _snd_seq snd_seq_t;

struct snd_seq_addr;
typedef snd_seq_addr snd_seq_addr_t;

class AlsaMidiOutputDevice : public MidiOutputDevice
{
public:
    AlsaMidiOutputDevice(const QVariant& uid, const QString& name,
                         const snd_seq_addr_t* recv_address, snd_seq_t* alsa,
                         snd_seq_addr_t* send_address, QObject* parent);
    virtual ~AlsaMidiOutputDevice();

    void open();
    void close();
    bool isOpen() const;

    void writeChannel(ushort channel, uchar value);
    void writeUniverse(const QByteArray& universe);
    void writeFeedback(uchar cmd, uchar data1, uchar data2);

private:
    snd_seq_t* m_alsa;
    snd_seq_addr_t* m_receiver_address;
    snd_seq_addr_t* m_sender_address;
    bool m_open;
    QByteArray m_universe;
};

#endif
