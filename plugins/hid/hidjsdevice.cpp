/*
  Q Light Controller
  hidjsdevice.cpp

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

#include <QApplication>
#include <QObject>
#include <QString>
#include <QDebug>
#include <QFile>

#if defined(Q_WS_X11) || defined(Q_OS_LINUX)
  #include <linux/joystick.h>
  #include <linux/input.h>
  #include <errno.h>
  #include <unistd.h>
  #include <poll.h>
#endif

#include "hidjsdevice.h"
#include "qlcmacros.h"
#include "hid.h"

#define KPollTimeout 1000

HIDJsDevice::HIDJsDevice(HID* parent, quint32 line, const QString &name, const QString& path)
    : HIDDevice(parent, line, name, path)
{
    m_capabilities = QLCIOPlugin::Input;
    init();
}

HIDJsDevice::~HIDJsDevice()
{

}

void HIDJsDevice::init()
{
    if (openInput() == false)
        return;

#if defined(Q_WS_X11) || defined(Q_OS_LINUX)
    /* Number of axes */
    if (ioctl(m_file.handle(), JSIOCGAXES, &m_axes) < 0)
    {
        m_axes = 0;
        qWarning() << "Unable to get number of axes:"
                   << strerror(errno);
    }

    /* Number of buttons */
    if (ioctl(m_file.handle(), JSIOCGBUTTONS, &m_buttons) < 0)
    {
        m_buttons = 0;
        qWarning() << "Unable to get number of buttons:"
                   << strerror(errno);
    }
#endif
    closeInput();
}

/*****************************************************************************
 * File operations
 *****************************************************************************/

bool HIDJsDevice::openInput()
{
    bool result = false;

    result = m_file.open(QIODevice::Unbuffered | QIODevice::ReadWrite);
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

    m_running = true;
    start();

    return result;
}

void HIDJsDevice::closeInput()
{
    if (isRunning() == true)
    {
        m_running = false;
        wait();
    }
    m_file.close();
}

QString HIDJsDevice::path() const
{
    return m_file.fileName();
}

bool HIDJsDevice::readEvent()
{

#if defined(Q_WS_X11) || defined(Q_OS_LINUX)
    struct js_event ev;
    HIDInputEvent* e;
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
            ch = quint32(m_axes + ev.number);

            /* Generate and post an event */
            e = new HIDInputEvent(this, m_line, ch, val, true);
            QApplication::postEvent(parent(), e);
        }
        else if ((ev.type & ~JS_EVENT_INIT) == JS_EVENT_AXIS)
        {
            val = SCALE(double(ev.value), double(SHRT_MIN), double(SHRT_MAX),
                        double(0), double(UCHAR_MAX));
            ch = quint32(ev.number);

            e = new HIDInputEvent(this, m_line, ch, val, true);
            QApplication::postEvent(parent(), e);
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
        e = new HIDInputEvent(this, 0, 0, 0, false);
        QApplication::postEvent(parent(), e);

        return false;
    }
#else
    return false;
#endif
}

/*****************************************************************************
 * Device info
 *****************************************************************************/

QString HIDJsDevice::infoText()
{
    QString info;

    info += QString("<B>%1</B><P>").arg(m_name);
    info += tr("Axes: %1").arg(m_axes);
    info += QString("<BR/>");
    info += tr("Buttons: %1").arg(m_buttons);
    info += QString("</P>");

    return info;
}

/*****************************************************************************
 * Input data
 *****************************************************************************/

void HIDJsDevice::feedBack(quint32 channel, uchar value)
{
    /* HID devices don't (yet) support feedback */
    Q_UNUSED(channel);
    Q_UNUSED(value);
}

void HIDJsDevice::run()
{
#if defined(Q_WS_X11) || defined(Q_OS_LINUX)
    struct pollfd* fds = NULL;
    fds = new struct pollfd[1];
    memset(fds, 0, 1);

    fds[0].fd = handle();
    fds[0].events = POLLIN;
#endif

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

