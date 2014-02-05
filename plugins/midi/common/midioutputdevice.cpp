/*
  Q Light Controller
  midioutputdevice.cpp

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

#include <QDebug>
#include "midioutputdevice.h"

MidiOutputDevice::MidiOutputDevice(const QVariant& uid, const QString& name, QObject* parent)
    : MidiDevice(uid, name, Output, parent)
{
    //qDebug() << Q_FUNC_INFO;
}

MidiOutputDevice::~MidiOutputDevice()
{
    //qDebug() << Q_FUNC_INFO;
}
