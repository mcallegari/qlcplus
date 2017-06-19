/*
  Q Light Controller Plus
  CustomPopupDialog.qml

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

import QtQuick 2.6
import QtQuick.Layouts 1.0
import QtQuick.Controls 2.1

import "."

Dialog
{
    id: control
    x: (mainView.width - width) / 2
    y: (mainView.height - height) / 2
    parent: mainView

    modal: true
    title: ""
    standardButtons: Dialog.Ok | Dialog.Cancel
    onVisibleChanged: mainView.setDimScreen(visible)

    property string message: ""

    function setButtonStatus(index, status)
    {
        buttonBoxControl.itemAt(index).enabled = status
    }

    header:
        Label
        {
            text: control.title
            color: "white"
            visible: control.title
            elide: Label.ElideRight
            font.family: UISettings.robotoFontName
            font.pixelSize: UISettings.textSizeDefault
            font.bold: true
            padding: 12
            background:
                Rectangle
                {
                    color: UISettings.bgLight
                    x: 2
                    y: 2
                    width: parent.width - 4
                    height: parent.height - 2
                }
        }

    background:
        Rectangle
        {
            color: UISettings.bgControl
            border.color: UISettings.bgLight
            border.width: 2
        }

    contentItem:
        Text
        {
            visible: message
            font.family: UISettings.robotoFontName
            font.pixelSize: UISettings.textSizeDefault
            color: UISettings.fgMain
            text: message
        }

    footer:
        DialogButtonBox
        {
            id: buttonBoxControl
            visible: count > 0

            contentItem.implicitHeight: UISettings.iconSizeDefault

            background:
                Rectangle
                {
                    implicitHeight: UISettings.iconSizeDefault
                    color: UISettings.bgControl
                    x: 2
                    y: 2
                    width: parent.width - 4
                }

            delegate:
                Button
                {
                    id: buttonControl
                    width: buttonBoxControl.count === 1 ? buttonBoxControl.availableWidth / 2 : undefined

                    hoverEnabled: true

                    contentItem:
                        Text
                        {
                            text: buttonControl.text
                            font.family: UISettings.robotoFontName
                            font.pixelSize: UISettings.textSizeDefault
                            opacity: enabled ? 1.0 : 0.3
                            color: UISettings.fgMain
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                            elide: Text.ElideRight
                        }

                    background:
                        Rectangle
                        {
                            color: buttonControl.hovered ?
                                       buttonControl.down ? UISettings.highlightPressed : UISettings.highlight : UISettings.bgLight
                            opacity: enabled ? 1 : 0.3
                            border.color: UISettings.bgStrong
                            border.width: 2
                        }
                }
        }
}
