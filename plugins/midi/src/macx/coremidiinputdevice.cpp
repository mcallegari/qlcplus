/*
  Q Light Controller
  coremidiinputdevice.cpp

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
            cmd = packet->data[i];
            if (!MIDI_IS_CMD(cmd))
                continue; // Not a MIDI command. Skip to the next byte.
            if (cmd == MIDI_SYSEX)
                break; // Sysex reserves the whole packet. Not interested.

            // 1 or 2 MIDI Data bytes
            if (packet->length > (i + 1) && !MIDI_IS_CMD(packet->data[i + 1]))
            {
                data1 = packet->data[++i];
                if (packet->length > (i + 1) && !MIDI_IS_CMD(packet->data[i + 1]))
                    data2 = packet->data[++i];
                else
                    // no data2 ? Could be a Program Change, so act like Linux
                    // and give it a value
                    data2 = 127;
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
                                         MIDIEndpointRef source, MIDIClientRef client,
                                         QObject* parent)
    : MidiInputDevice(uid, name, parent)
    , m_client(client)
    , m_inPort(0)
    , m_source(source)
{
}

CoreMidiInputDevice::~CoreMidiInputDevice()
{
    qDebug() << Q_FUNC_INFO;

    close();
}

bool CoreMidiInputDevice::open()
{
    qDebug() << Q_FUNC_INFO;

    // Don't open twice
    if (m_inPort != 0)
        return false;

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
        // Connect the input port to the source
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
    return true;
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
    else if (type == MIDI_BEAT_CONTINUE)
    {
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
