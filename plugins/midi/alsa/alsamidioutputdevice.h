/*
  Q Light Controller
  alsamidioutputdevice.h

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

    bool open();
    void close();
    bool isOpen() const;

    void writeChannel(ushort channel, uchar value);
    void writeUniverse(const QByteArray& universe);
    void writeFeedback(uchar cmd, uchar data1, uchar data2);
    void writeSysEx(QByteArray message);

private:
    snd_seq_t* m_alsa;
    snd_seq_addr_t* m_receiver_address;
    snd_seq_addr_t* m_sender_address;
    bool m_open;
    QByteArray m_universe;
};

#endif
