/*
  Q Light Controller
  win32midioutputdevice.cpp

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

#include "win32midioutputdevice.h"
#include "midiprotocol.h"

Win32MidiOutputDevice::Win32MidiOutputDevice(const QVariant& uid, const QString& name, UINT id,
                                             QObject* parent)
    : MidiOutputDevice(uid, name, parent)
    , m_id(id)
    , m_handle(NULL)
    , m_universe(MAX_MIDI_DMX_CHANNELS, char(0))
{
    qDebug() << Q_FUNC_INFO;
}

Win32MidiOutputDevice::~Win32MidiOutputDevice()
{
    qDebug() << Q_FUNC_INFO;
}

bool Win32MidiOutputDevice::open()
{
    qDebug() << Q_FUNC_INFO;

    if (m_handle != NULL)
        return false;

    MMRESULT result = midiOutOpen(&m_handle, m_id, 0, 0, 0);
    if (result != MMSYSERR_NOERROR)
    {
        qWarning() << Q_FUNC_INFO << "Unable to open MIDI output device with id:" << m_id
                   << "name:" << name() << ":" << result;
        m_handle = NULL;
        return false;
    }
    return true;
}

void Win32MidiOutputDevice::close()
{
    qDebug() << Q_FUNC_INFO;

    if (m_handle == NULL)
        return;

    MMRESULT result = midiOutClose(m_handle);
    if (result != MMSYSERR_NOERROR)
        qWarning() << Q_FUNC_INFO << "Unable to close MIDI output with id:" << m_id
                   << "name:" << name() << ":" << result;
    m_handle = NULL;
}

bool Win32MidiOutputDevice::isOpen() const
{
    if (m_handle != NULL)
        return true;
    else
        return false;
}

void Win32MidiOutputDevice::writeChannel(ushort channel, uchar value)
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

void Win32MidiOutputDevice::writeUniverse(const QByteArray& universe)
{
    if (isOpen() == false)
        return;

    for (BYTE channel = 0; channel < MAX_MIDI_DMX_CHANNELS &&
                           channel < universe.size(); channel++)
    {
        /* Scale 0-255 to 0-127 */
        char scaled = DMX2MIDI(uchar(universe[channel]));

        /* Since MIDI is so slow, we only send values that are
           actually changed. */
        if (m_universe[channel] == scaled)
            continue;

        /* Store the changed MIDI value */
        m_universe[channel] = scaled;

        if (mode() == Note)
        {
            if (scaled == 0)
            {
                /* Zero is sent as a note off command */
                sendData(MIDI_NOTE_OFF | (BYTE) midiChannel(), channel, scaled);
            }
            else
            {
                /* 1-127 are sent as note on commands */
                sendData(MIDI_NOTE_ON | (BYTE) midiChannel(), channel, scaled);
            }
        }
        else if (mode() == ProgramChange)
        {
            /* Program change */
            sendData(MIDI_PROGRAM_CHANGE | (BYTE) midiChannel(), channel, (BYTE)scaled);
        }
        else
        {
            //qDebug() << "[writeUniverse] MIDI: " << midiChannel() << ", channel: " << channel << ", value: " << scaled;
            /* Control change */
            sendData(MIDI_CONTROL_CHANGE | (BYTE) midiChannel(), channel, (BYTE)scaled);
        }
    }
}

void Win32MidiOutputDevice::writeFeedback(uchar cmd, uchar data1, uchar data2)
{
    if (isOpen() == false)
        return;

    sendData(cmd, data1, data2);
}

void Win32MidiOutputDevice::sendData(BYTE command, BYTE channel, BYTE value)
{
    union
    {
        DWORD dwData;
        BYTE bData[4];
    } msg;

    msg.bData[0] = command;
    msg.bData[1] = channel;
    msg.bData[2] = value;
    msg.bData[3] = 0;

    /* Push the message out */
    midiOutShortMsg(m_handle, msg.dwData);
}

void Win32MidiOutputDevice::writeSysEx(QByteArray message)
{
    if (message.isEmpty())
        return;

    if (isOpen() == false)
        return;

    MIDIHDR midiHdr;

    /* Store pointer in MIDIHDR */
    midiHdr.lpData = (LPSTR)message.data();

    /* Store its size in the MIDIHDR */
    midiHdr.dwBufferLength = message.length();

    /* Flags must be set to 0 */
    midiHdr.dwFlags = 0;

    UINT err;
    /* Prepare the buffer and MIDIHDR */
    err = midiOutPrepareHeader(m_handle,  &midiHdr, sizeof(MIDIHDR));
    if (!err)
    {
        /* Output the SysEx message */
        err = midiOutLongMsg(m_handle, &midiHdr, sizeof(MIDIHDR));
        if (err)
            qDebug() << "Error while sending SysEx message";

        /* Unprepare the buffer and MIDIHDR */
        while (MIDIERR_STILLPLAYING == midiOutUnprepareHeader(m_handle, &midiHdr, sizeof(MIDIHDR)))
        {
            /* Should put a delay in here rather than a busy-wait */
        }
    }
}
