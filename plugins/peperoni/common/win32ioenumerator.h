/*
  Q Light Controller
  win32ioenumerator.h

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

#ifndef WIN32IOENUMERATOR_H
#define WIN32IOENUMERATOR_H

#include <Windows.h>
#include <QVariant>
#include <QObject>

#include "ioenumerator.h"

struct usbdmx_functions;

class Win32IOEnumerator : public IOEnumerator
{
    Q_OBJECT

public:
    Win32IOEnumerator(QObject* parent = 0);
    ~Win32IOEnumerator();

    void rescan();

    QVariant extractUID(HANDLE handle);
    QString extractName(HANDLE handle);

    QList <OutputDevice*> outputDevices() const;
    QList <InputDevice*> inputDevices() const;

    OutputDevice* outputDevice(const QVariant& uid) const;
    InputDevice* inputDevice(const QVariant& uid) const;

signals:
    void configurationChanged();

private:
    struct usbdmx_functions* m_usbdmx;
    QList <OutputDevice*> m_outputDevices;
    QList <InputDevice*> m_inputDevices;
};

#endif
