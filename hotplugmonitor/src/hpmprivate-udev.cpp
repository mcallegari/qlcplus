/*
  Q Light Controller
  hpmprivate-udev.cpp

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

#include <sys/types.h>
#include <sys/time.h>
#include <libudev.h>
#include <unistd.h>
#include <errno.h>

#include <QStringList>
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
#define PROPERTY_PRODUCT     "PRODUCT"

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
    if (isRunning() == true)
    {
        m_run = false;
        wait();
    }
}

void HPMPrivate::run()
{
    udev *udev_ctx = udev_new();
    Q_ASSERT(udev_ctx != NULL);

    udev_monitor *mon = udev_monitor_new_from_netlink(udev_ctx, UDEV_NETLINK_SOURCE);
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
        tv.tv_sec = 1;
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
            udev_device *dev = udev_monitor_receive_device(mon);
            if (dev != NULL)
            {
                QString action = QString(udev_device_get_action(dev));
                QString vendor = QString(udev_device_get_property_value(dev, PROPERTY_VID));
                QString product = QString(udev_device_get_property_value(dev, PROPERTY_PID));

                // fallback to composite PRODUCT property VID/PID/REV
                if (vendor.isEmpty() && product.isEmpty())
                {
                    QString compProduct = QString(udev_device_get_property_value(dev, PROPERTY_PRODUCT));
                    QStringList tokens = compProduct.split("/");
                    if (tokens.count() >= 2)
                    {
                        vendor = tokens.at(0);
                        product = tokens.at(1);
                    }
                }
                if (action.isEmpty() || vendor.isEmpty() || product.isEmpty())
                {
                    qWarning() << Q_FUNC_INFO << "Unable to get device properties"
                               << (void*) dev << "Action:" << QString(action);
                }
                else if (action == QString(DEVICE_ACTION_ADD))
                {
                    uint vid = vendor.toUInt(0, 16);
                    uint pid = product.toUInt(0, 16);
                    HotPlugMonitor* hpm = qobject_cast<HotPlugMonitor*> (parent());
                    Q_ASSERT(hpm != NULL);
                    hpm->emitDeviceAdded(vid, pid);
                }
                else if (action == QString(DEVICE_ACTION_REMOVE))
                {
                    uint vid = vendor.toUInt(0, 16);
                    uint pid = product.toUInt(0, 16);
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
