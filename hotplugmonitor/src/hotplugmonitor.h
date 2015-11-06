/*
  Q Light Controller
  hotplugmonitor.h

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

#ifndef HOTPLUGMONITOR_H
#define HOTPLUGMONITOR_H

#include <QObject>
#if defined(WIN32) || defined(Q_OS_WIN)
#  include <qwindowdefs.h>
#endif

class HPMPrivate;

/**
 * HotPlugMonitor monitors for USB subsystem hotplug events and emits either
 * deviceAdded() or deviceRemoved() signal, depending on which event has occurred.
 * This info can then be used by plugins to see if they need to update their
 * own device lists.
 */
class HotPlugMonitor : public QObject
{
    Q_OBJECT

    friend class HPMPrivate;

public:
    /** Connect $listener to receive deviceAdded() and deviceRemoved() signals */
    static void connectListener(QObject* listener);

#if defined(WIN32) || defined(Q_OS_WIN)
    static void setWinId(WId id);
    static bool parseWinEvent(void * message, long * result);
#endif

    /** Destructor */
    ~HotPlugMonitor();

signals:
    /** Emitted when a device with a specific VID/PID has been added to the system. */
    void deviceAdded(uint vid, uint pid);

    /** Emitted when a device with a specific VID/PID has been removed from the system. */
    void deviceRemoved(uint vid, uint pid);

private slots:
    /** Start receiving notifications. */
    void start();

    /** Stop sending hotplug notifications. */
    void stop();

private:
    HotPlugMonitor(QObject* parent);
    static HotPlugMonitor* instance();

    void emitDeviceAdded(uint vid, uint pid);
    void emitDeviceRemoved(uint vid, uint pid);

private:
    HPMPrivate* d_ptr;
    static HotPlugMonitor* s_instance;
};

#endif
