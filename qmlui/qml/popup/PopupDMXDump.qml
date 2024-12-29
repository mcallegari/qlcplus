/*
  Q Light Controller Plus
  PopupDMXDump.qml

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
import QtQuick.Controls 2.2

import org.qlcplus.classes 1.0
import "."

CustomPopupDialog
{
    id: popupRoot
    width: mainView.width / 2
    title: qsTr("DMX Channel Dump")

    property int capabilityMask: 0
    property int channelSetMask: 0
    property alias sceneName: nameInputBox.text
    property alias existingScene: existingSceneCheck.checked
    property alias allChannels: allChannelsCheck.checked
    property alias nonZeroOnly: nonZeroCheck.checked
    property int universesCount: 0
    property int nextFunctionId: 0

    property QLCFunction func
    property int selectedFunctionsCount: 0

    onOpened:
    {
        universesCount = ioManager.universesCount()
        nextFunctionId = functionManager.nextFunctionId()

        // handle Function selection
        var funcList = functionManager.selectedFunctionsID()
        selectedFunctionsCount = funcList.length
        if (funcList.length > 0)
            func = functionManager.getFunction(funcList[0])

        if (capabilityMask == 0 && activeChannelsCheck.checked)
            allChannels = true
    }

    function getChannelsMask()
    {
        var mask = 0

        if (intTypeCheck.checked)
            mask |= App.DimmerType
        if (colTypeCheck.checked)
            mask |= App.ColorType
        if (colMacroTypeCheck.checked)
            mask |= App.ColorMacroType
        if (goboTypeCheck.checked)
            mask |= App.GoboType
        if (panTypeCheck.checked)
            mask |= App.PanType
        if (tiltTypeCheck.checked)
            mask |= App.TiltType
        if (speedTypeCheck.checked)
            mask |= App.SpeedType
        if (shutterTypeCheck.checked)
            mask |= App.ShutterType
        if (prismTypeCheck.checked)
            mask |= App.PrismType
        if (beamTypeCheck.checked)
            mask |= App.BeamType
        if (effectTypeCheck.checked)
            mask |= App.EffectType
        if (maintTypeCheck.checked)
            mask |= App.MaintenanceType

        return mask
    }

    function focusEditItem()
    {
        nameInputBox.selectAndFocus()
    }

    ButtonGroup { id: sceneTypeGroup }
    ButtonGroup { id: dumpTypeGroup }

    contentItem:
        GridLayout
        {
            width: parent.width
            columns: 1

            GroupBox
            {
                title: qsTr("Target Scene")
                Layout.fillWidth: true
                font.family: UISettings.robotoFontName
                font.pixelSize: UISettings.textSizeDefault
                palette.windowText: UISettings.fgMain

                GridLayout
                {
                    width: parent.width
                    columns: 3

                    // row 1
                    CustomCheckBox
                    {
                        id: newSceneCheck
                        implicitHeight: UISettings.listItemHeight
                        implicitWidth: implicitHeight
                        ButtonGroup.group: sceneTypeGroup
                        checked: true
                    }

                    RobotoText
                    {
                        height: UISettings.listItemHeight
                        label: qsTr("Dump to a new Scene")
                    }

                    CustomTextEdit
                    {
                        id: nameInputBox
                        Layout.fillWidth: true
                        text: qsTr("New Scene") + " " + nextFunctionId
                        onAccepted: popupRoot.accept()
                    }

                    // row 2
                    CustomCheckBox
                    {
                        id: existingSceneCheck
                        implicitHeight: UISettings.listItemHeight
                        implicitWidth: implicitHeight
                        ButtonGroup.group: sceneTypeGroup
                    }

                    RobotoText
                    {
                        height: UISettings.listItemHeight
                        label: qsTr("Dump to existing Scene")
                    }

                    RobotoText
                    {
                        visible: selectedFunctionsCount === 0
                        label: qsTr("(None selected)")
                    }

                    IconTextEntry
                    {
                        id: funcBox
                        visible: selectedFunctionsCount
                        Layout.fillWidth: true

                        tFontSize: UISettings.textSizeDefault

                        tLabel: func ? func.name : ""
                        functionType: func ? func.type : -1
                    }
                }
            }

            GroupBox
            {
                title: qsTr("Channels to dump")
                Layout.fillWidth: true
                font.family: UISettings.robotoFontName
                font.pixelSize: UISettings.textSizeDefault
                palette.windowText: UISettings.fgMain

                GridLayout
                {
                    width: parent.width
                    columns: 4
                    columnSpacing: 5

                    // row 1
                    RowLayout
                    {
                        Layout.columnSpan: 4
                        Layout.fillWidth: true

                        CustomCheckBox
                        {
                            id: allChannelsCheck
                            implicitHeight: UISettings.listItemHeight
                            implicitWidth: implicitHeight
                            ButtonGroup.group: dumpTypeGroup
                        }
                        RobotoText
                        {
                            implicitHeight: UISettings.listItemHeight
                            Layout.fillWidth: true
                            label: qsTr("Dump all the available channels") + " (" + universesCount + " " + qsTr("Universes") + ", " +
                                   fixtureManager.fixturesCount + " " + qsTr("Fixtures") + ")"
                        }
                    }

                    Rectangle
                    {
                        width: UISettings.bigItemHeight / 2
                        height: UISettings.listItemHeight
                        color: "transparent"
                    }

                    // row 2
                    RowLayout
                    {
                        Layout.columnSpan: 3
                        Layout.fillWidth: true

                        CustomCheckBox
                        {
                            id: nonZeroCheck
                            implicitHeight: UISettings.listItemHeight
                            implicitWidth: implicitHeight
                            checked: false
                            enabled: allChannelsCheck.checked
                        }
                        RobotoText
                        {
                            implicitHeight: UISettings.listItemHeight
                            Layout.fillWidth: true
                            label: qsTr("Dump only non-zero values")
                            enabled: allChannelsCheck.checked
                        }
                    }

                    // row 3
                    RowLayout
                    {
                        Layout.columnSpan: 4
                        Layout.fillWidth: true

                        CustomCheckBox
                        {
                            id: activeChannelsCheck
                            implicitHeight: UISettings.listItemHeight
                            implicitWidth: implicitHeight
                            enabled: capabilityMask !== 0 ? true : false
                            checked: true
                            ButtonGroup.group: dumpTypeGroup
                        }
                        RobotoText
                        {
                            enabled: capabilityMask !== 0 ? true : false
                            implicitHeight: UISettings.listItemHeight
                            label: qsTr("Dump the selected fixture channels")
                        }
                    }

                    // row 5
                    RobotoText
                    {
                        Layout.columnSpan: 4
                        visible: capabilityMask !== 0 ? true : false
                        label: qsTr("Detected channel types")
                    }

                    // row 6
                    CustomCheckBox
                    {
                        id: intTypeCheck
                        visible: capabilityMask & App.DimmerType
                        implicitHeight: UISettings.listItemHeight
                        implicitWidth: implicitHeight
                        Layout.alignment: Qt.AlignRight
                        checked: channelSetMask & App.DimmerType

                    }
                    IconTextEntry
                    {
                        visible: capabilityMask & App.DimmerType
                        Layout.fillWidth: true
                        iSrc: "qrc:/intensity.svg"
                        tLabel: qsTr("Intensity")
                    }

                    CustomCheckBox
                    {
                        id: colTypeCheck
                        visible: capabilityMask & App.ColorType
                        implicitHeight: UISettings.listItemHeight
                        implicitWidth: implicitHeight
                        Layout.alignment: Qt.AlignRight
                        checked: channelSetMask & App.ColorType
                    }
                    IconTextEntry
                    {
                        visible: capabilityMask & App.ColorType
                        Layout.fillWidth: true
                        iSrc: "qrc:/color.svg"
                        tLabel: qsTr("RGB/CMY/WAUV")
                    }

                    // row 7
                    CustomCheckBox
                    {
                        id: colMacroTypeCheck
                        visible: capabilityMask & App.ColorMacroType
                        implicitHeight: UISettings.listItemHeight
                        implicitWidth: implicitHeight
                        Layout.alignment: Qt.AlignRight
                        checked: channelSetMask & App.ColorMacroType
                    }
                    IconTextEntry
                    {
                        visible: capabilityMask & App.ColorMacroType
                        Layout.fillWidth: true
                        iSrc: "qrc:/colorwheel.svg"
                        tLabel: qsTr("Color macros")
                    }

                    CustomCheckBox
                    {
                        id: goboTypeCheck
                        visible: capabilityMask & App.GoboType
                        implicitHeight: UISettings.listItemHeight
                        implicitWidth: implicitHeight
                        Layout.alignment: Qt.AlignRight
                        checked: channelSetMask & App.GoboType
                    }
                    IconTextEntry
                    {
                        visible: capabilityMask & App.GoboType
                        Layout.fillWidth: true
                        iSrc: "qrc:/gobo.svg"
                        tLabel: qsTr("Gobo")
                    }

                    // row 8
                    CustomCheckBox
                    {
                        id: panTypeCheck
                        visible: capabilityMask & App.PanType
                        implicitHeight: UISettings.listItemHeight
                        implicitWidth: implicitHeight
                        Layout.alignment: Qt.AlignRight
                        checked: channelSetMask & App.PanType
                    }
                    IconTextEntry
                    {
                        visible: capabilityMask & App.PanType
                        Layout.fillWidth: true
                        iSrc: "qrc:/pan.svg"
                        tLabel: qsTr("Pan")
                    }

                    CustomCheckBox
                    {
                        id: tiltTypeCheck
                        visible: capabilityMask & App.TiltType
                        implicitHeight: UISettings.listItemHeight
                        implicitWidth: implicitHeight
                        Layout.alignment: Qt.AlignRight
                        checked: channelSetMask & App.TiltType
                    }
                    IconTextEntry
                    {
                        visible: capabilityMask & App.TiltType
                        Layout.fillWidth: true
                        iSrc: "qrc:/tilt.svg"
                        tLabel: qsTr("Tilt")
                    }

                    // row 9
                    CustomCheckBox
                    {
                        id: speedTypeCheck
                        visible: capabilityMask & App.SpeedType
                        implicitHeight: UISettings.listItemHeight
                        implicitWidth: implicitHeight
                        Layout.alignment: Qt.AlignRight
                        checked: channelSetMask & App.SpeedType
                    }
                    IconTextEntry
                    {
                        visible: capabilityMask & App.SpeedType
                        Layout.fillWidth: true
                        iSrc: "qrc:/speed.svg"
                        tLabel: qsTr("Speed")
                    }

                    CustomCheckBox
                    {
                        id: shutterTypeCheck
                        visible: capabilityMask & App.ShutterType
                        implicitHeight: UISettings.listItemHeight
                        implicitWidth: implicitHeight
                        Layout.alignment: Qt.AlignRight
                        checked: channelSetMask & App.ShutterType
                    }
                    IconTextEntry
                    {
                        visible: capabilityMask & App.ShutterType
                        Layout.fillWidth: true
                        iSrc: "qrc:/shutter.svg"
                        tLabel: qsTr("Shutter/Strobe")
                    }

                    // row 10
                    CustomCheckBox
                    {
                        id: prismTypeCheck
                        visible: capabilityMask & App.PrismType
                        implicitHeight: UISettings.listItemHeight
                        implicitWidth: implicitHeight
                        Layout.alignment: Qt.AlignRight
                        checked: channelSetMask & App.PrismType
                    }
                    IconTextEntry
                    {
                        visible: capabilityMask & App.PrismType
                        Layout.fillWidth: true
                        iSrc: "qrc:/prism.svg"
                        tLabel: qsTr("Prism")
                    }

                    CustomCheckBox
                    {
                        id: beamTypeCheck
                        visible: capabilityMask & App.BeamType
                        implicitHeight: UISettings.listItemHeight
                        implicitWidth: implicitHeight
                        Layout.alignment: Qt.AlignRight
                        checked: channelSetMask & App.BeamType
                    }
                    IconTextEntry
                    {
                        visible: capabilityMask & App.BeamType
                        Layout.fillWidth: true
                        iSrc: "qrc:/beam.svg"
                        tLabel: qsTr("Beam")
                    }

                    // row 11
                    CustomCheckBox
                    {
                        id: effectTypeCheck
                        visible: capabilityMask & App.EffectType
                        implicitHeight: UISettings.listItemHeight
                        implicitWidth: implicitHeight
                        Layout.alignment: Qt.AlignRight
                        checked: channelSetMask & App.EffectType
                    }
                    IconTextEntry
                    {
                        visible: capabilityMask & App.EffectType
                        Layout.fillWidth: true
                        iSrc: "qrc:/star.svg"
                        tLabel: qsTr("Effect")
                    }

                    CustomCheckBox
                    {
                        id: maintTypeCheck
                        visible: capabilityMask & App.MaintenanceType
                        implicitHeight: UISettings.listItemHeight
                        implicitWidth: implicitHeight
                        Layout.alignment: Qt.AlignRight
                        checked: channelSetMask & App.MaintenanceType
                    }
                    IconTextEntry
                    {
                        visible: capabilityMask & App.MaintenanceType
                        Layout.fillWidth: true
                        iSrc: "qrc:/configure.svg"
                        tLabel: qsTr("Maintenance")
                    }
                } // GridLayout
            } // GroupBox
        } // GridLayout
}
