/*
  Q Light Controller Plus
  hidwindowsjoystick.h

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

#ifndef HIDWINDOWSJOYSTICK_H
#define HIDWINDOWSJOYSTICK_H

#include <windows.h>
#include <mmsystem.h>

#include "hidjsdevice.h"

class HIDWindowsJoystick: public HIDJsDevice
{
    Q_OBJECT
public:
    HIDWindowsJoystick(HIDPlugin *parent, quint32 line, hid_device_info *info);

    static bool isJoystick(unsigned short vid, unsigned short pid);

    /** @reimp */
    void init();

    /** @reimp */
    bool readEvent();

protected:
    JOYCAPS m_caps;
    JOYINFOEX m_info;
    UINT m_windId;
    DWORD m_buttonsMask;
    QByteArray m_axesValues;
};

#endif // HIDWINDOWSJOYSTICK_H
