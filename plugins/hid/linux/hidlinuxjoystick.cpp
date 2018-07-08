/*
  Q Light Controller Plus
  hidlinuxjoystick.cpp

  Copyright (c) Massimo Callegari

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

#include <QDebug>

#include <linux/joystick.h>
#include <linux/input.h>
#include <errno.h>
#include <unistd.h>
#include <poll.h>

#include "hidlinuxjoystick.h"
#include "hidplugin.h"

#define KPollTimeout 1000

HIDLinuxJoystick::HIDLinuxJoystick(HIDPlugin* parent, quint32 line, struct hid_device_info *info)
    : HIDJsDevice(parent, line, info)
{
    init();
}

bool HIDLinuxJoystick::openDevice()
{
    bool result = m_file.open(QIODevice::Unbuffered | QIODevice::ReadWrite);
    if (result == false)
    {
        result = m_file.open(QIODevice::Unbuffered |
                             QIODevice::ReadOnly);
        if (result == false)
        {
            qWarning() << "Unable to open" << m_file.fileName()
                       << ":" << m_file.errorString();
        }
        else
        {
            qDebug() << "Opened" << m_file.fileName()
                     << "in read only mode";
        }
    }
    return result;
}

void HIDLinuxJoystick::init()
{
    if (openDevice() == false)
        return;

    /* Number of axes */
    if (ioctl(m_file.handle(), JSIOCGAXES, &m_axesNumber) < 0)
    {
        m_axesNumber = 0;
        qWarning() << "Unable to get number of axes:"
                   << strerror(errno);
    }

    /* Number of buttons */
    if (ioctl(m_file.handle(), JSIOCGBUTTONS, &m_buttonsNumber) < 0)
    {
        m_buttonsNumber = 0;
        qWarning() << "Unable to get number of buttons:"
                   << strerror(errno);
    }

    closeInput();
}

bool HIDLinuxJoystick::openInput()
{
    bool result = openDevice();

    if (result == true)
    {
        m_running = true;
        start();
    }

    return result;
}

bool HIDLinuxJoystick::readEvent()
{
    struct js_event ev;
    int r;

    r = read(m_file.handle(), &ev, sizeof(struct js_event));
    if (r > 0)
    {
        quint32 ch;
        uchar val;

        /* Get the event type */
        if ((ev.type & ~JS_EVENT_INIT) == JS_EVENT_BUTTON)
        {
            if (ev.value != 0)
                val = UCHAR_MAX;
            else
                val = 0;

            /* Map button channels to start after axes */
            ch = quint32(m_axesNumber + ev.number);

            /* Generate and post an event */
            emit valueChanged(UINT_MAX, m_line, ch, val);
        }
        else if ((ev.type & ~JS_EVENT_INIT) == JS_EVENT_AXIS)
        {
            val = SCALE(double(ev.value), double(SHRT_MIN), double(SHRT_MAX),
                        double(0), double(UCHAR_MAX));
            ch = quint32(ev.number);

            qDebug() << "HID JS" << m_line << ch << val;
            emit valueChanged(UINT_MAX, m_line, ch, val);
        }
        else
        {
            /* Unknown event type */
        }

        return true;
    }
    else
    {
        /* This device seems to be dead */
        /*
        e = new HIDInputEvent(this, 0, 0, 0, false);
        QApplication::postEvent(parent(), e);
        */
        return false;
    }
}

void HIDLinuxJoystick::run()
{
    struct pollfd* fds = NULL;
    fds = new struct pollfd[1];
    memset(fds, 0, 1);

    fds[0].fd = handle();
    fds[0].events = POLLIN;


    while (m_running == true)
    {
        int r = poll(fds, 1, KPollTimeout);

        if (r < 0 && errno != EINTR)
        {
            /* Print abnormal errors. EINTR may happen often. */
            perror("poll");
        }
        else if (r != 0)
        {
            if (fds[0].revents != 0)
                readEvent();
        }
    }
}


