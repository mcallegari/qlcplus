/*
  Q Light Controller
  hpmprivate-udev.cpp

  Copyright (C) Heikki Junnila

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

#include <sys/types.h>
#include <sys/time.h>
#include <libudev.h>
#include <unistd.h>
#include <errno.h>
#include <QDebug>

#include "hpmprivate-udev.h"
#include "hotplugmonitor.h"

#define DEVICE_ACTION_ADD    "add"
#define DEVICE_ACTION_REMOVE "remove"
#define UDEV_NETLINK_SOURCE  "udev"
#define USB_SUBSYSTEM        "usb"
#define USB_DEVICE_TYPE      "usb_device"
#define PROPERTY_VID         "ID_VENDOR_ID"
#define PROPERTY_PID         "ID_MODEL_ID"

HPMPrivate::HPMPrivate(HotPlugMonitor* parent)
    : QThread(parent)
    , m_run(false)
{
    Q_ASSERT(parent != NULL);
}

HPMPrivate::~HPMPrivate()
{
    if (isRunning() == true)
        stop();
}

void HPMPrivate::stop()
{
    m_run = false;
    while (isRunning() == true)
        usleep(10);
}

void HPMPrivate::run()
{
    udev* udev_ctx = udev_new();
    Q_ASSERT(udev_ctx != NULL);

    udev_monitor* mon = udev_monitor_new_from_netlink(udev_ctx, UDEV_NETLINK_SOURCE);
    Q_ASSERT(mon != NULL);

    if (udev_monitor_filter_add_match_subsystem_devtype(mon, USB_SUBSYSTEM, USB_DEVICE_TYPE) < 0)
    {
        qWarning() << Q_FUNC_INFO << "Unable to add match for USB devices";
        udev_monitor_unref(mon);
        udev_unref(udev_ctx);
        return;
    }

    if (udev_monitor_enable_receiving(mon) < 0)
    {
        qWarning() << Q_FUNC_INFO << "Unable to enable udev uevent reception";
        udev_monitor_unref(mon);
        udev_unref(udev_ctx);
        return;
    }

    int fd = udev_monitor_get_fd(mon);
    fd_set readfs;
    FD_ZERO(&readfs);

    m_run = true;
    while (m_run == true)
    {
        struct timeval tv;
        tv.tv_sec = 5;
        tv.tv_usec = 0;

        FD_SET(fd, &readfs);
        int retval = select(fd + 1, &readfs, NULL, NULL, &tv);
        if (retval == -1)
        {
            qWarning() << Q_FUNC_INFO << strerror(errno);
            m_run = false;
        }
        else if (retval > 0 && FD_ISSET(fd, &readfs))
        {
            udev_device* dev = udev_monitor_receive_device(mon);
            if (dev != NULL)
            {
                const char* action = udev_device_get_action(dev);
                const char* vendor = udev_device_get_property_value(dev, PROPERTY_VID);
                const char* product = udev_device_get_property_value(dev, PROPERTY_PID);
                if (action == NULL || vendor == NULL || product == NULL)
                {
                    qWarning() << Q_FUNC_INFO << "Unable to get device properties"
                               << (void*) dev;
                }
                else if (strcmp(action, DEVICE_ACTION_ADD) == 0)
                {
                    uint vid = QString(vendor).toUInt(0, 16);
                    uint pid = QString(product).toUInt(0, 16);
                    HotPlugMonitor* hpm = qobject_cast<HotPlugMonitor*> (parent());
                    Q_ASSERT(hpm != NULL);
                    hpm->emitDeviceAdded(vid, pid);
                }
                else if (strcmp(action, DEVICE_ACTION_REMOVE) == 0)
                {
                    uint vid = QString(vendor).toUInt(0, 16);
                    uint pid = QString(product).toUInt(0, 16);
                    HotPlugMonitor* hpm = qobject_cast<HotPlugMonitor*> (parent());
                    Q_ASSERT(hpm != NULL);
                    hpm->emitDeviceRemoved(vid, pid);
                }
                else
                {
                    qWarning() << Q_FUNC_INFO << "Unhandled udev action:" << action;
                }

                udev_device_unref(dev);
            }
        }
    }

    udev_monitor_unref(mon);
    udev_unref(udev_ctx);
}
