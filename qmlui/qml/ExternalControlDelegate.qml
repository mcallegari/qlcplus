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
    width: parent.width

    property var dObjRef: null
    property bool invalid: false
    property int controlID
    property alias controlIndex: controlsCombo.currentIndex
    property var universe
    property var channel
    property string uniName
    property string chName
    property bool customFeedback: false
    property int lowerFb: 0
    property int upperFb: 255

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
            Layout.columnSpan: 2
            height: UISettings.listItemHeight
            model: dObjRef ? dObjRef.externalControlsList : null
            currentValue: controlID
            onValueChanged:
            {
                controlID = value
                dObjRef.updateInputSourceControlID(universe, channel, controlID)
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
            checkable: true
            checked: invalid
            imgSource: "qrc:/inputoutput.svg"
            tooltip: qsTr("Activate auto detection")

            onToggled:
            {
                if (checked == true)
                {
                    if (invalid === false &&
                        virtualConsole.enableInputSourceAutoDetection(dObjRef, controlID, universe, channel) === true)
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
            width: UISettings.iconSizeMedium
            height: width
            imgSource: "qrc:/remove.svg"
            tooltip: qsTr("Remove this input source")

            onClicked: virtualConsole.deleteInputSource(dObjRef, controlID, universe, channel)
        }

        RobotoText
        {
            Layout.columnSpan: 3
            Layout.fillWidth: true
            height: UISettings.listItemHeight
            visible: customFeedback
            label: qsTr("Custom feedbacks")
            color: UISettings.bgMedium
        }

        Row
        {
            id: cfRow
            Layout.columnSpan: 3
            Layout.fillWidth: true
            spacing: 5
            visible: customFeedback

            RobotoText { id: cfLower; height: UISettings.listItemHeight; label: qsTr("Lower") }
            CustomSpinBox
            {
                id: lowerSpin
                width: (cfRow.width - cfLower.width - cfUpper.width - 20) / 2
                from: 0
                to: 255
                value: lowerFb
                onValueChanged: if (dObjRef) dObjRef.updateInputSourceRange(universe, channel, value, upperSpin.value)
            }
            RobotoText { id: cfUpper; height: UISettings.listItemHeight; label: qsTr("Upper") }
            CustomSpinBox
            {
                id: upperSpin
                width: (cfRow.width - cfLower.width - cfUpper.width - 20) / 2
                from: 0
                to: 255
                value: upperFb
                onValueChanged: if (dObjRef) dObjRef.updateInputSourceRange(universe, channel, lowerSpin.value, value)
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
