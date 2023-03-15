/*
  Q Light Controller Plus
  VCFrameProperties.qml

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
    color: "transparent"
    height: fPropsColumn.height

    property VCFrame widgetRef: null
    property int gridItemsHeight: UISettings.listItemHeight

    Column
    {
        id: fPropsColumn
        width: parent.width
        spacing: 5

        SectionBox
        {
            sectionLabel: qsTr("Header")

            sectionContents:
              GridLayout
              {
                width: parent.width
                columns: 2
                columnSpacing: 5
                rowSpacing: 3

                // row 1
                RobotoText
                {
                    height: gridItemsHeight
                    Layout.fillWidth: true
                    label: qsTr("Show header")
                }

                CustomCheckBox
                {
                    implicitWidth: UISettings.iconSizeMedium
                    implicitHeight: implicitWidth
                    checked: widgetRef ? widgetRef.showHeader : false
                    onCheckedChanged: if (widgetRef) widgetRef.showHeader = checked
                }

                // row 2
                RobotoText
                {
                    height: gridItemsHeight
                    Layout.fillWidth: true
                    label: qsTr("Show enable button")
                }

                CustomCheckBox
                {
                    implicitWidth: UISettings.iconSizeMedium
                    implicitHeight: implicitWidth
                    checked: widgetRef ? widgetRef.showEnable : false
                    onCheckedChanged: if (widgetRef) widgetRef.showEnable = checked
                }
              }
        }

        SectionBox
        {
            sectionLabel: qsTr("Pages")

            sectionContents:
              GridLayout
              {
                width: parent.width
                columns: 2
                columnSpacing: 5
                rowSpacing: 3

                // row 1
                RobotoText
                {
                    height: gridItemsHeight
                    Layout.fillWidth: true
                    label: qsTr("Enable pages")
                }

                CustomCheckBox
                {
                    implicitWidth: UISettings.iconSizeMedium
                    implicitHeight: implicitWidth
                    checked: widgetRef ? widgetRef.multiPageMode : false
                    onCheckedChanged: if (widgetRef) widgetRef.multiPageMode = checked
                }

                // row 2
                RobotoText
                {
                    height: gridItemsHeight
                    Layout.fillWidth: true
                    label: qsTr("Circular pages scrolling")
                }

                CustomCheckBox
                {
                    implicitWidth: UISettings.iconSizeMedium
                    implicitHeight: implicitWidth
                    checked: widgetRef ? widgetRef.pagesLoop : false
                    onCheckedChanged: if (checked && widgetRef) widgetRef.pagesLoop = checked
                }

                // row 3
                RowLayout
                {
                    Layout.columnSpan: 2
                    Layout.fillWidth: true

                    RobotoText
                    {
                        height: gridItemsHeight
                        label: qsTr("Pages number")
                    }

                    CustomSpinBox
                    {
                        Layout.fillWidth: true
                        height: gridItemsHeight
                        from: 1
                        to: 100
                        value: widgetRef ? widgetRef.totalPagesNumber : 1
                        onValueChanged: if (widgetRef) widgetRef.totalPagesNumber = value
                    }
                }

                GenericButton
                {
                    enabled: widgetRef ? widgetRef.totalPagesNumber > 1 : false
                    Layout.columnSpan: 2
                    Layout.fillWidth: true
                    height: gridItemsHeight
                    label: qsTr("Clone first page widgets")
                    onClicked: if (widgetRef) widgetRef.cloneFirstPage()
                }
              }
        }

        SectionBox
        {
            sectionLabel: qsTr("Shortcuts")
            visible: widgetRef ? widgetRef.totalPagesNumber > 1 : false

            sectionContents:
              GridLayout
              {
                width: parent.width
                columns: 2
                columnSpacing: 5
                rowSpacing: 3

                // row 1
                RobotoText
                {
                    height: gridItemsHeight
                    Layout.fillWidth: true
                    label: qsTr("Shortcuts")
                }
                CustomComboBox
                {
                    id: shortcutList
                    Layout.fillWidth: true
                    height: gridItemsHeight
                    textRole: ""
                    model: widgetRef ? widgetRef.pageLabels : null
                }

                // row 2
                RobotoText
                {
                    height: gridItemsHeight
                    Layout.fillWidth: true
                    label: qsTr("Shortcut name")
                }
                CustomTextEdit
                {
                    id: shortcutEdit
                    Layout.fillWidth: true
                    text: shortcutList.currentText
                    onTextChanged:
                    {
                        var idx = shortcutList.currentIndex
                        if (widgetRef)
                            widgetRef.setShortcutName(shortcutList.currentIndex, text)
                        // combo model has changed. Restore selected index
                        shortcutList.currentIndex = idx
                    }
                }
              }
        }
    }

}
