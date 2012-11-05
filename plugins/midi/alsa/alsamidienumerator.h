/*
  Q Light Controller
  alsamidienumeratorprivate.h

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

#ifndef ALSAMIDIENUMERATORPRIVATE_H
#define ALSAMIDIENUMERATORPRIVATE_H

#include <QObject>
#include <QList>

class AlsaMidiInputThread;
class MidiOutputDevice;
class MidiInputDevice;
class MidiEnumerator;

struct _snd_seq;
typedef _snd_seq snd_seq_t;

struct snd_seq_addr;
typedef snd_seq_addr snd_seq_addr_t;

class MidiEnumeratorPrivate : public QObject
{
    Q_OBJECT

public:
    MidiEnumeratorPrivate(MidiEnumerator* parent);
    ~MidiEnumeratorPrivate();

    void initAlsa();
    void rescan();

    MidiOutputDevice* outputDevice(const QVariant& uid) const;
    MidiInputDevice* inputDevice(const QVariant& uid) const;

    QList <MidiOutputDevice*> outputDevices() const;
    QList <MidiInputDevice*> inputDevices() const;

signals:
    void configurationChanged();

private:
    snd_seq_t* m_alsa;
    snd_seq_addr_t* m_address;

    QList <MidiOutputDevice*> m_outputDevices;
    QList <MidiInputDevice*> m_inputDevices;

    AlsaMidiInputThread* m_inputThread;
};

#endif
