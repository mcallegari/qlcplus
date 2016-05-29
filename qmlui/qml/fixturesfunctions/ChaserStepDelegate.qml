/*
  Q Light Controller Plus
  ChaserStepDelegate.qml

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
import "."

Rectangle
{
    id: stepDelegate
    width: 100
    height: 35

    color: "transparent"

    property int functionID: -1
    property Function func
    property string stepLabel
    property string stepFadeIn
    property string stepHold
    property string stepFadeOut
    property string stepDuration
    property string stepNote

    property int labelFontSize: 11

    property bool isSelected: false
    property int indexInList: -1
    property int highlightIndex: -1

    property int col1Width: 25
    property int col2Width: 120
    property int col3Width: 60
    property int col4Width: 60
    property int col5Width: 60
    property int col6Width: 60

    signal clicked(int ID, var qItem, int mouseMods)
    signal doubleClicked(int ID, string type)

    onFunctionIDChanged:
    {
        func = functionManager.getFunction(functionID)
        stepLabel = func.name
        funcIconName.functionType = func.type
    }

    onHighlightIndexChanged:
    {
        if (indexInList >= 0 && highlightIndex == indexInList)
            topDragLine.visible = true
        else
            topDragLine.visible = false
    }

    // Highlight rectangle
    Rectangle
    {
        anchors.fill: parent
        radius: 3
        color: UISettings.highlight
        visible: isSelected
    }

    MouseArea
    {
        anchors.fill: parent
        onClicked:
        {
            stepDelegate.clicked(functionID, stepDelegate, mouse.modifiers)
        }
        onDoubleClicked:
        {
            console.log("Double click happened at " + mouse.x + "," + mouse.y)

            var item = fieldsRow.childAt(mouse.x, mouse.y)
            if (item === funcIconName)
                console.log("Func name clicked")
            else if (item === fadeInText)
                stepDelegate.doubleClicked(functionID, "FI")
            else if (item === holdText)
                stepDelegate.doubleClicked(functionID, "H")
            else if (item === fadeOutText)
                stepDelegate.doubleClicked(functionID, "FO")
            else if (item === durationText)
                stepDelegate.doubleClicked(functionID, "D")
            else if (mouse.x >= noteText.x)
                console.log("Note clicked")
        }
    }

    // top line drag highlight
    Rectangle
    {
        id: topDragLine
        visible: false
        width: parent.width
        height: 2
        z: 1
        color: UISettings.selection
    }

    Row
    {
        id: fieldsRow
        height: stepDelegate.height
        spacing: 2

        RobotoText
        {
            width: col1Width
            height: parent.height
            label: indexInList + 1
            fontSize: labelFontSize
            wrapText: true
            textAlign: Text.AlignHCenter
        }
        Rectangle { height: parent.height; width: 1; color: UISettings.fgMedium }

        IconTextEntry
        {
            id: funcIconName
            width: col2Width
            height: 35
            anchors.verticalCenter: parent.verticalCenter
            tLabel: stepLabel
        }
        Rectangle { height: parent.height; width: 1; color: UISettings.fgMedium }

        RobotoText
        {
            id: fadeInText
            width: col3Width
            height: parent.height
            label: stepFadeIn
            fontSize: labelFontSize
            wrapText: true
            textAlign: Text.AlignHCenter
        }
        Rectangle { height: parent.height; width: 1; color: UISettings.fgMedium }

        RobotoText
        {
            id: holdText
            width: col4Width
            height: parent.height
            label: stepHold
            fontSize: labelFontSize
            wrapText: true
            textAlign: Text.AlignHCenter
        }
        Rectangle { height: parent.height; width: 1; color: UISettings.fgMedium }

        RobotoText
        {
            id: fadeOutText
            width: col5Width
            height: parent.height
            label: stepFadeOut
            fontSize: labelFontSize
            wrapText: true
            textAlign: Text.AlignHCenter
        }
        Rectangle { height: parent.height; width: 1; color: UISettings.fgMedium }

        RobotoText
        {
            id: durationText
            width: col6Width
            height: parent.height
            label: stepDuration
            fontSize: labelFontSize
            wrapText: true
            textAlign: Text.AlignHCenter
        }
        Rectangle { height: parent.height; width: 1; color: UISettings.fgMedium }

        RobotoText
        {
            id: noteText
            height: parent.height
            label: stepNote
            fontSize: labelFontSize
            //Layout.fillWidth: true
        }
    }

    // steps divider
    Rectangle
    {
        width: parent.width
        height: 1
        y: parent.height - 1
        color: UISettings.fgMedium
    }
}
