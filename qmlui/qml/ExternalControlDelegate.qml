/*
  Q Light Controller Plus
  ExternalControlDelegate.qml

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

import org.qlcplus.classes 1.0
import "."

Column
{
    id: itemRoot
    width: parent.width

    property var widgetObjRef: null
    property bool invalid: false
    property int controlID
    property alias inputModel: controlsCombo.model
    property var universe
    property var channel
    property string uniName
    property string chName
    property bool customFeedback: false

    signal requestCustomFeedbackPopup()

    GridLayout
    {
        width: parent.width
        columns: 3
        columnSpacing: 3
        rowSpacing: 3

        // row 1
        RobotoText
        {
            height: UISettings.listItemHeight
            label: qsTr("Control")
        }

        CustomComboBox
        {
            id: controlsCombo
            Layout.fillWidth: true
            height: UISettings.listItemHeight
            currValue: controlID
            onValueChanged:
            {
                controlID = value
                widgetObjRef.updateInputSourceControlID(universe, channel, controlID)
            }
        }

        IconButton
        {
            width: UISettings.iconSizeMedium
            height: width
            checkable: true
            checked: invalid
            imgSource: "qrc:/wizard.svg"
            tooltip: qsTr("Activate auto detection")

            onToggled:
            {
                if (checked == true)
                {
                    if (invalid === false &&
                        virtualConsole.enableInputSourceAutoDetection(widgetObjRef, controlID, universe, channel) === true)
                        invalid = true
                    else
                        checked = false
                }
                else
                {
                    virtualConsole.disableAutoDetection()
                    invalid = false
                    uniNameBox.color = UISettings.bgLight
                    chNameBox.color = UISettings.bgLight
                }
            }
        }

        // row 2
        RobotoText
        {
            height: UISettings.listItemHeight
            label: qsTr("Universe")
        }

        RobotoText
        {
            id: uniNameBox
            Layout.fillWidth: true
            height: UISettings.listItemHeight
            color: UISettings.bgLight
            label: uniName

            SequentialAnimation on color
            {
                PropertyAnimation { to: "red"; duration: 1000 }
                PropertyAnimation { to: UISettings.bgLight; duration: 1000 }
                running: invalid
                loops: Animation.Infinite
            }
        }

        IconButton
        {
            width: UISettings.iconSizeMedium
            height: width
            imgSource: "qrc:/remove.svg"
            tooltip: qsTr("Remove this input source")

            onClicked: virtualConsole.deleteInputSource(widgetObjRef, controlID, universe, channel)
        }

        // row 3
        RobotoText
        {
            height: UISettings.listItemHeight
            label: qsTr("Channel")
        }

        RobotoText
        {
            id: chNameBox
            Layout.fillWidth: true
            height: UISettings.listItemHeight
            color: UISettings.bgLight
            label: chName

            SequentialAnimation on color
            {
                PropertyAnimation { to: "red"; duration: 1000 }
                PropertyAnimation { to: UISettings.bgLight; duration: 1000 }
                running: invalid
                loops: Animation.Infinite
            }
        }

        IconButton
        {
            visible: customFeedback
            width: UISettings.iconSizeMedium
            height: width
            imgSource: "qrc:/inputoutput.svg"
            tooltip: qsTr("Custom feedback selection")

            onClicked:
            {
                itemRoot.requestCustomFeedbackPopup()
            }
        }
    } // end of GridLayout

    // items divider
    Rectangle
    {
        width: parent.width
        height: 2
        color: UISettings.fgMedium
    }
}
