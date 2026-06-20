/*
  Q Light Controller Plus
  Position3DTool.qml

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
import QtQuick.Layouts

import org.qlcplus.classes 1.0
import "."

Rectangle
{
    id: pos3DToolRoot
    width: UISettings.bigItemHeight * 2.5
    height: colLayout.implicitHeight + paletteBox.height + 10
    color: UISettings.bgStrong

    property alias showPalette: paletteBox.visible
    property bool isLoading: false
    property bool metersMode: true

    readonly property real metersToFeet: 3.28084
    readonly property real feetToMeters: 0.3048

    signal close()

    function toDisplay(meters) { return metersMode ? meters : meters * metersToFeet }
    function toMeters(display) { return metersMode ? display : display * feetToMeters }

    function loadPalette(id)
    {
        isLoading = true
        let palette = paletteManager.getPalette(id)
        if (palette && palette.type === QLCPalette.Position3D)
        {
            paletteToolbar.text = palette.name
            paletteBox.editPalette(palette)
            xSpin.realValue = toDisplay(palette.floatValue1)
            ySpin.realValue = toDisplay(palette.floatValue2)
            zSpin.realValue = toDisplay(palette.floatValue3)
            View3D.setPosition3DMarker(Qt.vector3d(palette.floatValue1,
                                                   palette.floatValue2,
                                                   palette.floatValue3))
        }
        isLoading = false
    }

    function updateValues()
    {
        if (isLoading)
            return
        let xm = toMeters(xSpin.realValue)
        let ym = toMeters(ySpin.realValue)
        let zm = toMeters(zSpin.realValue)
        paletteManager.updatePalette(paletteBox.editingPalette, xm, ym, zm)
        if (paletteBox.isEditing || paletteBox.checked)
            paletteBox.updatePreview()

        View3D.setPosition3DMarker(Qt.vector3d(xm, ym, zm))
    }

    Component.onCompleted: View3D.setPosition3DMarkerVisible(true)
    Component.onDestruction: View3D.setPosition3DMarkerVisible(false)

    MouseArea
    {
        anchors.fill: parent
        onWheel: (wheel) => { return false }
    }

    ColumnLayout
    {
        id: colLayout
        width: parent.width
        spacing: 0

        EditorTopBar
        {
            id: paletteToolbar
            onBackClicked: pos3DToolRoot.parent.dismiss()
            onTextChanged: paletteBox.setName(text)

            Rectangle
            {
                height: paletteToolbar.height - 4
                width: UISettings.iconSizeDefault * 1.1
                Layout.alignment: Qt.AlignVCenter
                border.width: 2
                border.color: "white"
                radius: 5
                color: UISettings.sectionHeader

                RobotoText
                {
                    height: parent.height
                    anchors.horizontalCenter: parent.horizontalCenter
                    label: pos3DToolRoot.metersMode ? "m" : "ft"
                    fontSize: UISettings.textSizeDefault
                    fontBold: true
                }

                MouseArea
                {
                    anchors.fill: parent
                    onClicked:
                    {
                        pos3DToolRoot.isLoading = true
                        if (pos3DToolRoot.metersMode)
                        {
                            xSpin.realValue = xSpin.realValue * pos3DToolRoot.metersToFeet
                            ySpin.realValue = ySpin.realValue * pos3DToolRoot.metersToFeet
                            zSpin.realValue = zSpin.realValue * pos3DToolRoot.metersToFeet
                        }
                        else
                        {
                            xSpin.realValue = xSpin.realValue * pos3DToolRoot.feetToMeters
                            ySpin.realValue = ySpin.realValue * pos3DToolRoot.feetToMeters
                            zSpin.realValue = zSpin.realValue * pos3DToolRoot.feetToMeters
                        }
                        pos3DToolRoot.metersMode = !pos3DToolRoot.metersMode
                        pos3DToolRoot.isLoading = false
                    }
                }
            }
        }

        // X / Y / Z spinboxes
        GridLayout
        {
            Layout.fillWidth: true
            Layout.leftMargin: 6
            Layout.rightMargin: 6
            Layout.topMargin: 4
            columns: 2
            rowSpacing: 4
            columnSpacing: 6

            RobotoText { height: UISettings.listItemHeight; label: "X" }
            CustomDoubleSpinBox
            {
                id: xSpin
                Layout.fillWidth: true
                realFrom: -1000
                realTo: 1000
                realStep: 0.1
                suffix: pos3DToolRoot.metersMode ? " m" : " ft"
                onValueChanged: pos3DToolRoot.updateValues()
            }

            RobotoText { height: UISettings.listItemHeight; label: "Y" }
            CustomDoubleSpinBox
            {
                id: ySpin
                Layout.fillWidth: true
                realFrom: -1000
                realTo: 1000
                realStep: 0.1
                suffix: pos3DToolRoot.metersMode ? " m" : " ft"
                onValueChanged: pos3DToolRoot.updateValues()
            }

            RobotoText { height: UISettings.listItemHeight; label: "Z" }
            CustomDoubleSpinBox
            {
                id: zSpin
                Layout.fillWidth: true
                realFrom: -1000
                realTo: 1000
                realStep: 0.1
                suffix: pos3DToolRoot.metersMode ? " m" : " ft"
                onValueChanged: pos3DToolRoot.updateValues()
            }
        }
    }

    PaletteFanningBox
    {
        id: paletteBox
        y: colLayout.implicitHeight + 5
        width: parent.width
        paletteType: QLCPalette.Position3D
    }
}
