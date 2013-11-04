/*
  Q Light Controller
  ioenumerator.h

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

#ifndef IOENUMERATOR_H
#define IOENUMERATOR_H

#include <QObject>
#include <QList>

class OutputDevice;
class InputDevice;

class IOEnumerator : public QObject
{
    Q_OBJECT

public:
    IOEnumerator(QObject* parent = 0);
    virtual ~IOEnumerator();

    virtual void rescan() = 0;

    virtual QList <OutputDevice*> outputDevices() const = 0;
    virtual QList <InputDevice*> inputDevices() const = 0;

    virtual OutputDevice* outputDevice(const QVariant& uid) const = 0;
    virtual InputDevice* inputDevice(const QVariant& uid) const = 0;

signals:
    void configurationChanged();
};

#endif
