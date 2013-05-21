/*
  Q Light Controller
  coremidioutputdevice.cpp

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

#include <QDebug>

#include "coremidioutputdevice.h"
#include "midiprotocol.h"

/****************************************************************************
 * CoreMidiOutputDevice
 ****************************************************************************/

CoreMidiOutputDevice::CoreMidiOutputDevice(const QVariant& uid, const QString& name,
                                           MIDIEntityRef entity, MIDIClientRef client,
                                           QObject* parent)
    : MidiOutputDevice(uid, name, parent)
    , m_client(client)
    , m_entity(entity)
    , m_outPort(0)
    , m_destination(0)
    , m_universe(MAX_MIDI_DMX_CHANNELS, char(0))
{
    qDebug() << Q_FUNC_INFO;
}

CoreMidiOutputDevice::~CoreMidiOutputDevice()
{
    qDebug() << Q_FUNC_INFO;
    close();
}

void CoreMidiOutputDevice::open()
{
    qDebug() << Q_FUNC_INFO;

    // Don't open twice
    if (m_outPort != 0)
        return;

    OSStatus s = MIDIOutputPortCreate(m_client, CFSTR("QLC Input Port"), &m_outPort);
    if (s != 0)
    {
        qWarning() << Q_FUNC_INFO << "Unable to make an output port for"
                   << name() << ":" << s;
        m_outPort = 0;
    }
    else
    {
        m_destination = MIDIEntityGetDestination(m_entity, 0);
    }
}

void CoreMidiOutputDevice::close()
{
    qDebug() << Q_FUNC_INFO;

    // Don't close twice
    if (m_outPort == 0)
        return;

    OSStatus s = MIDIPortDispose(m_outPort);
    if (s != 0)
        qWarning() << "Unable to dispose of output port in" << name();
    m_outPort = 0;
}

bool CoreMidiOutputDevice::isOpen() const
{
    if (m_outPort != 0)
        return true;
    else
        return false;
}

void CoreMidiOutputDevice::writeChannel(ushort channel, uchar value)
{
    // m_universe contains scaled values (0-127), so we have to compare scaled value as well
    // however, since writeUniverse scales the value again, we have to store unscaled value.
    char scaled = DMX2MIDI(value);
    if (channel < ushort(m_universe.size()) && m_universe[channel] != scaled)
    {
        QByteArray tmp(m_universe);

        for (uchar ch = 0; ch < MAX_MIDI_DMX_CHANNELS && ch < tmp.size(); ++ch)
        {
           char midi = tmp[ch];
           tmp[ch] = (char)MIDI2DMX(midi);
        }

        tmp[channel] = value;
        writeUniverse(tmp);
    }
}

void CoreMidiOutputDevice::writeUniverse(const QByteArray& universe)
{
    if (isOpen() == false)
        return;

    Byte buffer[512]; // Should be enough for 128 channels
    MIDIPacketList* list = (MIDIPacketList*) buffer;
    MIDIPacket* packet = MIDIPacketListInit(list);

    /* Since MIDI devices can have only 128 real channels, we don't
       attempt to write more than that */
    for (Byte channel = 0; channel < MAX_MIDI_DMX_CHANNELS &&
                           channel < universe.size(); channel++)
    {
        Byte cmd[3];
        cmd[1] = channel;
        cmd[2] = DMX2MIDI(uchar(universe[channel]));

        /* Since MIDI is so slow, we only send values that are
           actually changed. */
        if (uchar(m_universe[channel]) == cmd[2])
            continue;

        /* Store the changed MIDI value. */
        m_universe[channel] = cmd[2];

        if (mode() == Note)
        {
            if (cmd[2] == 0)
            {
                /* Zero is sent as a note off command */
                cmd[0] = MIDI_NOTE_OFF;
            }
            else
            {
                /* 1-127 is sent as note on command */
                cmd[0] = MIDI_NOTE_ON;
            }
        }
        else
        {
            /* Control change */
            cmd[0] = MIDI_CONTROL_CHANGE;
        }

        /* Encode MIDI channel to the command */
        cmd[0] |= (Byte) midiChannel();

        /* Add the MIDI command to the packet list */
        packet = MIDIPacketListAdd(list, sizeof(buffer), packet, 0, sizeof(cmd), cmd);
        if (packet == 0)
        {
            qWarning() << "MIDIOut buffer overflow";
            break;
        }
    }

    /* Send the MIDI packet list */
    OSStatus s = MIDISend(m_outPort, m_destination, list);
    if (s != 0)
        qWarning() << Q_FUNC_INFO << "Unable to send MIDI data to" << name();
}
