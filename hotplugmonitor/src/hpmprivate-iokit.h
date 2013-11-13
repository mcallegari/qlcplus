/*
  Q Light Controller
  hpmprivate-iokit.h

  Copyright (C) Heikki Junnila

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

#ifndef HPMPRIVATE_IOKIT_H
#define HPMPRIVATE_IOKIT_H

#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/usb/IOUSBLib.h>
#include <IOKit/IOCFPlugIn.h>
#include <mach/mach_port.h>

#include <QThread>

class HotPlugMonitor;

class HPMPrivate : public QThread
{
    Q_OBJECT

    friend void onHPMPrivateRawDeviceAdded(void* refCon, io_iterator_t iterator);
    friend void onHPMPrivateRawDeviceRemoved(void* refCon, io_iterator_t iterator);

public:
    HPMPrivate(HotPlugMonitor* parent);
    ~HPMPrivate();

public slots:
    void stop();

private:
    void extractVidPid(io_service_t usbDevice, UInt16* vid, UInt16* pid);
    void deviceAdded(io_iterator_t iterator);
    void deviceRemoved(io_iterator_t iterator);
    void run();

private:
    bool m_run;
    CFRunLoopRef loop;
};

#endif
