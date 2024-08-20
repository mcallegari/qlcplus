/*
  Q Light Controller Plus
  VCSpeedDialProperties.qml

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

import QtQuick 2.14
import QtQuick.Layouts 1.14
import QtQuick.Controls 2.14

import org.qlcplus.classes 1.0
import "."

Rectangle
{
    id: propsRoot
    color: "transparent"
    height: speedDialPropsColumn.height

    property VCSpeedDial widgetRef: null

    property int gridItemsHeight: UISettings.listItemHeight

    Column
    {
        id: speedDialPropsColumn
        width: parent.width
        spacing: 5

        SectionBox
        {
            sectionLabel: qsTr("Functions")

            sectionContents:
                Column
                {
                    width: parent.width

                    ListView
                    {
                        id: functionList
                        width: parent.width
                        height: count ? (count + 1) * UISettings.listItemHeight : UISettings.bigItemHeight
                        boundsBehavior: Flickable.StopAtBounds
                        headerPositioning: ListView.OverlayHeader

                        property real funcColWidth: (propsRoot.width * 0.4) - removeColWidth
                        property real fInColWidth: propsRoot.width * 0.2
                        property real fOutColWidth: propsRoot.width * 0.2
                        property real durColWidth: propsRoot.width * 0.2
                        property real removeColWidth: UISettings.listItemHeight

                        ListModel
                        {
                            id: speedModel
                            ListElement { mLabel: qsTr("(Not sent)"); mValue: VCSpeedDial.None }
                            ListElement { mLabel: qsTr("0"); mValue: VCSpeedDial.Zero }
                            ListElement { mLabel: qsTr("1/16"); mValue: VCSpeedDial.OneSixteenth }
                            ListElement { mLabel: qsTr("1/8"); mValue: VCSpeedDial.OneEighth }
                            ListElement { mLabel: qsTr("1/4"); mValue: VCSpeedDial.OneFourth }
                            ListElement { mLabel: qsTr("1/2"); mValue: VCSpeedDial.Half }
                            ListElement { mLabel: qsTr("1"); mValue: VCSpeedDial.One }
                            ListElement { mLabel: qsTr("2"); mValue: VCSpeedDial.Two }
                            ListElement { mLabel: qsTr("4"); mValue: VCSpeedDial.Four }
                            ListElement { mLabel: qsTr("8"); mValue: VCSpeedDial.Eight }
                            ListElement { mLabel: qsTr("16"); mValue: VCSpeedDial.Sixteen }
                        }

                        model: widgetRef ? widgetRef.functionsList : null

                        header:
                            Row
                            {
                                z: 2
                                Layout.fillWidth: true
                                height: UISettings.listItemHeight

                                RobotoText
                                {
                                    width: functionList.funcColWidth
                                    height: UISettings.listItemHeight
                                    label: qsTr("Function")
                                    color: UISettings.sectionHeader
                                }
                                Rectangle { width: 1; height: UISettings.listItemHeight; color: UISettings.fgMedium }

                                RobotoText
                                {
                                    width: functionList.fInColWidth
                                    height: UISettings.listItemHeight
                                    label: qsTr("Fade In Factor")
                                    color: UISettings.sectionHeader
                                }
                                Rectangle { width: 1; height: UISettings.listItemHeight; color: UISettings.fgMedium }

                                RobotoText
                                {
                                    width: functionList.fOutColWidth
                                    height: UISettings.listItemHeight
                                    label: qsTr("Fade Out Factor")
                                    color: UISettings.sectionHeader
                                }
                                Rectangle { width: 1; height: UISettings.listItemHeight; color: UISettings.fgMedium }

                                RobotoText
                                {
                                    width: functionList.durColWidth
                                    height: UISettings.listItemHeight
                                    label: qsTr("Duration Factor (+tap)")
                                    color: UISettings.sectionHeader
                                }
                                Rectangle { width: 1; height: UISettings.listItemHeight; color: UISettings.fgMedium }

                                Rectangle
                                {
                                    width: functionList.removeColWidth
                                    height: UISettings.listItemHeight
                                    color: UISettings.sectionHeader
                                }
                            }

                        delegate:
                            Row
                            {
                                width: functionList.width
                                height: UISettings.listItemHeight

                                property int functionID: modelData.funcID
                                property QLCFunction func

                                onFunctionIDChanged:
                                {
                                    func = functionManager.getFunction(functionID)
                                    funcEntry.tLabel = func.name
                                    funcEntry.functionType = func.type
                                }

                                IconTextEntry
                                {
                                    id: funcEntry
                                    width: functionList.funcColWidth
                                    height: parent.height
                                }

                                CustomComboBox
                                {
                                    model: speedModel
                                    implicitWidth: functionList.fInColWidth
                                    currentIndex: modelData.fadeIn
                                    onActivated: widgetRef.setFunctionSpeed(functionID, QLCFunction.FadeIn, currentIndex)
                                }
                                CustomComboBox
                                {
                                    model: speedModel
                                    implicitWidth: functionList.fOutColWidth
                                    currentIndex: modelData.fadeOut
                                    onActivated: widgetRef.setFunctionSpeed(functionID, QLCFunction.FadeOut, currentIndex)
                                }
                                CustomComboBox
                                {
                                    model: speedModel
                                    implicitWidth: functionList.durColWidth
                                    currentIndex: modelData.duration
                                    onActivated: widgetRef.setFunctionSpeed(functionID, QLCFunction.Duration, currentIndex)

                                }
                                IconButton
                                {
                                    width: height
                                    height: UISettings.listItemHeight
                                    imgSource: "qrc:/remove.svg"
                                    tooltip: qsTr("Remove this function")
                                    onClicked:
                                    {
                                        if (widgetRef)
                                            widgetRef.removeFunction(functionID)
                                    }
                                }
                            }
                    } // ListView

                    Rectangle
                    {
                        width: parent.width
                        height: UISettings.bigItemHeight * 0.6
                        color: UISettings.bgMedium
                        radius: 10
                        visible: functionList.count == 0

                        RobotoText
                        {
                            anchors.centerIn: parent
                            textHAlign: Text.AlignHCenter
                            label: qsTr("Drag & Drop Functions over\nthe widget to populate this list")
                        }
                    }
                } // Column
        } // SectionBox

        SectionBox
        {
            sectionLabel: qsTr("Dial Properties")

            sectionContents:
              GridLayout
              {
                width: parent.width
                columns: 5

                Row
                {
                    Layout.columnSpan: 5

                    CustomCheckBox
                    {
                        implicitWidth: UISettings.iconSizeMedium
                        implicitHeight: implicitWidth
                        checked: widgetRef ? widgetRef.resetOnDialChange : false
                        onClicked: if (widgetRef) widgetRef.resetOnDialChange = checked
                    }

                    RobotoText
                    {
                        height: UISettings.listItemHeight
                        label: qsTr("Reset multiplier factor when the dial value changes")
                    }
                }

                RobotoText
                {
                    height: UISettings.listItemHeight
                    label: qsTr("Dial time range")
                }
                CustomSpinBox
                {
                    from: 0
                    to: 100000
                    suffix: timeSwitchButton.checked ? "ms" : "s"
                    value: widgetRef ? (timeSwitchButton.checked ? widgetRef.timeMinimumValue : Math.floor(widgetRef.timeMinimumValue / 1000)) : 0
                    onValueModified:
                    {
                        if (timeSwitchButton.checked)
                            widgetRef.timeMinimumValue = value
                        else
                            widgetRef.timeMinimumValue = value * 1000
                    }
                }

                RobotoText
                {
                    height: UISettings.listItemHeight
                    label: qsTr("to")
                }

                CustomSpinBox
                {
                    from: 0
                    to: 100000
                    suffix: timeSwitchButton.checked ? "ms" : "s"
                    value: widgetRef ? (timeSwitchButton.checked ? widgetRef.timeMaximumValue : Math.floor(widgetRef.timeMaximumValue / 1000)) : 0
                    onValueModified:
                    {
                        if (timeSwitchButton.checked)
                            widgetRef.timeMaximumValue = value
                        else
                            widgetRef.timeMaximumValue = value * 1000
                    }
                }

                Rectangle
                {
                    id: timeSwitchButton
                    width: UISettings.iconSizeDefault * 1.1
                    height: UISettings.listItemHeight
                    border.width: 2
                    border.color: "white"
                    radius: 5
                    color: UISettings.sectionHeader

                    property bool checked: false

                    RobotoText
                    {
                        height: parent.height
                        anchors.horizontalCenter: parent.horizontalCenter
                        label: timeSwitchButton.checked ? "ms" : "S"
                        fontSize: UISettings.textSizeDefault
                        fontBold: true
                    }

                    MouseArea
                    {
                        anchors.fill: parent
                        onClicked: timeSwitchButton.checked = !timeSwitchButton.checked
                    }
                }
              }
        }

        SectionBox
        {
            sectionLabel: qsTr("Appearance")

            sectionContents:
              GridLayout
              {
                width: parent.width
                columns: 4
                columnSpacing: 5
                rowSpacing: 4

                // row 1
                CustomCheckBox
                {
                    implicitWidth: UISettings.iconSizeMedium
                    implicitHeight: implicitWidth
                    checked: widgetRef ? widgetRef.visibilityMask & VCSpeedDial.PlusMinus : false
                    onClicked:
                    {
                        if (!widgetRef)
                            return

                        if (checked)
                            widgetRef.visibilityMask |= VCSpeedDial.PlusMinus
                        else
                            widgetRef.visibilityMask &= ~VCSpeedDial.PlusMinus
                    }
                }

                RobotoText
                {
                    height: UISettings.listItemHeight
                    Layout.fillWidth: true
                    label: qsTr("+/- Buttons")
                }

                CustomCheckBox
                {
                    implicitWidth: UISettings.iconSizeMedium
                    implicitHeight: implicitWidth
                    checked: widgetRef ? widgetRef.visibilityMask & VCSpeedDial.Dial : false
                    onClicked:
                    {
                        if (!widgetRef)
                            return

                        if (checked)
                            widgetRef.visibilityMask |= VCSpeedDial.Dial
                        else
                            widgetRef.visibilityMask &= ~VCSpeedDial.Dial
                    }
                }

                RobotoText
                {
                    height: UISettings.listItemHeight
                    Layout.fillWidth: true
                    label: qsTr("Dial")
                }

                // row 2
                CustomCheckBox
                {
                    implicitWidth: UISettings.iconSizeMedium
                    implicitHeight: implicitWidth
                    checked: widgetRef ? widgetRef.visibilityMask & VCSpeedDial.Tap : false
                    onClicked:
                    {
                        if (!widgetRef)
                            return

                        if (checked)
                            widgetRef.visibilityMask |= VCSpeedDial.Tap
                        else
                            widgetRef.visibilityMask &= ~VCSpeedDial.Tap
                    }
                }

                RobotoText
                {
                    height: UISettings.listItemHeight
                    Layout.fillWidth: true
                    label: qsTr("Tap")
                }

                CustomCheckBox
                {
                    implicitWidth: UISettings.iconSizeMedium
                    implicitHeight: implicitWidth
                    checked: widgetRef ? widgetRef.visibilityMask & VCSpeedDial.Multipliers : false
                    onClicked:
                    {
                        if (!widgetRef)
                            return

                        if (checked)
                            widgetRef.visibilityMask |= VCSpeedDial.Multipliers
                        else
                            widgetRef.visibilityMask &= ~VCSpeedDial.Multipliers
                    }
                }

                RobotoText
                {
                    height: UISettings.listItemHeight
                    Layout.fillWidth: true
                    label: qsTr("Multipliers")
                }

                // row 3
                CustomCheckBox
                {
                    implicitWidth: UISettings.iconSizeMedium
                    implicitHeight: implicitWidth
                    checked: widgetRef ? widgetRef.visibilityMask & VCSpeedDial.Apply : false
                    onClicked:
                    {
                        if (!widgetRef)
                            return

                        if (checked)
                            widgetRef.visibilityMask |= VCSpeedDial.Apply
                        else
                            widgetRef.visibilityMask &= ~VCSpeedDial.Apply
                    }
                }

                RobotoText
                {
                    height: UISettings.listItemHeight
                    Layout.fillWidth: true
                    label: qsTr("Apply")
                }

                CustomCheckBox
                {
                    implicitWidth: UISettings.iconSizeMedium
                    implicitHeight: implicitWidth
                    checked: widgetRef ? widgetRef.visibilityMask & VCSpeedDial.Beats : false
                    onClicked:
                    {
                        if (!widgetRef)
                            return

                        if (checked)
                            widgetRef.visibilityMask |= VCSpeedDial.Beats
                        else
                            widgetRef.visibilityMask &= ~VCSpeedDial.Beats
                    }
                }

                RobotoText
                {
                    height: UISettings.listItemHeight
                    Layout.fillWidth: true
                    label: qsTr("Beats")
                }

                // row 4
                CustomCheckBox
                {
                    implicitWidth: UISettings.iconSizeMedium
                    implicitHeight: implicitWidth
                    checked: widgetRef ? widgetRef.visibilityMask & VCSpeedDial.Hours : false
                    onClicked:
                    {
                        if (!widgetRef)
                            return

                        if (checked)
                            widgetRef.visibilityMask |= VCSpeedDial.Hours
                        else
                            widgetRef.visibilityMask &= ~VCSpeedDial.Hours
                    }
                }

                RobotoText
                {
                    height: UISettings.listItemHeight
                    Layout.fillWidth: true
                    label: qsTr("Hours")
                }

                CustomCheckBox
                {
                    implicitWidth: UISettings.iconSizeMedium
                    implicitHeight: implicitWidth
                    checked: widgetRef ? widgetRef.visibilityMask & VCSpeedDial.Minutes : false
                    onClicked:
                    {
                        if (!widgetRef)
                            return

                        if (checked)
                            widgetRef.visibilityMask |= VCSpeedDial.Minutes
                        else
                            widgetRef.visibilityMask &= ~VCSpeedDial.Minutes
                    }
                }

                RobotoText
                {
                    height: UISettings.listItemHeight
                    Layout.fillWidth: true
                    label: qsTr("Minutes")
                }

                // row 5
                CustomCheckBox
                {
                    implicitWidth: UISettings.iconSizeMedium
                    implicitHeight: implicitWidth
                    checked: widgetRef ? widgetRef.visibilityMask & VCSpeedDial.Seconds : false
                    onClicked:
                    {
                        if (!widgetRef)
                            return

                        if (checked)
                            widgetRef.visibilityMask |= VCSpeedDial.Seconds
                        else
                            widgetRef.visibilityMask &= ~VCSpeedDial.Seconds
                    }
                }

                RobotoText
                {
                    height: UISettings.listItemHeight
                    Layout.fillWidth: true
                    label: qsTr("Seconds")
                }

                CustomCheckBox
                {
                    implicitWidth: UISettings.iconSizeMedium
                    implicitHeight: implicitWidth
                    checked: widgetRef ? widgetRef.visibilityMask & VCSpeedDial.Milliseconds : false
                    onClicked:
                    {
                        if (!widgetRef)
                            return

                        if (checked)
                            widgetRef.visibilityMask |= VCSpeedDial.Milliseconds
                        else
                            widgetRef.visibilityMask &= ~VCSpeedDial.Milliseconds
                    }
                }

                RobotoText
                {
                    height: UISettings.listItemHeight
                    Layout.fillWidth: true
                    label: qsTr("Milliseconds")
                }
              } // GridLayout
        } // SectionBox
    } // Column
}
