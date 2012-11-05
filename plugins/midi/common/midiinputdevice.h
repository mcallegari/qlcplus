/*
  Q Light Controller
  midiinputdevice.h

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

#ifndef MIDIINPUTDEVICE_H
#define MIDIINPUTDEVICE_H

#include "mididevice.h"

class MidiInputDevice : public MidiDevice
{
    Q_OBJECT

public:
    MidiInputDevice(const QVariant& uid, const QString& name, QObject* parent = 0);
    virtual ~MidiInputDevice();

    void emitValueChanged(uint channel, uchar value);

signals:
    void valueChanged(const QVariant& uid, ushort channel, uchar value);
};

#endif
