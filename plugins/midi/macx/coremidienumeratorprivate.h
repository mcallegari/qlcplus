/*
  Q Light Controller
  coremidienumeratorprivate.h

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

    static QString extractName(MIDIEndpointRef endpoint);
    static QVariant extractEndpointUID(MIDIEndpointRef endpoint);
    static QVariant extractEntityUID(MIDIEntityRef entity);

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
