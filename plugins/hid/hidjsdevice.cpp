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
#elif defined(WIN32) || defined (Q_OS_WIN)
  #define JOY_BUTTON_MASK(n) (1 << n)
#endif

#include "hidjsdevice.h"
#include "qlcmacros.h"
#include "hidplugin.h"

#define KPollTimeout 1000

HIDJsDevice::HIDJsDevice(HIDPlugin* parent, quint32 line, const QString &name, const QString& path)
    : HIDDevice(parent, line, name, path)
{
    m_capabilities = QLCIOPlugin::Input;
    init();
}

HIDJsDevice::~HIDJsDevice()
{

}

#if defined(WIN32) || defined (Q_OS_WIN)
bool HIDJsDevice::isJoystick(unsigned short vid, unsigned short pid)
{
    JOYCAPS caps;
    JOYINFO joyInfo;

    for (UINT i = 0; i < joyGetNumDevs(); i++)
    {
        memset( &caps, 0, sizeof( JOYCAPS ) );

        MMRESULT error = joyGetDevCapsW( i, &caps, sizeof(JOYCAPS));
        if (error == JOYERR_NOERROR && vid == caps.wMid && pid == caps.wPid)
        {
            if( joyGetPos(i, & joyInfo) == JOYERR_NOERROR )
                return true;
        }
    }
    return false;
}
#endif

#if defined(Q_WS_X11) || defined(Q_OS_LINUX)
bool HIDJsDevice::openDevice()
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
#endif

void HIDJsDevice::init()
{
#if defined(Q_WS_X11) || defined(Q_OS_LINUX)
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

#elif defined(WIN32) || defined (Q_OS_WIN)

    m_info.dwFlags = JOY_RETURNALL;
    m_info.dwSize  = sizeof( m_info );

    QString devPath = path();
    bool ok;
    unsigned short VID = devPath.mid(devPath.indexOf("vid_") + 4, 4).toUShort(&ok, 16);
    unsigned short PID = devPath.mid(devPath.indexOf("pid_") + 4, 4).toUShort(&ok, 16);

    for (UINT i = 0; i < joyGetNumDevs(); i++)
    {
        memset( &m_caps, 0, sizeof( m_caps ) );

        MMRESULT error = joyGetDevCapsW( i, &m_caps, sizeof(JOYCAPS));

        if (error == JOYERR_NOERROR && VID == m_caps.wMid && PID == m_caps.wPid)
        {
            /* Windows joystick drivers may provide any combination of
             * X,Y,Z,R,U,V,POV - not necessarily the first n of these.
             */
            if( m_caps.wCaps & JOYCAPS_HASV )
            {
                m_axesNumber = 6;
                //joy->min[ 7 ] = -1.0; joy->max[ 7 ] = 1.0;  /* POV Y */
                //joy->min[ 6 ] = -1.0; joy->max[ 6 ] = 1.0;  /* POV X */
            }
            else
                m_axesNumber = m_caps.wNumAxes;

            m_buttonsNumber = m_caps.wNumButtons;
            m_axesValues.fill(0, m_axesNumber);
            m_windId = i;
            break;
        }
        else
        {
            m_axesNumber = 0;
            m_buttonsNumber = 0;
            m_windId = -1;
        }
    }
#endif
}

/*****************************************************************************
 * File operations
 *****************************************************************************/

bool HIDJsDevice::openInput()
{
    bool result = false;
#if defined(Q_WS_X11) || defined(Q_OS_LINUX)
    result = openDevice();
#elif defined(WIN32) || defined (Q_OS_WIN)
    // nothing to do on dumb Windows
    result = true;
#endif

    if (result == true)
    {
        m_running = true;
        start();
    }

    return result;
}

void HIDJsDevice::closeInput()
{
    if (isRunning() == true)
    {
        m_running = false;
        wait();
    }
    if (m_file.isOpen())
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
#elif defined(WIN32) || defined (Q_OS_WIN)
    MMRESULT status = joyGetPosEx( m_windId, &m_info );

    if ( status != JOYERR_NOERROR )
        return false;

    if ( m_buttonsNumber )
    {
        for (int i = 0; i < m_buttonsNumber; ++i)
        {
            if ((m_info.dwButtons & JOY_BUTTON_MASK(i)) !=
                (m_buttonsMask & JOY_BUTTON_MASK(i)))
            {
                if (m_info.dwButtons & JOY_BUTTON_MASK(i))
                    emit valueChanged(UINT_MAX, m_line, m_axesNumber + i, 255);
                else
                    emit valueChanged(UINT_MAX, m_line, m_axesNumber + i, 0);
            }
        }
        m_buttonsMask = m_info.dwButtons;
    }

    if ( m_axesNumber )
    {
        QList<DWORD> cmpVals;
        cmpVals.append(m_info.dwXpos);
        cmpVals.append(m_info.dwYpos);
        cmpVals.append(m_info.dwZpos);
        cmpVals.append(m_info.dwRpos);
        cmpVals.append(m_info.dwUpos);
        cmpVals.append(m_info.dwVpos);

        for (int i = 0; i < m_axesNumber; i++)
        {
            uchar val = SCALE(double(cmpVals.at(i)), double(0), double(USHRT_MAX),
                        double(0), double(UCHAR_MAX));
            if (val != (uchar)m_axesValues.at(i))
                emit valueChanged(UINT_MAX, m_line, i, val);
            m_axesValues[i] = val;
        }
    }
    return true;
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
    info += tr("Axes: %1").arg(m_axesNumber);
    info += QString("<BR/>");
    info += tr("Buttons: %1").arg(m_buttonsNumber);
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
#if defined(Q_WS_X11) || defined(Q_OS_LINUX)
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
#elif defined(WIN32) || defined (Q_OS_WIN)
        readEvent();
        Sleep(50);
#endif
    }
}

