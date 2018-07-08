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
    title: qsTr("Enter a name for the scene")

    property bool show: !dontAskCheck.checked
    property int channelsMask: contextManager ? contextManager.dumpChannelMask : 0
    property alias sceneName: nameInputBox.inputText

    function getChannelsMask()
    {
        var mask = 0

        if (intTypeCheck.checked)
            mask |= ContextManager.DimmerType
        if (colTypeCheck.checked)
            mask |= ContextManager.ColorType
        if (colMacroTypeCheck.checked)
            mask |= ContextManager.ColorMacroType
        if (goboTypeCheck.checked)
            mask |= ContextManager.GoboType
        if (panTypeCheck.checked)
            mask |= ContextManager.PanType
        if (tiltTypeCheck.checked)
            mask |= ContextManager.TiltType
        if (speedTypeCheck.checked)
            mask |= ContextManager.SpeedType
        if (shutterTypeCheck.checked)
            mask |= ContextManager.ShutterType
        if (prismTypeCheck.checked)
            mask |= ContextManager.PrismType
        if (beamTypeCheck.checked)
            mask |= ContextManager.BeamType
        if (effectTypeCheck.checked)
            mask |= ContextManager.EffectType
        if (maintTypeCheck.checked)
            mask |= ContextManager.MaintenanceType

        return mask
    }

    function focusEditItem()
    {
        nameInputBox.selectAndFocus()
    }

    contentItem:
        GridLayout
        {
            columns: 4
            columnSpacing: 5

            // row 1
            RowLayout
            {
                Layout.columnSpan: 4

                RobotoText
                {
                    height: UISettings.listItemHeight
                    label: qsTr("Scene name")
                }

                CustomTextEdit
                {
                    id: nameInputBox
                    Layout.fillWidth: true
                    inputText: qsTr("New Scene")
                    onEnterPressed: popupRoot.accept()
                }
            }

            // row 2
            CustomCheckBox
            {
                id: dontAskCheck
                implicitHeight: UISettings.listItemHeight
                implicitWidth: implicitHeight
                Layout.alignment: Qt.AlignRight
                autoExclusive: false
            }
            RobotoText { label: qsTr("Don't ask again") }

            // row 3
            RobotoText
            {
                Layout.columnSpan: 4
                label: qsTr("Available channel types")
            }

            // row 4
            CustomCheckBox
            {
                id: intTypeCheck
                visible: channelsMask & ContextManager.DimmerType
                implicitHeight: UISettings.listItemHeight
                implicitWidth: implicitHeight
                Layout.alignment: Qt.AlignRight
                autoExclusive: false
                checked: channelsMask & ContextManager.DimmerType

            }
            IconTextEntry
            {
                visible: channelsMask & ContextManager.DimmerType
                Layout.fillWidth: true
                iSrc: "qrc:/intensity.svg"
                tLabel: qsTr("Intensity")
            }

            CustomCheckBox
            {
                id: colTypeCheck
                visible: channelsMask & ContextManager.ColorType
                implicitHeight: UISettings.listItemHeight
                implicitWidth: implicitHeight
                Layout.alignment: Qt.AlignRight
                autoExclusive: false
                checked: channelsMask & ContextManager.ColorType
            }
            IconTextEntry
            {
                visible: channelsMask & ContextManager.ColorType
                Layout.fillWidth: true
                iSrc: "qrc:/color.svg"
                tLabel: qsTr("RGB/CMY/WAUV")
            }

            // row 5
            CustomCheckBox
            {
                id: colMacroTypeCheck
                visible: channelsMask & ContextManager.ColorMacroType
                implicitHeight: UISettings.listItemHeight
                implicitWidth: implicitHeight
                Layout.alignment: Qt.AlignRight
                autoExclusive: false
                checked: channelsMask & ContextManager.ColorMacroType
            }
            IconTextEntry
            {
                visible: channelsMask & ContextManager.ColorMacroType
                Layout.fillWidth: true
                iSrc: "qrc:/colorwheel.svg"
                tLabel: qsTr("Color macros")
            }

            CustomCheckBox
            {
                id: goboTypeCheck
                visible: channelsMask & ContextManager.GoboType
                implicitHeight: UISettings.listItemHeight
                implicitWidth: implicitHeight
                Layout.alignment: Qt.AlignRight
                autoExclusive: false
                checked: channelsMask & ContextManager.GoboType
            }
            IconTextEntry
            {
                visible: channelsMask & ContextManager.GoboType
                Layout.fillWidth: true
                iSrc: "qrc:/gobo.svg"
                tLabel: qsTr("Gobo")
            }

            // row 6
            CustomCheckBox
            {
                id: panTypeCheck
                visible: channelsMask & ContextManager.PanType
                implicitHeight: UISettings.listItemHeight
                implicitWidth: implicitHeight
                Layout.alignment: Qt.AlignRight
                autoExclusive: false
                checked: channelsMask & ContextManager.PanType
            }
            IconTextEntry
            {
                visible: channelsMask & ContextManager.PanType
                Layout.fillWidth: true
                iSrc: "qrc:/pan.svg"
                tLabel: qsTr("Pan")
            }

            CustomCheckBox
            {
                id: tiltTypeCheck
                visible: channelsMask & ContextManager.TiltType
                implicitHeight: UISettings.listItemHeight
                implicitWidth: implicitHeight
                Layout.alignment: Qt.AlignRight
                autoExclusive: false
                checked: channelsMask & ContextManager.TiltType
            }
            IconTextEntry
            {
                visible: channelsMask & ContextManager.TiltType
                Layout.fillWidth: true
                iSrc: "qrc:/tilt.svg"
                tLabel: qsTr("Tilt")
            }

            // row 7
            CustomCheckBox
            {
                id: speedTypeCheck
                visible: channelsMask & ContextManager.SpeedType
                implicitHeight: UISettings.listItemHeight
                implicitWidth: implicitHeight
                Layout.alignment: Qt.AlignRight
                autoExclusive: false
                checked: channelsMask & ContextManager.SpeedType
            }
            IconTextEntry
            {
                visible: channelsMask & ContextManager.SpeedType
                Layout.fillWidth: true
                iSrc: "qrc:/speed.svg"
                tLabel: qsTr("Speed")
            }

            CustomCheckBox
            {
                id: shutterTypeCheck
                visible: channelsMask & ContextManager.ShutterType
                implicitHeight: UISettings.listItemHeight
                implicitWidth: implicitHeight
                Layout.alignment: Qt.AlignRight
                autoExclusive: false
                checked: channelsMask & ContextManager.ShutterType
            }
            IconTextEntry
            {
                visible: channelsMask & ContextManager.ShutterType
                Layout.fillWidth: true
                iSrc: "qrc:/shutter.svg"
                tLabel: qsTr("Shutter/Strobe")
            }

            // row 8
            CustomCheckBox
            {
                id: prismTypeCheck
                visible: channelsMask & ContextManager.PrismType
                implicitHeight: UISettings.listItemHeight
                implicitWidth: implicitHeight
                Layout.alignment: Qt.AlignRight
                autoExclusive: false
                checked: channelsMask & ContextManager.PrismType
            }
            IconTextEntry
            {
                visible: channelsMask & ContextManager.PrismType
                Layout.fillWidth: true
                iSrc: "qrc:/prism.svg"
                tLabel: qsTr("Prism")
            }

            CustomCheckBox
            {
                id: beamTypeCheck
                visible: channelsMask & ContextManager.BeamType
                implicitHeight: UISettings.listItemHeight
                implicitWidth: implicitHeight
                Layout.alignment: Qt.AlignRight
                autoExclusive: false
                checked: channelsMask & ContextManager.BeamType
            }
            IconTextEntry
            {
                visible: channelsMask & ContextManager.BeamType
                Layout.fillWidth: true
                iSrc: "qrc:/beam.svg"
                tLabel: qsTr("Beam")
            }

            // row 9
            CustomCheckBox
            {
                id: effectTypeCheck
                visible: channelsMask & ContextManager.EffectType
                implicitHeight: UISettings.listItemHeight
                implicitWidth: implicitHeight
                Layout.alignment: Qt.AlignRight
                autoExclusive: false
                checked: channelsMask & ContextManager.EffectType
            }
            IconTextEntry
            {
                visible: channelsMask & ContextManager.EffectType
                Layout.fillWidth: true
                iSrc: "qrc:/star.svg"
                tLabel: qsTr("Effect")
            }

            CustomCheckBox
            {
                id: maintTypeCheck
                visible: channelsMask & ContextManager.MaintenanceType
                implicitHeight: UISettings.listItemHeight
                implicitWidth: implicitHeight
                Layout.alignment: Qt.AlignRight
                autoExclusive: false
                checked: channelsMask & ContextManager.MaintenanceType
            }
            IconTextEntry
            {
                visible: channelsMask & ContextManager.MaintenanceType
                Layout.fillWidth: true
                iSrc: "qrc:/configure.svg"
                tLabel: qsTr("Maintenance")
            }
        }

}
