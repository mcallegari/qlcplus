/*
  Q Light Controller Plus
  WizardStepIndicator.qml

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
import "."

Item
{
    id: root

    property int currentStep: 0
    property int totalSteps: 6
    property var stepTitles: []

    height: UISettings.iconSizeMedium * 0.8
    width: parent.width

    // Connecting line behind the dots
    Rectangle
    {
        id: line
        anchors.verticalCenter: dots.verticalCenter
        anchors.left: dots.left
        anchors.right: dots.right
        anchors.leftMargin: dotRepeater.count > 0 ? (dots.width / (dotRepeater.count * 2)) : 0
        anchors.rightMargin: anchors.leftMargin
        height: 2
        color: "#333355"

        // Filled progress portion
        Rectangle
        {
            width: root.totalSteps > 1
                   ? (parent.width * root.currentStep / (root.totalSteps - 1))
                   : 0
            height: parent.height
            color: "#0978FF"
            Behavior on width { NumberAnimation { duration: 280; easing.type: Easing.InOutQuad } }
        }
    }

    Row
    {
        id: dots
        anchors.verticalCenter: parent.verticalCenter
        spacing: 0
        width: parent.width * 0.85
        height: UISettings.iconSizeMedium * 0.8

        Repeater
        {
            id: dotRepeater
            model: root.totalSteps

            Item
            {
                width: dots.width / root.totalSteps
                height: root.height

                property bool isDone:   index < root.currentStep
                property bool isCurrent: index === root.currentStep

                // Dot circle
                Rectangle
                {
                    id: dot
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.top: parent.top
                    anchors.topMargin: 4
                    width: isCurrent ? UISettings.iconSizeMedium * 0.8
                                     : UISettings.iconSizeMedium * 0.65
                    height: width
                    radius: width / 2
                    color: isCurrent ? "#0978FF"
                                     : (isDone ? '#5c9f83' : "#333355")

                    Behavior on width { NumberAnimation { duration: 300 } }
                    Behavior on color { ColorAnimation  { duration: 300 } }

                    // Checkmark for done steps
                    RobotoText
                    {
                        anchors.centerIn: parent
                        visible: isDone
                        height: UISettings.textSizeDefault * 0.8
                        label: "✓"
                        labelColor: "white"
                        fontSize: height
                    }

                    // Step number for future steps
                    RobotoText
                    {
                        anchors.centerIn: parent
                        visible: !isDone && !isCurrent
                        height: UISettings.textSizeDefault * 0.8
                        label: (index + 1).toString()
                        labelColor: "#777799"
                        fontSize: height
                    }
                }

                // Label below dot
                RobotoText
                {
                    anchors.top: dot.bottom
                    //anchors.topMargin: 4
                    anchors.horizontalCenter: parent.horizontalCenter
                    width: parent.width - 4
                    label: root.stepTitles[index] || ""
                    labelColor: isCurrent ? "#0978FF"
                                         : (isDone ? "#9999BB" : "#555577")
                    fontSize: UISettings.textSizeDefault * 0.8
                    textHAlign: Text.AlignHCenter
                    wrapText: true

                    Behavior on labelColor { ColorAnimation { duration: 200 } }
                }
            }
        }
    }
}
