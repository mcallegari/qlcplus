/*
  Q Light Controller
  mididevice.cpp

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

#include <QSettings>
#include "mididevice.h"

#define SETTINGS_MIDICHANNEL "midiplugin/%1/midichannel"
#define SETTINGS_MODE "midiplugin/%1/mode"

#define NOTE_VELOCITY "Note Velocity"
#define CONTROL_CHANGE "Control Change"
#define PROGRAM_CHANGE "Program Change"

/****************************************************************************
 * Initialization
 ****************************************************************************/

MidiDevice::MidiDevice(const QVariant& uid, const QString& name, QObject* parent)
    : QObject(parent)
    , m_uid(uid)
    , m_name(name)
    , m_midiChannel(0)
    , m_mode(ControlChange)
{
    loadSettings();
}

MidiDevice::~MidiDevice()
{
    saveSettings();
}

QVariant MidiDevice::uid() const
{
    return m_uid;
}

QString MidiDevice::name() const
{
    return m_name;
}

/****************************************************************************
 * MIDI Channel
 ****************************************************************************/

void MidiDevice::setMidiChannel(int channel)
{
    m_midiChannel = channel;
}

int MidiDevice::midiChannel() const
{
    return m_midiChannel;
}

/****************************************************************************
 * Mode
 ****************************************************************************/

void MidiDevice::setMode(MidiDevice::Mode mode)
{
    m_mode = mode;
}

MidiDevice::Mode MidiDevice::mode() const
{
    return m_mode;
}

QString MidiDevice::modeToString(Mode mode)
{
    switch (mode)
    {
    default:
    case ControlChange:
        return QString(CONTROL_CHANGE);
        break;
    case Note:
        return QString(NOTE_VELOCITY);
        break;
    case ProgramChange:
        return QString(PROGRAM_CHANGE);
        break;
    }
}

MidiDevice::Mode MidiDevice::stringToMode(const QString& mode)
{
   if (mode == QString(NOTE_VELOCITY))
       return Note;
   else if (mode == QString(PROGRAM_CHANGE))
       return ProgramChange;
   else
       return ControlChange;
}

/****************************************************************************
 * Private API
 ****************************************************************************/

void MidiDevice::loadSettings()
{
    QSettings settings;

    QString key = QString(SETTINGS_MIDICHANNEL).arg(uid().toString());
    QVariant value = settings.value(key);
    if (value.isValid() == true)
        setMidiChannel(value.toInt());
    else
        setMidiChannel(0);

    key = QString(SETTINGS_MODE).arg(uid().toString());
    value = settings.value(key);
    if (value.isValid() == true)
        setMode(stringToMode(value.toString()));
    else
        setMode(ControlChange);
}

void MidiDevice::saveSettings() const
{
    QSettings settings;

    QString key = QString(SETTINGS_MIDICHANNEL).arg(uid().toString());
    settings.setValue(key, midiChannel());

    key = QString(SETTINGS_MODE).arg(uid().toString());
    settings.setValue(key, MidiDevice::modeToString(mode()));
}
