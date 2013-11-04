/*
  Q Light Controller
  midienumerator.h

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

#ifndef MIDIENUMERATOR_H
#define MIDIENUMERATOR_H

#include <QObject>
#include <QList>

class MidiEnumeratorPrivate;
class MidiOutputDevice;
class MidiInputDevice;

class MidiEnumerator : public QObject
{
    Q_OBJECT

public:
    MidiEnumerator(QObject* parent = 0);
    ~MidiEnumerator();

    void rescan();

    QList <MidiOutputDevice*> outputDevices() const;
    QList <MidiInputDevice*> inputDevices() const;

    MidiOutputDevice* outputDevice(const QVariant& uid) const;
    MidiInputDevice* inputDevice(const QVariant& uid) const;

signals:
    void configurationChanged();

private:
    MidiEnumeratorPrivate* d_ptr;
};

#endif
