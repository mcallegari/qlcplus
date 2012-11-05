/*
  Q Light Controller
  hotplugmonitor.h

  Copyright (c) Heikki Junnila

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  Version 2 as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details. The license is
  in the file "COPYING".

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef HOTPLUGMONITOR_H
#define HOTPLUGMONITOR_H

#include <QObject>

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
