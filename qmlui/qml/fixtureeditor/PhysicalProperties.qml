/*
  Q Light Controller Plus
  PhysicalProperties.qml

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

import QtQuick 2.2
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.4

import org.qlcplus.classes 1.0
import "."

GridLayout
{
    id: controlRoot
    columns: 2

    property PhysicalEdit phy: null

    GroupBox
    {
        title: qsTr("Bulb")
        Layout.fillWidth: true
        font.family: UISettings.robotoFontName
        font.pixelSize: UISettings.textSizeDefault
        palette.windowText: UISettings.fgMain

        GridLayout
        {
            width: parent.width
            columns: 2

            RobotoText { label: qsTr("Type") }
            ComboBox
            {
                Layout.fillWidth: true
                palette.base: UISettings.bgControl
                palette.window: UISettings.bgControl
                palette.text: UISettings.fgMain
                palette.highlightedText: UISettings.bgMedium

                model: ["LED", "CDM 70W", "CDM 150W", "CP29 5000W", "CP41 2000W", "CP60 1000W",
                        "CP61 1000W", "CP62 1000W", "CP86 500W", "CP87 500W", "CP88 500W",
                        "EFP 100W", "EFP 150W", "EFR 100W", "EFR 150W", "ELC 250W",
                        "HMI 150W", "HMI 250W", "HMI 400W", "HMI 575W", "HMI 700W",
                        "HMI 1200W", "HMI 4000W", "HSD 150W", "HSD 200W", "HSD 250W",
                        "HSD 575W", "HTI 150W", "HTI 250W", "HTI 300W", "HTI 400W",
                        "HTI 575W", "HTI 700W", "HTI 1200W", "HTI 2500W", "MSD 200W",
                        "MSD 250W", "MSD 275W", "MSD Platinum 15 R 300W", "MSD 575W",
                        "MSR 575W", "MSR 700W", "MSR 1200W"]
                editable: true

                editText: phy ? phy.bulbType : ""
                onEditTextChanged: if (phy) phy.bulbType = editText

                Rectangle
                {
                    anchors.fill: parent
                    color: "black"
                    opacity: 0.7
                    visible: !controlRoot.enabled
                }
            }
            RobotoText { label: qsTr("Lumens") }
            CustomSpinBox
            {
                Layout.fillWidth: true
                enabled: controlRoot.enabled
                from: 0
                to: 999999
                stepSize: 1
                value: phy ? phy.bulbLumens : 0
                onValueModified: if (phy) phy.bulbLumens = value
            }
            RobotoText { label: qsTr("Colour Temp (K)") }
            CustomSpinBox
            {
                Layout.fillWidth: true
                enabled: controlRoot.enabled
                from: 0
                to: 999999
                stepSize: 1
                value: phy ? phy.bulbColorTemperature : 0
                onValueModified: if (phy) phy.bulbColorTemperature = value
            }
        }
    }

    GroupBox
    {
        title: qsTr("Lens")
        Layout.fillWidth: true
        font.family: UISettings.robotoFontName
        font.pixelSize: UISettings.textSizeDefault
        palette.windowText: UISettings.fgMain

        GridLayout
        {
            width: parent.width
            columns: 2

            RobotoText { label: qsTr("Type") }
            ComboBox
            {
                Layout.fillWidth: true
                enabled: controlRoot.enabled
                palette.base: UISettings.bgControl
                palette.window: UISettings.bgControl
                palette.text: UISettings.fgMain
                palette.highlightedText: UISettings.bgMedium
                model: ["Other", "PC", "Fresnel"]
                editable: true

                editText: phy ? phy.lensType : ""
                onEditTextChanged: if (phy) phy.lensType = editText

                Rectangle
                {
                    anchors.fill: parent
                    color: "black"
                    opacity: 0.7
                    visible: !controlRoot.enabled
                }
            }
            RobotoText { label: qsTr("Min Degrees") }
            CustomSpinBox
            {
                Layout.fillWidth: true
                enabled: controlRoot.enabled
                from: 0
                to: 999999
                stepSize: 1
                suffix: "°"
                value: phy ? phy.lensDegreesMin : 0
                onValueModified: if (phy) phy.lensDegreesMin = value
            }
            RobotoText { label: qsTr("Max Degrees") }
            CustomSpinBox
            {
                Layout.fillWidth: true
                enabled: controlRoot.enabled
                from: 0
                to: 999999
                stepSize: 1
                suffix: "°"
                value: phy ? phy.lensDegreesMax : 0
                onValueModified: if (phy) phy.lensDegreesMax = value
            }
        }
    }

    GroupBox
    {
        title: qsTr("Head(s)")
        Layout.fillWidth: true
        font.family: UISettings.robotoFontName
        font.pixelSize: UISettings.textSizeDefault
        palette.windowText: UISettings.fgMain

        GridLayout
        {
            width: parent.width
            columns: 2

            RobotoText { label: qsTr("Type") }
            ComboBox
            {
                Layout.fillWidth: true
                enabled: controlRoot.enabled
                palette.base: UISettings.bgControl
                palette.window: UISettings.bgControl
                palette.text: UISettings.fgMain
                palette.highlightedText: UISettings.bgMedium
                model: ["Fixed", "Head", "Mirror", "Barrel"]
                editable: true

                editText: phy ? phy.focusType : ""
                onEditTextChanged: if (phy) phy.focusType = editText

                Rectangle
                {
                    anchors.fill: parent
                    color: "black"
                    opacity: 0.7
                    visible: !controlRoot.enabled
                }
            }
            RobotoText { label: qsTr("Pan Max Degrees") }
            CustomSpinBox
            {
                Layout.fillWidth: true
                enabled: controlRoot.enabled
                from: 0
                to: 999999
                stepSize: 1
                suffix: "°"
                value: phy ? phy.focusPanMax : 0
                onValueModified: if (phy) phy.focusPanMax = value
            }
            RobotoText { label: qsTr("Tilt Max Degrees") }
            CustomSpinBox
            {
                Layout.fillWidth: true
                enabled: controlRoot.enabled
                from: 0
                to: 999999
                stepSize: 1
                suffix: "°"
                value: phy ? phy.focusTiltMax : 0
                onValueModified: if (phy) phy.focusTiltMax = value
            }
            RobotoText { label: qsTr("Layout\n(Columns x Rows)") }
            RowLayout
            {
                Layout.fillWidth: true
                spacing: 5

                CustomSpinBox
                {
                    id: layoutColumns
                    Layout.fillWidth: true
                    enabled: controlRoot.enabled
                    from: 1
                    to: 999
                    stepSize: 1
                    value: phy ? phy.layoutSize.width : 1
                    onValueModified: if (phy) phy.layoutSize = Qt.size(value, layoutRows.value)
                }
                RobotoText { label: "x"; textVAlign: Text.AlignVCenter }
                CustomSpinBox
                {
                    id: layoutRows
                    Layout.fillWidth: true
                    enabled: controlRoot.enabled
                    from: 1
                    to: 999
                    stepSize: 1
                    value: phy ? phy.layoutSize.height : 1
                    onValueModified: if (phy) phy.layoutSize = Qt.size(layoutColumns.value, value)
                }
            }
        }
    }

    GroupBox
    {
        title: qsTr("Dimensions")
        Layout.fillWidth: true
        font.family: UISettings.robotoFontName
        font.pixelSize: UISettings.textSizeDefault
        palette.windowText: UISettings.fgMain

        GridLayout
        {
            width: parent.width
            columns: 2

            RobotoText { label: qsTr("Weight") }
            CustomDoubleSpinBox
            {
                Layout.fillWidth: true
                enabled: controlRoot.enabled
                realFrom: 0
                realTo: 999999
                stepSize: 1
                suffix: "kg"
                realValue: phy ? phy.weight : 0
                onRealValueChanged: if (phy) phy.weight = realValue
            }
            RobotoText { label: qsTr("Width") }
            CustomSpinBox
            {
                Layout.fillWidth: true
                enabled: controlRoot.enabled
                from: 0
                to: 999999
                stepSize: 1
                suffix: "mm"
                value: phy ? phy.width : 0
                onValueModified: if (phy) phy.width = value
            }
            RobotoText { label: qsTr("Height") }
            CustomSpinBox
            {
                Layout.fillWidth: true
                enabled: controlRoot.enabled
                from: 0
                to: 999999
                stepSize: 1
                suffix: "mm"
                value: phy ? phy.height : 0
                onValueModified: if (phy) phy.height = value
            }
            RobotoText { label: qsTr("Depth") }
            CustomSpinBox
            {
                Layout.fillWidth: true
                enabled: controlRoot.enabled
                from: 0
                to: 999999
                stepSize: 1
                suffix: "mm"
                value: phy ? phy.depth : 0
                onValueModified: if (phy) phy.depth = value
            }
        }
    }

    GroupBox
    {
        title: qsTr("Electrical")
        Layout.fillWidth: true
        font.family: UISettings.robotoFontName
        font.pixelSize: UISettings.textSizeDefault
        palette.windowText: UISettings.fgMain

        GridLayout
        {
            width: parent.width
            columns: 2

            RobotoText { label: qsTr("Power Consumption") }
            CustomSpinBox
            {
                Layout.fillWidth: true
                enabled: controlRoot.enabled
                from: 0
                to: 999999
                stepSize: 1
                suffix: "W"
                value: phy ? phy.powerConsumption : 0
                onValueModified: if (phy) phy.powerConsumption = value
            }

            RobotoText { label: qsTr("DMX Connector") }
            ComboBox
            {
                Layout.fillWidth: true
                enabled: controlRoot.enabled
                palette.base: UISettings.bgControl
                palette.window: UISettings.bgControl
                palette.text: UISettings.fgMain
                palette.highlightedText: UISettings.bgMedium
                model: ["3-pin", "5-pin", "3-pin and 5-pin", "3.5 mm stereo jack", "Other"]
                editable: true

                editText: phy ? phy.dmxConnector : ""
                onEditTextChanged: if (phy) phy.dmxConnector = editText

                Rectangle
                {
                    anchors.fill: parent
                    color: "black"
                    opacity: 0.7
                    visible: !controlRoot.enabled
                }
            }
        }
    }
}
