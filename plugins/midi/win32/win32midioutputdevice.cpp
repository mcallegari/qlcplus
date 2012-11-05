/*
  Q Light Controller
  win32midioutputdevice.cpp

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

void Win32MidiOutputDevice::open()
{
    qDebug() << Q_FUNC_INFO;

    if (m_handle != NULL)
        return;

    MMRESULT result = midiOutOpen(&m_handle, m_id, 0, 0, 0);
    if (result != MMSYSERR_NOERROR)
    {
        qWarning() << Q_FUNC_INFO << "Unable to open MIDI output device with id:" << m_id
                   << "name:" << name() << ":" << result;
        m_handle = NULL;
    }
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
    if (channel < m_universe.size())
    {
        QByteArray ua(m_universe);
        ua[channel] = value;
        writeUniverse(ua);
    }
}

void Win32MidiOutputDevice::writeUniverse(const QByteArray& universe)
{
    for (BYTE channel = 0; channel < MAX_MIDI_DMX_CHANNELS &&
                           channel < universe.size(); channel++)
    {
        /* Scale 0-255 to 0-127 */
        BYTE scaled = DMX2MIDI(universe[channel]);

        /* Since MIDI is so slow, we only send values that are
           actually changed. */
        if (m_universe[channel] == (char) scaled)
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
        else
        {
            /* Control change */
            sendData(MIDI_CONTROL_CHANGE | (BYTE) midiChannel(), channel, scaled);
        }
    }
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
