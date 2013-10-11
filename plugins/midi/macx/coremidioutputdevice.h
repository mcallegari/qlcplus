/*
  Q Light Controller
  coremidioutputdevice.h

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

#ifndef COREMIDIOUTPUTDEVICE_H
#define COREMIDIOUTPUTDEVICE_H

#include <CoreFoundation/CoreFoundation.h>
#include <CoreMIDI/CoreMIDI.h>

#include "midioutputdevice.h"

class CoreMidiOutputDevice : public MidiOutputDevice
{
public:
    CoreMidiOutputDevice(const QVariant& uid, const QString& name,
                         MIDIEntityRef entity, MIDIClientRef client,
                         QObject* parent);
    virtual ~CoreMidiOutputDevice();

    void open();
    void close();
    bool isOpen() const;

    void writeChannel(ushort channel, uchar value);
    void writeUniverse(const QByteArray& universe);
    void writeFeedback(uchar cmd, uchar data1, uchar data2);

private:
    MIDIClientRef m_client;
    MIDIEntityRef m_entity;
    MIDIPortRef m_outPort;
    MIDIEndpointRef m_destination;
    QByteArray m_universe;
};

#endif
