/*
  Q Light Controller
  win32midiinputdevice.cpp

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

#include "win32midiinputdevice.h"
#include "midiprotocol.h"

extern "C" {
static void CALLBACK MidiInProc(HMIDIIN hMidiIn, UINT wMsg,
                                DWORD_PTR dwInstance, DWORD_PTR dwParam1,
                                DWORD_PTR dwParam2)
{
    Win32MidiInputDevice* self = (Win32MidiInputDevice*) dwInstance;
    Q_ASSERT(self != NULL);

    if (wMsg == MIM_DATA)
    {
        BYTE cmd = dwParam1 & 0xFF;
        BYTE data1 = (dwParam1 & 0xFF00) >> 8;
        BYTE data2 = (dwParam1 & 0xFF0000) >> 16;
        quint32 channel = 0;
        uchar value = 0;

        if (QLCMIDIProtocol::midiToInput(cmd, data1, data2,
            uchar(self->midiChannel()), &channel, &value) == true)
        {/*
            if (channel == CHANNEL_OFFSET_MBC)
                if (self->incrementMBCCount() == false)
                    return;
*/
            self->emitValueChanged(channel, value);
        }
    }
}
}

Win32MidiInputDevice::Win32MidiInputDevice(const QVariant& uid, const QString& name, UINT id,
                                           QObject* parent)
    : MidiInputDevice(uid, name, parent)
    , m_id(id)
    , m_handle(NULL)
    , m_universe(MAX_MIDI_DMX_CHANNELS, char(0))
{
    qDebug() << Q_FUNC_INFO;
}

Win32MidiInputDevice::~Win32MidiInputDevice()
{
    qDebug() << Q_FUNC_INFO;
}

void Win32MidiInputDevice::open()
{
    qDebug() << Q_FUNC_INFO;

    if (m_handle != NULL)
        return;

    MMRESULT result = midiInOpen(&m_handle, m_id, (DWORD_PTR) MidiInProc, (DWORD_PTR) this, CALLBACK_FUNCTION);
    if (result != MMSYSERR_NOERROR)
    {
        qWarning() << Q_FUNC_INFO << "Unable to open MIDI input device with id:" << m_id
                   << "name:" << name() << ":" << result;
        m_handle = NULL;
    }
    else
    {
        midiInStart(m_handle);
    }
}

void Win32MidiInputDevice::close()
{
    qDebug() << Q_FUNC_INFO;

    if (m_handle == NULL)
        return;

    MMRESULT result = midiInClose(m_handle);
    if (result != MMSYSERR_NOERROR)
        qWarning() << Q_FUNC_INFO << "Unable to close MIDI input with id:" << m_id
                   << "name:" << name() << ":" << result;
    m_handle = NULL;
}

bool Win32MidiInputDevice::isOpen() const
{
    if (m_handle != NULL)
        return true;
    else
        return false;
}
