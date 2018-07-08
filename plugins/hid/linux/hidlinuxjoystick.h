/*
  Q Light Controller Plus
  hidlinuxjoystick.h

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

#ifndef HIDLINUXJOYSTICK_H
#define HIDLINUXJOYSTICK_H

#include <sys/ioctl.h>
#include <linux/input.h>
#include <linux/types.h>

#include "hidjsdevice.h"

class HIDLinuxJoystick: public HIDJsDevice
{
    Q_OBJECT
public:
    HIDLinuxJoystick(HIDPlugin* parent, quint32 line, struct hid_device_info *info);

    /** @reimp */
    void init();

    /** @reimp */
    bool openInput();

    /** @reimp */
    bool readEvent();

protected:
    bool openDevice();

private:
    /** @reimp */
    void run();
};

#endif // HIDLINUXJOYSTICK_H
