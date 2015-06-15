/*
  Q Light Controller Plus
  hidosxjoystick.h

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


#ifndef HIDOSXJOYSTICK_H
#define HIDOSXJOYSTICK_H

#include "hidjsdevice.h"

#include <IOKit/hid/IOHIDManager.h>
#include <IOKit/hid/IOHIDKeys.h>
#include <CoreFoundation/CoreFoundation.h>

typedef struct
{
    IOHIDElementRef IOHIDRef;
    int value;
} HIDInfo;

class HIDOSXJoystick: public HIDJsDevice
{
    Q_OBJECT
public:
    HIDOSXJoystick(HIDPlugin* parent, quint32 line, struct hid_device_info *info);

    static bool isJoystick(unsigned short usage);

    /** @reimp */
    void init();

    /** @reimp */
    bool readEvent();

private:
    IOHIDManagerRef m_HIDManager;
    IOHIDDeviceRef m_IOKitDevice;

    QList<HIDInfo> m_axesValues;
    QList<HIDInfo> m_buttonsValues;
};

#endif // HIDOSXJOYSTICK_H
