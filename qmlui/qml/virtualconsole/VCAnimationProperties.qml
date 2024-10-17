/*
  Q Light Controller Plus
  VCAnimationProperties.qml

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
import QtQuick.Controls 2.1

import org.qlcplus.classes 1.0
import "."

Rectangle
{
    id: propsRoot
    color: "transparent"
    height: animationPropsColumn.height

    property VCAnimation widgetRef: null

    property int gridItemsHeight: UISettings.listItemHeight
    property QLCFunction func
    property int funcID: widgetRef ? widgetRef.functionID : -1

    onFuncIDChanged: func = functionManager.getFunction(funcID)

    Column
    {
        id: animationPropsColumn
        width: parent.width
        spacing: 5

        SectionBox
        {
            id: btnFuncProps
            sectionLabel: qsTr("Attached Function")

            sectionContents:
              GridLayout
              {
                width: parent.width
                columns: 2
                columnSpacing: 5
                rowSpacing: 4

                // row 1
                IconTextEntry
                {
                    id: funcBox
                    Layout.columnSpan: 2
                    Layout.fillWidth: true

                    tFontSize: UISettings.textSizeDefault

                    tLabel: func ? func.name : ""
                    functionType: func ? func.type : -1

                    IconButton
                    {
                        anchors.top: parent.top
                        anchors.right: parent.right
                        faSource: FontAwesome.fa_remove
                        faColor: UISettings.bgControl
                        tooltip: qsTr("Detach the current function")
                        onClicked: widgetRef.functionID = -1
                    }
                }

                CustomCheckBox
                {
                    implicitWidth: UISettings.iconSizeMedium
                    implicitHeight: implicitWidth
                    checked: widgetRef ? widgetRef.instantChanges : false
                    onClicked:
                    {
                        if (!widgetRef)
                            return

                        widgetRef.instantChanges = checked
                    }
                }

                RobotoText
                {
                    height: UISettings.listItemHeight
                    Layout.fillWidth: true
                    label: qsTr("Apply color and preset changes immediately")
                }
              } // GridLayout
        } // SectionBox

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
                    checked: widgetRef ? widgetRef.visibilityMask & VCAnimation.Fader : false
                    onClicked:
                    {
                        if (!widgetRef)
                            return

                        if (checked)
                            widgetRef.visibilityMask |= VCAnimation.Fader
                        else
                            widgetRef.visibilityMask &= ~VCAnimation.Fader
                    }
                }

                RobotoText
                {
                    height: UISettings.listItemHeight
                    Layout.fillWidth: true
                    label: qsTr("Level Fader")
                }

                CustomCheckBox
                {
                    implicitWidth: UISettings.iconSizeMedium
                    implicitHeight: implicitWidth
                    checked: widgetRef ? widgetRef.visibilityMask & VCAnimation.Label : false
                    onClicked:
                    {
                        if (!widgetRef)
                            return

                        if (checked)
                            widgetRef.visibilityMask |= VCAnimation.Label
                        else
                            widgetRef.visibilityMask &= ~VCAnimation.Label
                    }
                }

                RobotoText
                {
                    height: UISettings.listItemHeight
                    Layout.fillWidth: true
                    label: qsTr("Label")
                }

                // row 2
                CustomCheckBox
                {
                    implicitWidth: UISettings.iconSizeMedium
                    implicitHeight: implicitWidth
                    checked: widgetRef ? widgetRef.visibilityMask & VCAnimation.Color1 : false
                    onClicked:
                    {
                        if (!widgetRef)
                            return

                        if (checked)
                            widgetRef.visibilityMask |= VCAnimation.Color1
                        else
                            widgetRef.visibilityMask &= ~VCAnimation.Color1
                    }
                }

                RobotoText
                {
                    height: UISettings.listItemHeight
                    Layout.fillWidth: true
                    label: qsTr("Color 1 Button")
                }

                CustomCheckBox
                {
                    implicitWidth: UISettings.iconSizeMedium
                    implicitHeight: implicitWidth
                    checked: widgetRef ? widgetRef.visibilityMask & VCAnimation.Color2 : false
                    onClicked:
                    {
                        if (!widgetRef)
                            return

                        if (checked)
                            widgetRef.visibilityMask |= VCAnimation.Color2
                        else
                            widgetRef.visibilityMask &= ~VCAnimation.Color2
                    }
                }

                RobotoText
                {
                    height: UISettings.listItemHeight
                    Layout.fillWidth: true
                    label: qsTr("Color 2 Button")
                }


                CustomCheckBox
                {
                    implicitWidth: UISettings.iconSizeMedium
                    implicitHeight: implicitWidth
                    checked: widgetRef ? widgetRef.visibilityMask & VCAnimation.Color3 : false
                    onClicked:
                    {
                        if (!widgetRef)
                            return

                        if (checked)
                            widgetRef.visibilityMask |= VCAnimation.Color3
                        else
                            widgetRef.visibilityMask &= ~VCAnimation.Color3
                    }
                }

                RobotoText
                {
                    height: UISettings.listItemHeight
                    Layout.fillWidth: true
                    label: qsTr("Color 3 Button")
                }


                CustomCheckBox
                {
                    implicitWidth: UISettings.iconSizeMedium
                    implicitHeight: implicitWidth
                    checked: widgetRef ? widgetRef.visibilityMask & VCAnimation.Color4 : false
                    onClicked:
                    {
                        if (!widgetRef)
                            return

                        if (checked)
                            widgetRef.visibilityMask |= VCAnimation.Color4
                        else
                            widgetRef.visibilityMask &= ~VCAnimation.Color4
                    }
                }

                RobotoText
                {
                    height: UISettings.listItemHeight
                    Layout.fillWidth: true
                    label: qsTr("Color 4 Button")
                }


                CustomCheckBox
                {
                    implicitWidth: UISettings.iconSizeMedium
                    implicitHeight: implicitWidth
                    checked: widgetRef ? widgetRef.visibilityMask & VCAnimation.Color5 : false
                    onClicked:
                    {
                        if (!widgetRef)
                            return

                        if (checked)
                            widgetRef.visibilityMask |= VCAnimation.Color5
                        else
                            widgetRef.visibilityMask &= ~VCAnimation.Color5
                    }
                }

                RobotoText
                {
                    height: UISettings.listItemHeight
                    Layout.fillWidth: true
                    label: qsTr("Color 5 Button")
                }

                // row 3
                CustomCheckBox
                {
                    implicitWidth: UISettings.iconSizeMedium
                    implicitHeight: implicitWidth
                    checked: widgetRef ? widgetRef.visibilityMask & VCAnimation.PresetCombo : false
                    onClicked:
                    {
                        if (!widgetRef)
                            return

                        if (checked)
                            widgetRef.visibilityMask |= VCAnimation.PresetCombo
                        else
                            widgetRef.visibilityMask &= ~VCAnimation.PresetCombo
                    }
                }

                RobotoText
                {
                    height: UISettings.listItemHeight
                    Layout.fillWidth: true
                    label: qsTr("Preset List")
                }
              } // GridLayout
        } // SectionBox
    } // Column
}
