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
    title: qsTr("Enter a name for the scene")

    property bool show: !dontAskCheck.checked
    property int channelsMask: 0
    property alias sceneName: nameInputBox.text

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
                    text: qsTr("New Scene")
                    onAccepted: popupRoot.accept()
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
            RobotoText
            {
                Layout.columnSpan: 3
                label: qsTr("Don't ask again")
            }

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
                visible: channelsMask & App.DimmerType
                implicitHeight: UISettings.listItemHeight
                implicitWidth: implicitHeight
                Layout.alignment: Qt.AlignRight
                autoExclusive: false
                checked: channelsMask & App.DimmerType

            }
            IconTextEntry
            {
                visible: channelsMask & App.DimmerType
                Layout.fillWidth: true
                iSrc: "qrc:/intensity.svg"
                tLabel: qsTr("Intensity")
            }

            CustomCheckBox
            {
                id: colTypeCheck
                visible: channelsMask & App.ColorType
                implicitHeight: UISettings.listItemHeight
                implicitWidth: implicitHeight
                Layout.alignment: Qt.AlignRight
                autoExclusive: false
                checked: channelsMask & App.ColorType
            }
            IconTextEntry
            {
                visible: channelsMask & App.ColorType
                Layout.fillWidth: true
                iSrc: "qrc:/color.svg"
                tLabel: qsTr("RGB/CMY/WAUV")
            }

            // row 5
            CustomCheckBox
            {
                id: colMacroTypeCheck
                visible: channelsMask & App.ColorMacroType
                implicitHeight: UISettings.listItemHeight
                implicitWidth: implicitHeight
                Layout.alignment: Qt.AlignRight
                autoExclusive: false
                checked: channelsMask & App.ColorMacroType
            }
            IconTextEntry
            {
                visible: channelsMask & App.ColorMacroType
                Layout.fillWidth: true
                iSrc: "qrc:/colorwheel.svg"
                tLabel: qsTr("Color macros")
            }

            CustomCheckBox
            {
                id: goboTypeCheck
                visible: channelsMask & App.GoboType
                implicitHeight: UISettings.listItemHeight
                implicitWidth: implicitHeight
                Layout.alignment: Qt.AlignRight
                autoExclusive: false
                checked: channelsMask & App.GoboType
            }
            IconTextEntry
            {
                visible: channelsMask & App.GoboType
                Layout.fillWidth: true
                iSrc: "qrc:/gobo.svg"
                tLabel: qsTr("Gobo")
            }

            // row 6
            CustomCheckBox
            {
                id: panTypeCheck
                visible: channelsMask & App.PanType
                implicitHeight: UISettings.listItemHeight
                implicitWidth: implicitHeight
                Layout.alignment: Qt.AlignRight
                autoExclusive: false
                checked: channelsMask & App.PanType
            }
            IconTextEntry
            {
                visible: channelsMask & App.PanType
                Layout.fillWidth: true
                iSrc: "qrc:/pan.svg"
                tLabel: qsTr("Pan")
            }

            CustomCheckBox
            {
                id: tiltTypeCheck
                visible: channelsMask & App.TiltType
                implicitHeight: UISettings.listItemHeight
                implicitWidth: implicitHeight
                Layout.alignment: Qt.AlignRight
                autoExclusive: false
                checked: channelsMask & App.TiltType
            }
            IconTextEntry
            {
                visible: channelsMask & App.TiltType
                Layout.fillWidth: true
                iSrc: "qrc:/tilt.svg"
                tLabel: qsTr("Tilt")
            }

            // row 7
            CustomCheckBox
            {
                id: speedTypeCheck
                visible: channelsMask & App.SpeedType
                implicitHeight: UISettings.listItemHeight
                implicitWidth: implicitHeight
                Layout.alignment: Qt.AlignRight
                autoExclusive: false
                checked: channelsMask & App.SpeedType
            }
            IconTextEntry
            {
                visible: channelsMask & App.SpeedType
                Layout.fillWidth: true
                iSrc: "qrc:/speed.svg"
                tLabel: qsTr("Speed")
            }

            CustomCheckBox
            {
                id: shutterTypeCheck
                visible: channelsMask & App.ShutterType
                implicitHeight: UISettings.listItemHeight
                implicitWidth: implicitHeight
                Layout.alignment: Qt.AlignRight
                autoExclusive: false
                checked: channelsMask & App.ShutterType
            }
            IconTextEntry
            {
                visible: channelsMask & App.ShutterType
                Layout.fillWidth: true
                iSrc: "qrc:/shutter.svg"
                tLabel: qsTr("Shutter/Strobe")
            }

            // row 8
            CustomCheckBox
            {
                id: prismTypeCheck
                visible: channelsMask & App.PrismType
                implicitHeight: UISettings.listItemHeight
                implicitWidth: implicitHeight
                Layout.alignment: Qt.AlignRight
                autoExclusive: false
                checked: channelsMask & App.PrismType
            }
            IconTextEntry
            {
                visible: channelsMask & App.PrismType
                Layout.fillWidth: true
                iSrc: "qrc:/prism.svg"
                tLabel: qsTr("Prism")
            }

            CustomCheckBox
            {
                id: beamTypeCheck
                visible: channelsMask & App.BeamType
                implicitHeight: UISettings.listItemHeight
                implicitWidth: implicitHeight
                Layout.alignment: Qt.AlignRight
                autoExclusive: false
                checked: channelsMask & App.BeamType
            }
            IconTextEntry
            {
                visible: channelsMask & App.BeamType
                Layout.fillWidth: true
                iSrc: "qrc:/beam.svg"
                tLabel: qsTr("Beam")
            }

            // row 9
            CustomCheckBox
            {
                id: effectTypeCheck
                visible: channelsMask & App.EffectType
                implicitHeight: UISettings.listItemHeight
                implicitWidth: implicitHeight
                Layout.alignment: Qt.AlignRight
                autoExclusive: false
                checked: channelsMask & App.EffectType
            }
            IconTextEntry
            {
                visible: channelsMask & App.EffectType
                Layout.fillWidth: true
                iSrc: "qrc:/star.svg"
                tLabel: qsTr("Effect")
            }

            CustomCheckBox
            {
                id: maintTypeCheck
                visible: channelsMask & App.MaintenanceType
                implicitHeight: UISettings.listItemHeight
                implicitWidth: implicitHeight
                Layout.alignment: Qt.AlignRight
                autoExclusive: false
                checked: channelsMask & App.MaintenanceType
            }
            IconTextEntry
            {
                visible: channelsMask & App.MaintenanceType
                Layout.fillWidth: true
                iSrc: "qrc:/configure.svg"
                tLabel: qsTr("Maintenance")
            }
        } // GridLayout
}
