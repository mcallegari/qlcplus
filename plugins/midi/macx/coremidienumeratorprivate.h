/*
  Q Light Controller
  coremidienumeratorprivate.h

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

#ifndef COREMIDIENUMERATORPRIVATE_H
#define COREMIDIENUMERATORPRIVATE_H

#include <CoreFoundation/CoreFoundation.h>
#include <CoreMIDI/CoreMIDI.h>
#include <QObject>
#include <QList>

class MidiOutputDevice;
class MidiInputDevice;
class MidiEnumerator;

class MidiEnumeratorPrivate : public QObject
{
    Q_OBJECT

public:
    MidiEnumeratorPrivate(MidiEnumerator* parent);
    ~MidiEnumeratorPrivate();

    void rescan();

    static QString extractName(MIDIEntityRef entity);
    static QVariant extractUID(MIDIEntityRef entity);

    MidiOutputDevice* outputDevice(const QVariant& uid) const;
    MidiInputDevice* inputDevice(const QVariant& uid) const;

    QList <MidiOutputDevice*> outputDevices() const;
    QList <MidiInputDevice*> inputDevices() const;

signals:
    void configurationChanged();

private:
    MIDIClientRef m_client;
    QList <MidiOutputDevice*> m_outputDevices;
    QList <MidiInputDevice*> m_inputDevices;
};

#endif
