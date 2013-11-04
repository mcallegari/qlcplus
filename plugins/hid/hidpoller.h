/*
  Q Light Controller
  hidpoller.h

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

#ifndef HIDPOLLER_H
#define HIDPOLLER_H

#include <QThread>
#include <QMutex>
#include <QMap>

class HIDDevice;
class HID;

class HIDPoller : public QThread
{
    Q_OBJECT

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    HIDPoller(HID* parent);
    ~HIDPoller();

    /*********************************************************************
     * Polled devices
     *********************************************************************/
public:
    bool addDevice(HIDDevice* device);
    bool removeDevice(HIDDevice* device);

protected:
    QMap <int, HIDDevice*> m_devices;
    bool m_changed;
    QMutex m_mutex;

    /*********************************************************************
     * Poller thread
     *********************************************************************/
public:
    virtual void stop();

protected:
    virtual void run();
    void readEvent(struct pollfd pfd);

protected:
    bool m_running;
};

#endif
