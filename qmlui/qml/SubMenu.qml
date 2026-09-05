/*
  Q Light Controller Plus
  SubMenu.qml

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

import QtQuick
import QtQuick.Controls

import "."

/** A cascading sub-menu, to be placed inside a ContextMenuEntry.
 *
 *  This is a Popup rather than a plain Item on purpose: a sub-menu sticks out
 *  of the bounds of both the entry it belongs to and of the menu Popup itself,
 *  and Qt skips the delivery of pointer events to such items, so entries would
 *  neither highlight on hover nor react to clicks.
 *
 *  Sub-menus are shown/hidden through the 'visible' property by the menu
 *  handling them, hence the disabled automatic close policy.
 */
Popup
{
    padding: 0
    closePolicy: Popup.NoAutoClose

    background:
        Rectangle
        {
            border.width: 1
            border.color: UISettings.bgStronger
            color: UISettings.bgStrong
        }
}
