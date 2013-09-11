/*
  Q Light Controller
  mididevice.h

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

#ifndef MIDIDEVICE_H
#define MIDIDEVICE_H

#include <QVariant>
#include <QObject>

class MidiDevice : public QObject
{
    Q_OBJECT

    /************************************************************************
     * Initialization
     ************************************************************************/
public:
    MidiDevice(const QVariant& uid, const QString& name, QObject* parent = 0);
    virtual ~MidiDevice();

    QVariant uid() const;
    QString name() const;

private:
    void loadSettings();
    void saveSettings() const;

private:
    const QVariant m_uid;
    const QString m_name;

    /************************************************************************
     * MIDI Channel
     ************************************************************************/
public:
    void setMidiChannel(int channel);
    int midiChannel() const;

private:
    int m_midiChannel;

    /************************************************************************
     * Mode
     ************************************************************************/
public:
    enum Mode { ControlChange, Note, ProgramChange };

    void setMode(Mode mode);
    Mode mode() const;

    static QString modeToString(Mode mode);
    static Mode stringToMode(const QString& mode);

private:
    Mode m_mode;

    /************************************************************************
     * Virtual Open/Close
     ************************************************************************/
public:
    virtual void open() = 0;
    virtual void close() = 0;
    virtual bool isOpen() const = 0;
};

#endif
