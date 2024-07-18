/*
  Q Light Controller Plus
  VCPageProperties.qml

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
    id: propsRoot
    color: "transparent"
    height: gridContents.height

    property VCFrame widgetRef: null
    property int gridItemsHeight: UISettings.listItemHeight

    onWidgetRefChanged:
    {
        if (widgetRef === null)
            return

        widthSpin.value = widgetRef.geometry.width
        heightSpin.value = widgetRef.geometry.height
    }

    GridLayout
    {
        id: gridContents
        width: parent.width
        columns: 2
        columnSpacing: 5
        rowSpacing: 4

        // row 1
        RobotoText
        {
            height: gridItemsHeight
            label: qsTr("Width")
        }
        CustomSpinBox
        {
            id: widthSpin
            Layout.fillWidth: true
            suffix: "px"
            from: 1
            to: 100000
            onValueChanged:
            {
                if (widgetRef)
                    widgetRef.geometry = Qt.rect(0, 0, value, widgetRef.geometry.height)
            }
        }

        // row 2
        RobotoText
        {
            height: gridItemsHeight
            label: qsTr("Height")
        }
        CustomSpinBox
        {
            id: heightSpin
            Layout.fillWidth: true
            suffix: "px"
            from: 1
            to: 100000
            onValueChanged:
            {
                if (widgetRef)
                    widgetRef.geometry = Qt.rect(0, 0, widgetRef.geometry.width, value)
            }
        }

        // row 3
        RobotoText
        {
            height: gridItemsHeight
            label: qsTr("Security")
        }

        GenericButton
        {
            Layout.fillWidth: true
            height: gridItemsHeight
            autoHeight: true
            label: qsTr("Set a PIN")
            onClicked: pinSetupPopup.open()

            PopupPINSetup
            {
                id: pinSetupPopup
                onAccepted:
                {
                    if (virtualConsole.setPagePIN(virtualConsole.selectedPage, currentPIN, newPIN) === false)
                        pinErrorPopup.open()
                }
            }

            CustomPopupDialog
            {
                id: pinErrorPopup
                title: qsTr("Error")
                message: qsTr("The entered PINs are either invalid or incorrect")
            }
        }

        // row 4
        RowLayout
        {
            Layout.columnSpan: 2
            Layout.fillWidth: true

            GenericButton
            {
                Layout.fillWidth: true
                height: gridItemsHeight
                autoHeight: true
                label: qsTr("Add page to the left")
                onClicked: virtualConsole.addPage(virtualConsole.selectedPage)
            }

            GenericButton
            {
                Layout.fillWidth: true
                height: gridItemsHeight
                autoHeight: true
                label: qsTr("Add page to the right")
                onClicked: virtualConsole.addPage(virtualConsole.selectedPage + 1)
            }
        }

        // row 5
        GenericButton
        {
            Layout.columnSpan: 2
            Layout.fillWidth: true
            bgColor: "darkred"
            hoverColor: "red"
            height: gridItemsHeight
            autoHeight: true
            label: qsTr("Delete this page")
            onClicked: deletePagePopup.open()

            CustomPopupDialog
            {
                id: deletePagePopup
                title: qsTr("Delete page")
                message: qsTr("Are you sure you want to delete the selected page?")
                onAccepted: virtualConsole.deletePage(virtualConsole.selectedPage)
            }
        }
    }
}
