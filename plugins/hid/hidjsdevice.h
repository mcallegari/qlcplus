/*
  Q Light Controller
  hidjsdevice.h

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

#ifndef HIDJSDEVICE_H
#define HIDJSDEVICE_H

#include <QObject>
#include <QFile>
#include <QHash>

#if defined(Q_WS_X11) || defined(Q_OS_LINUX)
  #include <sys/ioctl.h>
  #include <linux/input.h>
  #include <linux/types.h>
#elif defined(WIN32) || defined (Q_OS_WIN)
  #include <windows.h>
  #include <mmsystem.h>
  #include <regstr.h>
#endif

#include "hiddevice.h"

class HIDPlugin;

/*****************************************************************************
 * HIDEventDevice
 *****************************************************************************/

class HIDJsDevice : public HIDDevice
{
    Q_OBJECT

public:
    HIDJsDevice(HIDPlugin* parent, quint32 line, const QString& name, const QString& path);
    virtual ~HIDJsDevice();

#if defined(WIN32) || defined (Q_OS_WIN)
    static bool isJoystick(unsigned short vid, unsigned short pid);
#endif

protected:

#if defined(Q_WS_X11) || defined(Q_OS_LINUX)
    bool openDevice();
#endif

    /** Initialize the device, find out its capabilities etc. */
    void init();

    /** @reimp */
    bool hasInput() { return true; }

protected:
    unsigned char m_axesNumber;
    unsigned char m_buttonsNumber;
#if defined(WIN32) || defined (Q_OS_WIN)
    JOYCAPS m_caps;
    JOYINFOEX m_info;
    UINT m_windId;
    DWORD m_buttonsMask;
    QByteArray m_axesValues;
#endif
    /*********************************************************************
     * File operations
     *********************************************************************/
public:
    /** @reimp */
    bool openInput();

    /** @reimp */
    void closeInput();

    /** @reimp */
    QString path() const;

    /** @reimp */
    bool readEvent();

    /*********************************************************************
     * Device info
     *********************************************************************/
public:
    /** @reimp */
    QString infoText();

    /*********************************************************************
     * Input data
     *********************************************************************/
public:
    /** @reimp */
    void feedBack(quint32 channel, uchar value);

private:
    /** @reimp */
    void run();
};

#endif
