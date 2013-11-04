/*
  Q Light Controller
  hidpoller.cpp

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

#include <QMapIterator>
#include <linux/input.h>
#include <errno.h>
#include <QDebug>
#include <QMap>

#include "hiddevice.h"
#include "hidpoller.h"
#include "poll.h"
#include "hid.h"

#define KPollTimeout 1000

/*****************************************************************************
 * Initialization
 *****************************************************************************/

HIDPoller::HIDPoller(HID* parent)
    : QThread(parent)
{
    Q_ASSERT(parent != NULL);
    m_running = false;
}

HIDPoller::~HIDPoller()
{
    m_devices.clear();
    stop();
}

/*****************************************************************************
 * Polled devices
 *****************************************************************************/

bool HIDPoller::addDevice(HIDDevice* device)
{
    Q_ASSERT(device != NULL);

    m_mutex.lock();

    if (m_devices.contains(device->handle()) == true)
    {
        m_mutex.unlock();
        return false;
    }

    if (device->open() == true)
    {
        m_devices[device->handle()] = device;
        m_changed = true;
    }

    if (m_running == false)
    {
        m_running = true;
        start();
    }

    m_mutex.unlock();

    return true;
}

bool HIDPoller::removeDevice(HIDDevice* device)
{
    bool r = false;

    Q_ASSERT(device != NULL);

    m_mutex.lock();

    if (m_devices.remove(device->handle()) > 0)
    {
        device->close();
        m_changed = true;
        r = true;
    }

    m_mutex.unlock();

    return r;
}

/*****************************************************************************
 * Poller thread
 *****************************************************************************/

void HIDPoller::stop()
{
    m_running = false;
    wait();
}

void HIDPoller::run()
{
    struct pollfd* fds = NULL;
    int num = 0;
    int r;
    int i;

    m_mutex.lock();

    while (m_running == true)
    {
        /* If the list of polled devices has changed, reload all
           devices into the array of pollfd's */
        if (m_changed == true)
        {
            if (fds != NULL)
                delete [] fds;

            num = m_devices.count();
            if (num == 0)
                break;

            fds = new struct pollfd[num];
            memset(fds, 0, num);
            i = 0;

            QMapIterator<int, HIDDevice*> it(m_devices);
            while (it.hasNext() == true)
            {
                it.next();
                fds[i].fd = it.key();
                fds[i].events = POLLIN;
                i++;
            }

            m_changed = false;
        }

        m_mutex.unlock();
        r = poll(fds, num, KPollTimeout);
        m_mutex.lock();

        if (r < 0 && errno != EINTR)
        {
            /* Print abnormal errors. EINTR may happen often. */
            perror("poll");
        }
        else if (r != 0)
        {
            /* If the device map has changed, we can't trust
               that any of the devices are valid. */
            if (m_changed == false)
            {
                for (i = 0; i < num; i++)
                {
                    if (fds[i].revents != 0)
                        readEvent(fds[i]);
                }
            }
        }
    }

    m_running = false;
    m_mutex.unlock();
}

void HIDPoller::readEvent(struct pollfd pfd)
{
    HIDDevice* device = m_devices[pfd.fd];
    Q_ASSERT(device != NULL);

    if (device->readEvent() == false)
    {
        if (m_devices.remove(device->handle()) > 0)
        {
            device->close();
            m_changed = true;
        }
    }
}
