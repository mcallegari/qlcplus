/*
  Q Light Controller
  unixioenumerator.h

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

#ifndef UNIXIOENUMERATOR_H
#define UNIXIOENUMERATOR_H

#include <QVariant>
#include <QObject>
#include "ioenumerator.h"

struct libusb_device;

class UnixIOEnumerator : public IOEnumerator
{
    Q_OBJECT

public:
    UnixIOEnumerator(QObject* parent = 0);
    ~UnixIOEnumerator();

    void rescan();

    QList <OutputDevice*> outputDevices() const;
    QList <InputDevice*> inputDevices() const;

    OutputDevice* outputDevice(const QVariant& uid) const;
    InputDevice* inputDevice(const QVariant& uid) const;

signals:
    void configurationChanged();

private:
    void extractData(struct libusb_device* device,
                     struct libusb_device_descriptor* desc,
                     QVariant* uid, QString* name);

    static bool isPeperoniDevice(const struct libusb_device_descriptor* descriptor);

private:
    struct libusb_context* m_ctx;
    QList <OutputDevice*> m_outputDevices;
    QList <InputDevice*> m_inputDevices;
};

#endif
