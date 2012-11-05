/*
  Q Light Controller
  alsamidioutputdevice.cpp

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

#include <alsa/asoundlib.h>
#include <QDebug>

#include "alsamidioutputdevice.h"
#include "midiprotocol.h"

/****************************************************************************
 * AlsaMidiOutputDevice
 ****************************************************************************/

AlsaMidiOutputDevice::AlsaMidiOutputDevice(const QVariant& uid,
                                           const QString& name,
                                           const snd_seq_addr_t* address,
                                           snd_seq_t* alsa,
                                           QObject* parent)
    : MidiOutputDevice(uid, name, parent)
    , m_alsa(alsa)
    , m_address(new snd_seq_addr_t)
    , m_open(false)
    , m_universe(MAX_MIDI_DMX_CHANNELS, char(0))
{
    qDebug() << Q_FUNC_INFO;
    Q_ASSERT(alsa != NULL);
    Q_ASSERT(address != NULL);
    m_address->client = address->client;
    m_address->port = address->port;
}

AlsaMidiOutputDevice::~AlsaMidiOutputDevice()
{
    qDebug() << Q_FUNC_INFO;
    close();

    delete m_address;
    m_address = NULL;
}

void AlsaMidiOutputDevice::open()
{
    qDebug() << Q_FUNC_INFO;
    m_open = true;
}

void AlsaMidiOutputDevice::close()
{
    qDebug() << Q_FUNC_INFO;
    m_open = false;
}

bool AlsaMidiOutputDevice::isOpen() const
{
    return m_open;
}

void AlsaMidiOutputDevice::writeChannel(ushort channel, uchar value)
{
    if (channel < ushort(m_universe.size()) && uchar(m_universe[channel]) != value)
    {
        QByteArray tmp(m_universe);
        tmp[channel] = value;
        writeUniverse(tmp);
    }
}

void AlsaMidiOutputDevice::writeUniverse(const QByteArray& universe)
{
    if (isOpen() == false)
        return;

    // Setup a common event structure for all values
    snd_seq_event_t ev;
    snd_seq_ev_clear(&ev);
    snd_seq_ev_set_dest(&ev, m_address->client, m_address->port);
    snd_seq_ev_set_subs(&ev);
    snd_seq_ev_set_direct(&ev);

    // Since MIDI devices can have only 128 real channels, we don't
    // attempt to write more than that.
    for (uchar channel = 0; channel < MAX_MIDI_DMX_CHANNELS &&
                            channel < universe.size(); channel++)
    {
        // Scale 0-255 to 0-127
        char scaled = DMX2MIDI(universe[channel]);

        // Since MIDI is so slow, we only send values that are actually changed
        if (m_universe[channel] == scaled)
            continue;

        // Store the changed MIDI value
        m_universe[channel] = scaled;

        if (mode() == Note)
        {
            // 0 is sent as a note off
            // 1-127 is sent as note on
            if (scaled == 0)
                snd_seq_ev_set_noteoff(&ev, midiChannel(), channel, scaled);
            else
                snd_seq_ev_set_noteon(&ev, midiChannel(), channel, scaled);
            snd_seq_event_output(m_alsa, &ev);
        }
        else
        {
            // Control change
            snd_seq_ev_set_controller(&ev, midiChannel(), channel, scaled);
            snd_seq_event_output_buffer(m_alsa, &ev);
        }
    }

    // Make sure that all values go to the MIDI endpoint
    snd_seq_drain_output(m_alsa);
}
