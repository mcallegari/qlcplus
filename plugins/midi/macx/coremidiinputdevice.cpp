/*
  Q Light Controller
  coremidiinputdevice.cpp

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

#include "coremidiinputdevice.h"
#include "midiprotocol.h"

/****************************************************************************
 * MidiInProc
 ****************************************************************************/

extern "C" {
static void MidiInProc(const MIDIPacketList* pktList, void* readProcRefCon,
                       void* srcConnRefCon)
{
    Q_UNUSED(readProcRefCon);

    CoreMidiInputDevice* self = static_cast<CoreMidiInputDevice*>(srcConnRefCon);
    Q_ASSERT(self != 0);

    // Go thru all packets in the midi packet list
    const MIDIPacket* packet = &pktList->packet[0];
    for (quint32 p = 0; p < pktList->numPackets && packet != NULL; ++p)
    {
        // Go thru all simultaneously-occurring messages in the packet
        for (quint32 i = 0; i < packet->length && i < 256; ++i)
        {
            uchar cmd = 0;
            uchar data1 = 0;
            uchar data2 = 0;
            quint32 channel = 0;
            uchar value = 0;

            // MIDI Command
            cmd = packet->data[0];
            if (!MIDI_IS_CMD(cmd))
                continue; // Not a MIDI command. Skip to the next byte.
            if (MIDI_CMD(cmd) == MIDI_SYSEX)
                break; // Sysex reserves the whole packet. Not interested.

            // 1 or 2 MIDI Data bytes
            if (packet->length > (i + 1) && !MIDI_IS_CMD(packet->data[i + 1]))
            {
                data1 = packet->data[++i];
                if (packet->length > (i + 1) && !MIDI_IS_CMD(packet->data[i + 1]))
                    data2 = packet->data[++i];
            }

            if (cmd >= MIDI_BEAT_CLOCK && cmd <= MIDI_BEAT_STOP)
            {
                if (self->processMBC(cmd) == false)
                    continue;
            }

            // Convert the data to QLC input channel & value
            if (QLCMIDIProtocol::midiToInput(cmd, data1, data2, self->midiChannel(),
                                             &channel, &value) == true)
            {
                self->emitValueChanged(channel, value);
                // for MIDI beat clock signals,
                // generate a synthetic release event
                if (cmd >= MIDI_BEAT_CLOCK && cmd <= MIDI_BEAT_STOP)
                    self->emitValueChanged(channel, 0);
            }
        }

        // Get the next packet in the packet list
        packet = MIDIPacketNext(packet);
    }
}
} // extern "C"

/****************************************************************************
 * CoreMidiInputDevice
 ****************************************************************************/

CoreMidiInputDevice::CoreMidiInputDevice(const QVariant& uid, const QString& name,
                                         MIDIEntityRef entity, MIDIClientRef client,
                                         QObject* parent)
    : MidiInputDevice(uid, name, parent)
    , m_client(client)
    , m_entity(entity)
    , m_inPort(0)
    , m_source(0)
{
    qDebug() << Q_FUNC_INFO;
}

CoreMidiInputDevice::~CoreMidiInputDevice()
{
    qDebug() << Q_FUNC_INFO;

    close();
}

void CoreMidiInputDevice::open()
{
    qDebug() << Q_FUNC_INFO;

    // Don't open twice
    if (m_inPort != 0)
        return;

    OSStatus s = MIDIInputPortCreate(m_client, CFSTR("QLC Input Port"),
                                     MidiInProc, this, &m_inPort);
    if (s != 0)
    {
        qWarning() << Q_FUNC_INFO << "Unable to make an input port for"
                   << name() << ":" << s;
        m_inPort = 0;
    }
    else
    {
        // Connect the input port to the first source
        m_source = MIDIEntityGetSource(m_entity, 0);
        s = MIDIPortConnectSource(m_inPort, m_source, this);
        if (s != 0)
        {
            qWarning() << Q_FUNC_INFO << "Unable to connect input port to source for"
                       << name() << ":" << s;

            s = MIDIPortDispose(m_inPort);
            if (s != 0)
                qWarning() << "Unable to dispose of input port in" << name();
            m_inPort = 0;
        }
    }
}

void CoreMidiInputDevice::close()
{
    qDebug() << Q_FUNC_INFO;

    // Don't close twice
    if (m_inPort == 0)
        return;

    OSStatus s = MIDIPortDisconnectSource(m_inPort, 0);
    if (s != 0)
        qWarning() << Q_FUNC_INFO << "Unable to disconnect input source in" << name();

    s = MIDIPortDispose(m_inPort);
    if (s != 0)
        qWarning() << "Unable to dispose of input port in" << name();
    m_inPort = 0;
}

bool CoreMidiInputDevice::isOpen() const
{
    qDebug() << Q_FUNC_INFO;

    if (m_inPort != 0)
        return true;
    else
        return false;
}

bool CoreMidiInputDevice::processMBC(int type)
{
    if (type == MIDI_BEAT_START || type == MIDI_BEAT_STOP)
    {
        m_mbc_counter = 1;
        return true;
    }
    else if (type == MIDI_BEAT_CLOCK)
    {
        if (m_mbc_counter == UINT_MAX)
        {
            m_mbc_counter = 1;
            return true;
        }
        m_mbc_counter++;
        if (m_mbc_counter == MIDI_BEAT_CLOCK_PPQ)
        {
            m_mbc_counter = 0;
            return true;
        }
    }
    return false;
}
