/*
  Q Light Controller Plus
  IconTextEntry.qml

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

import QtQuick 2.0
import QtQuick.Layouts 1.1

import com.qlcplus.classes 1.0

Rectangle
{
    width: 100
    height: 40
    color: "transparent"

    property string iSrc
    property string tLabel
    property int fontSize: 11
    property int functionType: -1

    onFunctionTypeChanged:
    {
        if (functionType == -1)
        {
            iSrc = ""
            return
        }

        switch (functionType)
        {
            case Function.Scene: iSrc = "qrc:/scene.svg"; break;
            case Function.Chaser: iSrc = "qrc:/chaser.svg"; break;
            case Function.EFX: iSrc = "qrc:/efx.svg"; break;
            case Function.Collection: iSrc = "qrc:/collection.svg"; break;
            case Function.Script: iSrc = "qrc:/script.svg"; break;
            case Function.RGBMatrix: iSrc = "qrc:/rgbmatrix.svg"; break;
            case Function.Show: iSrc = "qrc:/showmanager.svg"; break;
            case Function.Audio: iSrc = "qrc:/audio.svg"; break;
            case Function.Video: iSrc = "qrc:/video.svg"; break;
        }
    }

    RowLayout
    {
        anchors.fill: parent
        spacing: 4

        Image
        {
            source: iSrc
            height: parent.height - 4
            width: height
            sourceSize: Qt.size(width, height)
        }

        RobotoText
        {
            Layout.fillWidth: true
            height: parent.height
            anchors.verticalCenter: parent.verticalCenter
            label: tLabel
            fontSize: fontSize
        }
    }
}

