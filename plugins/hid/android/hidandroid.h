/*
  Q Light Controller Plus
  hidandroid.h

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

#ifndef HIDANDROID_H
#define HIDANDROID_H

class QObject;

/** Set the object whose slotDeviceAdded(uint,uint) / slotDeviceRemoved(uint,uint)
 *  are invoked on USB attach/detach and when a USB permission is granted */
void hid_android_set_listener(QObject *listener);

#endif
