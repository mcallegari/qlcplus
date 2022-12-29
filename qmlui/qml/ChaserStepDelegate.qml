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
import org.qlcplus.classes 1.0
import "."

Rectangle
{
    id: stepDelegate
    width: 500
    height: UISettings.listItemHeight

    color: isPrinting ? "white" : "transparent"

    property int functionID: -1
    property QLCFunction func
    property bool showFunctionName: true
    property string stepFadeIn
    property string stepHold
    property string stepFadeOut
    property string stepDuration
    property string stepNote

    property bool isPrinting: false
    property color labelColor: isPrinting ? "black" : UISettings.fgMain
    property int labelFontSize: UISettings.textSizeDefault * 0.75

    property bool isSelected: false
    property int indexInList: -1
    property int highlightIndex: -1
    property int highlightEditTime: -1
    property int nextIndex: -1

    property int col1Width: 25
    property int col2Width: UISettings.bigItemHeight * 1.5
    property int col3Width: UISettings.bigItemHeight
    property int col4Width: UISettings.bigItemHeight
    property int col5Width: UISettings.bigItemHeight
    property int col6Width: UISettings.bigItemHeight

    signal doubleClicked(int ID, var qItem, int type)

    onFunctionIDChanged:
    {
        func = functionManager.getFunction(functionID)
        funcIconName.functionType = func.type
    }

    onHighlightEditTimeChanged:
    {
        var item = null

        switch(highlightEditTime)
        {
            case QLCFunction.FadeIn: item = fadeInText; break;
            case QLCFunction.Hold: item = holdText; break;
            case QLCFunction.FadeOut: item = fadeOutText; break;
            case QLCFunction.Duration: item = durationText; break;
        }

        if (item)
        {
            editBox.visible = true
            editBox.x = item.x
            editBox.width = item.width
        }
        else
            editBox.visible = false
    }

    function handleDoubleClick(x, y)
    {
        var item = fieldsRow.childAt(x, y)
        if (item === funcIconName)
            stepDelegate.doubleClicked(functionID, item, QLCFunction.Name)
        else if (item === fadeInText)
            stepDelegate.doubleClicked(functionID, item, QLCFunction.FadeIn)
        else if (item === holdText)
            stepDelegate.doubleClicked(functionID, item, QLCFunction.Hold)
        else if (item === fadeOutText)
            stepDelegate.doubleClicked(functionID, item, QLCFunction.FadeOut)
        else if (item === durationText)
            stepDelegate.doubleClicked(functionID, item, QLCFunction.Duration)
        else if (item === noteText)
            stepDelegate.doubleClicked(functionID, item, QLCFunction.Notes)
    }

    // Selection rectangle
    Rectangle
    {
        anchors.fill: parent
        radius: 3
        color: UISettings.highlight
        visible: isSelected
    }

    // Highlight the time being edited
    Rectangle
    {
        id: editBox
        visible: false
        height: parent.height
        color: "transparent"
        border.width: 2
        border.color: "yellow"
    }

    Timer
    {
        id: clickTimer
        interval: 200
        repeat: false
        running: false

        property int modifiers: 0

        onTriggered:
        {
            stepDelegate.clicked(functionID, stepDelegate, modifiers)
            modifiers = 0
        }
    }

    // top line drag highlight
    Rectangle
    {
        visible: highlightIndex == indexInList
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
            color: nextIndex === indexInList ? "orange" : "transparent"
            label: indexInList + 1
            labelColor: stepDelegate.labelColor
            fontSize: labelFontSize
            wrapText: true
            textHAlign: Text.AlignHCenter
        }
        Rectangle { height: parent.height; width: 1; color: UISettings.fgMedium }

        IconTextEntry
        {
            id: funcIconName
            visible: showFunctionName
            width: col2Width
            height: 35
            anchors.verticalCenter: parent.verticalCenter
            tLabel: func ? func.name : ""
            tLabelColor: stepDelegate.labelColor
        }
        Rectangle { visible: showFunctionName; height: parent.height; width: 1; color: UISettings.fgMedium }

        RobotoText
        {
            id: fadeInText
            width: col3Width
            height: parent.height
            label: stepFadeIn
            labelColor: stepDelegate.labelColor
            fontSize: labelFontSize
            wrapText: true
            textHAlign: Text.AlignHCenter
        }
        Rectangle { height: parent.height; width: 1; color: UISettings.fgMedium }

        RobotoText
        {
            id: holdText
            width: col4Width
            height: parent.height
            label: stepHold
            labelColor: stepDelegate.labelColor
            fontSize: labelFontSize
            wrapText: true
            textHAlign: Text.AlignHCenter
        }
        Rectangle { height: parent.height; width: 1; color: UISettings.fgMedium }

        RobotoText
        {
            id: fadeOutText
            width: col5Width
            height: parent.height
            label: stepFadeOut
            labelColor: stepDelegate.labelColor
            fontSize: labelFontSize
            wrapText: true
            textHAlign: Text.AlignHCenter
        }
        Rectangle { height: parent.height; width: 1; color: UISettings.fgMedium }

        RobotoText
        {
            id: durationText
            width: col6Width
            height: parent.height
            label: stepDuration
            labelColor: stepDelegate.labelColor
            fontSize: labelFontSize
            wrapText: true
            textHAlign: Text.AlignHCenter
        }
        Rectangle { height: parent.height; width: 1; color: UISettings.fgMedium }

        RobotoText
        {
            id: noteText
            width: stepDelegate.width - x
            height: parent.height
            label: stepNote
            labelColor: stepDelegate.labelColor
            fontSize: labelFontSize
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
