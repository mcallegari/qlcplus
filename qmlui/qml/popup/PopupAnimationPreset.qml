/*
  Q Light Controller Plus
  PopupAnimationPreset.qml

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
import QtQuick.Controls.Basic

import org.qlcplus.classes 1.0
import "."

CustomPopupDialog
{
    id: popupRoot
    width: mainView ? mainView.width / 2.5 : 500
    height: mainView ? mainView.height / 2 : 400
    title: qsTr("Add algorithm preset")

    property VCAnimation widgetRef: null

    // map of property name -> edited value (as string)
    property var editedValues: ({})
    // list of property metadata maps for the selected algorithm
    property var propsList: []

    function reloadProperties()
    {
        var values = {}
        var list = []

        if (widgetRef && algoCombo.currentText !== "")
        {
            list = widgetRef.algorithmProperties(algoCombo.currentText)
            for (var i = 0; i < list.length; i++)
                values[list[i].name] = list[i].value
        }

        editedValues = values
        propsList = list
    }

    onAccepted:
    {
        if (widgetRef && algoCombo.currentText !== "")
            widgetRef.addAlgorithmPreset(algoCombo.currentText, editedValues)
    }

    Component.onCompleted: reloadProperties()

    contentItem:
        ColumnLayout
        {
            spacing: 5

            GridLayout
            {
                Layout.fillWidth: true
                columns: 2
                columnSpacing: 5
                rowSpacing: 4

                RobotoText
                {
                    height: UISettings.listItemHeight
                    label: qsTr("Algorithm")
                }

                CustomComboBox
                {
                    id: algoCombo
                    Layout.fillWidth: true
                    height: UISettings.listItemHeight
                    textRole: ""
                    model: widgetRef ? widgetRef.scriptAlgorithms() : null
                    onCurrentTextChanged: popupRoot.reloadProperties()
                }
            }

            RobotoText
            {
                Layout.fillWidth: true
                height: UISettings.listItemHeight
                visible: popupRoot.propsList.length > 0
                label: qsTr("Parameters")
                fontBold: true
            }

            // Parameters list, scrollable when there are many of them.
            // Fills the remaining popup height so the view scrolls vertically.
            Flickable
            {
                id: paramsFlick
                Layout.fillWidth: true
                Layout.fillHeight: true
                visible: popupRoot.propsList.length > 0
                clip: true
                contentWidth: width
                contentHeight: paramsColumn.height
                boundsBehavior: Flickable.StopAtBounds

                ScrollBar.vertical: CustomScrollBar { id: paramsScroll }

                Column
                {
                    id: paramsColumn
                    // leave room for the scrollbar when it is visible
                    width: paramsFlick.width - (paramsScroll.visible ? paramsScroll.width : 0)
                    spacing: 4

                    Repeater
                    {
                        model: popupRoot.propsList

                        delegate: RowLayout
                        {
                            required property var modelData
                            width: paramsColumn.width
                            spacing: 5

                            RobotoText
                            {
                                Layout.preferredWidth: paramsColumn.width * 0.45
                                height: UISettings.listItemHeight
                                label: modelData.displayName
                            }

                            Loader
                            {
                                id: editorLoader
                                Layout.fillWidth: true

                                sourceComponent:
                                {
                                    switch (modelData.type)
                                    {
                                        case "List": return listEditor
                                        case "Range": return rangeEditor
                                        case "Float": return floatEditor
                                        case "String": return stringEditor
                                    }
                                    return null
                                }

                                onLoaded: item.propData = modelData
                            }
                        }
                    }
                }
            }
        }

    // ******************* property editors *******************
    // each editor receives its property metadata via the 'propData' property,
    // assigned by the Loader once the item is created

    Component
    {
        id: listEditor

        CustomComboBox
        {
            property var propData
            height: UISettings.listItemHeight
            textRole: ""
            model: propData ? propData.listValues : null
            onPropDataChanged:
            {
                if (!propData)
                    return
                var idx = propData.listValues.indexOf(propData.value)
                if (idx >= 0)
                    currentIndex = idx
            }
            onCurrentTextChanged:
            {
                if (propData)
                    popupRoot.editedValues[propData.name] = currentText
            }
        }
    }

    Component
    {
        id: rangeEditor

        CustomSpinBox
        {
            property var propData
            height: UISettings.listItemHeight
            from: propData ? propData.min : 0
            to: propData ? propData.max : 255
            value: (propData && propData.value) ? parseInt(propData.value) : (propData ? propData.min : 0)
            onValueModified:
            {
                if (propData)
                    popupRoot.editedValues[propData.name] = value.toString()
            }
        }
    }

    Component
    {
        id: floatEditor

        CustomDoubleSpinBox
        {
            property var propData
            height: UISettings.listItemHeight
            realFrom: -1000000
            realTo: 1000000
            decimals: 3
            realValue: (propData && propData.value) ? parseFloat(propData.value) : 0
            onRealValueChanged:
            {
                if (propData)
                    popupRoot.editedValues[propData.name] = realValue.toString()
            }
        }
    }

    Component
    {
        id: stringEditor

        CustomTextEdit
        {
            property var propData
            height: UISettings.listItemHeight
            text: (propData && propData.value) ? propData.value : ""
            onTextChanged:
            {
                if (propData)
                    popupRoot.editedValues[propData.name] = text
            }
        }
    }
}
