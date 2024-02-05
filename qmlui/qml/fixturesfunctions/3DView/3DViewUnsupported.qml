/*
  Q Light Controller Plus
  3DViewUnsupported.qml

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

import QtQuick 2.8
import "."

Rectangle
{
    id: viewRoot
    anchors.fill: parent
    color: UISettings.bgMedium

    function hasSettings()
    {
        return false
    }

    RobotoText
    {
        height: UISettings.bigItemHeight
        anchors.centerIn: parent
        textHAlign: Text.AlignHCenter
        label: "3D View disabled.\n" +
               "A graphics card with support for OpenGL 3.3 or higher is required.\n" +
               "(Your card supports OpenGL " + GraphicsInfo.majorVersion + "." + GraphicsInfo.minorVersion + ")"
    }
}
