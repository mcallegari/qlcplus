/*
  Q Light Controller
  alsamidioutputdevice.cpp

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

#include <alsa/asoundlib.h>
#include <QDebug>

#include "alsamidioutputdevice.h"
#include "midiprotocol.h"

/****************************************************************************
 * AlsaMidiOutputDevice
 ****************************************************************************/

AlsaMidiOutputDevice::AlsaMidiOutputDevice(const QVariant& uid,
                                           const QString& name,
                                           const snd_seq_addr_t* recv_address,
                                           snd_seq_t* alsa,
                                           snd_seq_addr_t* send_address,
                                           QObject* parent)
    : MidiOutputDevice(uid, name, parent)
    , m_alsa(alsa)
    , m_receiver_address(new snd_seq_addr_t)
    , m_open(false)
    , m_universe(MAX_MIDI_DMX_CHANNELS, char(0))
{
    Q_ASSERT(alsa != NULL);
    Q_ASSERT(recv_address != NULL);
    m_receiver_address->client = recv_address->client;
    m_receiver_address->port = recv_address->port;
    m_sender_address = send_address;
    qDebug() << "[AlsaMidiOutputDevice] receiver client: " << m_receiver_address->client << ", port: " << m_receiver_address->port;
    qDebug() << "[AlsaMidiOutputDevice] sender client (QLC+): " << m_sender_address->client << ", port: " << m_sender_address->port;
}

AlsaMidiOutputDevice::~AlsaMidiOutputDevice()
{
    qDebug() << Q_FUNC_INFO;
    close();

    delete m_receiver_address;
    m_receiver_address = NULL;
}

bool AlsaMidiOutputDevice::open()
{
    qDebug() << Q_FUNC_INFO;
    m_open = true;

    Q_ASSERT(m_sender_address != NULL);
    Q_ASSERT(m_receiver_address != NULL);

    /* Subscribe QLC+ ALSA client to the MIDI device */
    snd_seq_port_subscribe_t* sub = NULL;
    snd_seq_port_subscribe_alloca(&sub);
    snd_seq_port_subscribe_set_sender(sub, m_sender_address);
    snd_seq_port_subscribe_set_dest(sub, m_receiver_address);
    snd_seq_subscribe_port(m_alsa, sub);

    return true;
}

void AlsaMidiOutputDevice::close()
{
    qDebug() << Q_FUNC_INFO;
    m_open = false;

    Q_ASSERT(m_sender_address != NULL);
    Q_ASSERT(m_receiver_address != NULL);

    /* Unsubscribe QLC+ ALSA client to the MIDI device */
    snd_seq_port_subscribe_t* sub = NULL;
    snd_seq_port_subscribe_alloca(&sub);
    snd_seq_port_subscribe_set_sender(sub, m_sender_address);
    snd_seq_port_subscribe_set_dest(sub, m_receiver_address);
    snd_seq_unsubscribe_port(m_alsa, sub);
}

bool AlsaMidiOutputDevice::isOpen() const
{
    return m_open;
}

void AlsaMidiOutputDevice::writeChannel(ushort channel, uchar value)
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

void AlsaMidiOutputDevice::writeUniverse(const QByteArray& universe)
{
    if (isOpen() == false)
        return;

    // Setup a common event structure for all values
    snd_seq_event_t ev;
    snd_seq_ev_clear(&ev);
    snd_seq_ev_set_dest(&ev, m_receiver_address->client, m_receiver_address->port);
    //snd_seq_ev_set_subs(&ev);
    snd_seq_ev_set_direct(&ev);

    // Since MIDI devices can have only 128 real channels, we don't
    // attempt to write more than that.
    for (uchar channel = 0; channel < MAX_MIDI_DMX_CHANNELS &&
                            channel < universe.size(); channel++)
    {
        // Scale 0-255 to 0-127
        char scaled = DMX2MIDI(universe[channel]);
        bool invalidData = false;

        // Since MIDI is so slow, we only send values that are actually changed
        if (m_universe[channel] == scaled)
            continue;

        // Store the changed MIDI value
        m_universe[channel] = scaled;

        if (mode() == Note)
        {
            qDebug() << "Send out NOTE";
            // 0 is sent as a note off
            // 1-127 is sent as note on
            if (scaled == 0)
                snd_seq_ev_set_noteoff(&ev, midiChannel(), channel, scaled);
            else
                snd_seq_ev_set_noteon(&ev, midiChannel(), channel, scaled);
        }
        else if (mode() == ProgramChange)
        {
            qDebug() << "Send out Program Change";
            snd_seq_ev_set_pgmchange(&ev, midiChannel(), channel);
        }
        else if (mode() == ControlChange)
        {
            qDebug() << "Send out CC. Channel: " << midiChannel() << ", CC: " << channel << ", val: " << scaled;

            // Control change
            snd_seq_ev_set_controller(&ev, midiChannel(), channel, scaled);
        }
        else
            invalidData = true;

        if (!invalidData)
            if (snd_seq_event_output(m_alsa, &ev) < 0)
                qDebug() << "snd_seq_event_output ERROR";
    }

    // Make sure that all values go to the MIDI endpoint
    snd_seq_drain_output(m_alsa);
}

void AlsaMidiOutputDevice::writeFeedback(uchar cmd, uchar data1, uchar data2)
{
    if (isOpen() == false)
        return;

    // Setup a common event structure for all values
    snd_seq_event_t ev;
    snd_seq_ev_clear(&ev);
    snd_seq_ev_set_dest(&ev, m_receiver_address->client, m_receiver_address->port);
    //snd_seq_ev_set_subs(&ev);
    snd_seq_ev_set_direct(&ev);

    uchar midiCmd = MIDI_CMD(cmd);
    uchar midiCh = MIDI_CH(cmd);

    bool invalidCmd = false;

    switch(midiCmd)
    {
    case MIDI_NOTE_OFF:
        snd_seq_ev_set_noteoff(&ev, midiCh, data1, data2);
        break;

    case MIDI_NOTE_ON:
        snd_seq_ev_set_noteon(&ev, midiCh, data1, data2);
        break;

    case MIDI_CONTROL_CHANGE:
        snd_seq_ev_set_controller(&ev, midiCh, data1, data2);
        break;

    case MIDI_PROGRAM_CHANGE:
        snd_seq_ev_set_pgmchange(&ev, midiCh, data1);
        break;

    case MIDI_NOTE_AFTERTOUCH:
        snd_seq_ev_set_keypress(&ev, midiCh, data1, data2);
        break;

    case MIDI_CHANNEL_AFTERTOUCH:
        snd_seq_ev_set_chanpress(&ev, midiCh, data1);
        break;

    case MIDI_PITCH_WHEEL:
        snd_seq_ev_set_pitchbend(&ev, midiCh, ((data1 & 0x7f) | ((data2 & 0x7f) << 7)) - 8192);
        break;

    default:
        // What to do here ??
        invalidCmd = true;
        break;
    }

    if (!invalidCmd)
    {
        if (snd_seq_event_output(m_alsa, &ev) < 0)
            qDebug() << "snd_seq_event_output ERROR";
    }

    // Make sure that all values go to the MIDI endpoint
    snd_seq_drain_output(m_alsa);
}


void AlsaMidiOutputDevice::writeSysEx(QByteArray message)
{
    if (message.isEmpty())
        return;

    if (isOpen() == false)
        return;

    snd_seq_event_t ev;
    snd_seq_ev_clear(&ev);
    snd_seq_ev_set_dest(&ev, m_receiver_address->client, m_receiver_address->port);
    //snd_seq_ev_set_subs(&ev);
    snd_seq_ev_set_direct(&ev);

    snd_seq_ev_set_sysex (&ev, message.length(), message.data());

    if (snd_seq_event_output(m_alsa, &ev) < 0)
        qDebug() << "snd_seq_event_output ERROR";

    // Make sure that all values go to the MIDI endpoint
    snd_seq_drain_output(m_alsa);
}
