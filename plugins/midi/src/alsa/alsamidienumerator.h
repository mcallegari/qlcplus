/*
  Q Light Controller
  alsamidienumeratorprivate.h

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
