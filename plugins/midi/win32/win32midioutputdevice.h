/*
  Q Light Controller
  win32midioutputdevice.h

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

#ifndef WIN32MIDIOUTPUTDEVICE_H
#define WIN32MIDIOUTPUTDEVICE_H

#include <Windows.h>
#include <QObject>

#include "midioutputdevice.h"

class Win32MidiOutputDevice : public MidiOutputDevice
{
    Q_OBJECT

public:
    Win32MidiOutputDevice(const QVariant& uid, const QString& name, UINT id, QObject* parent = 0);
    ~Win32MidiOutputDevice();

    void open();
    void close();
    bool isOpen() const;

    void writeChannel(ushort channel, uchar value);
    void writeUniverse(const QByteArray& universe);
    void writeFeedback(uchar cmd, uchar data1, uchar data2);

private:
    void sendData(BYTE command, BYTE channel, BYTE value);

private:
    UINT m_id;
    HMIDIOUT m_handle;
    QByteArray m_universe;
};

#endif
