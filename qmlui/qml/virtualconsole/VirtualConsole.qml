/*
  Q Light Controller Plus
  VirtualConsole.qml

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
    id: vcContainer
    anchors.fill: parent
    color: "transparent"
    objectName: "virtualConsole"

    property string contextName: "VC"
    property int selectedPage: virtualConsole.selectedPage
    property bool docLoaded: qlcplus.docLoaded

    onDocLoadedChanged:
    {
        // force a reload of the selected page
        virtualConsole.selectedPage = 0
        pageLoader.active = false
        pageLoader.active = true
    }

    onSelectedPageChanged:
    {
        pageLoader.source = ""
        if (selectedPage < 0)
            return;
        pageLoader.source = "qrc:/VCPageArea.qml"
    }

    function enableContext(ctx, setChecked)
    {
        console.log("VC enable context " + ctx)
        for (var i = 0; i < pagesRepeater.count; i++)
        {
            console.log("Item " + i + " name: " + pagesRepeater.itemAt(i).contextName)
            if (pagesRepeater.itemAt(i).contextName === ctx)
                pagesRepeater.itemAt(i).visible = true
        }
    }

    function requestMatrixPopup(target, mparent, type, pos)
    {
        addMatrixPopup.targetFrame = target
        addMatrixPopup.matrixParent = mparent
        addMatrixPopup.wType = type
        addMatrixPopup.wPos = pos
        addMatrixPopup.open()
    }

    VCRightPanel
    {
        id: rightSidePanel
        visible: qlcplus.accessMask & App.AC_VCEditing
        x: parent.width - width
        z: 5
        height: parent.height
    }

    Rectangle
    {
        id: centerView
        width: parent.width - ((qlcplus.accessMask & App.AC_VCEditing) ? rightSidePanel.width : 0)
        height: parent.height
        color: "transparent"
        clip: true

        Rectangle
        {
            id: vcToolbar
            width: parent.width
            height: UISettings.iconSizeMedium
            z: 10
            gradient: Gradient
            {
                id: vcTbGradient
                GradientStop { position: 0; color: UISettings.toolbarStartSub }
                GradientStop { position: 1; color: UISettings.toolbarEnd }
            }

            RowLayout
            {
                id: rowLayout1
                anchors.fill: parent
                spacing: 5
                ButtonGroup { id: vcToolbarGroup }

                Repeater
                {
                    id: pagesRepeater
                    model: virtualConsole.pagesCount
                    delegate:
                        MenuBarEntry
                        {
                            property VCWidget wObj: virtualConsole.page(index)
                            property string contextName: "PAGE-" + index

                            entryText: wObj ? wObj.caption : qsTr("Page " + index)
                            mFontSize: UISettings.textSizeDefault
                            //editable: true
                            checked: index === virtualConsole.selectedPage ? true : false
                            checkedColor: UISettings.toolbarSelectionSub
                            bgGradient: vcTbGradient
                            ButtonGroup.group: vcToolbarGroup

                            onCheckedChanged:
                            {
                                if (wObj && checked == true)
                                {
                                    if (wObj.requirePIN())
                                        pinRequestPopup.open()
                                    else
                                        virtualConsole.selectedPage = index
                                }
                            }
                            onRightClicked:
                            {
                                visible = false
                                contextManager.detachContext("PAGE-" + index)
                            }
                            onTextChanged:
                            {
                                if (wObj)
                                    wObj.caption = text
                            }

                            PopupPINRequest
                            {
                                id: pinRequestPopup
                                onAccepted:
                                {
                                    if (virtualConsole.validatePagePIN(index, currentPIN, sessionValidate) === true)
                                    {
                                        virtualConsole.selectedPage = index
                                    }
                                    else
                                    {
                                        pinErrorPopup.open()
                                        // invalidate page selection so nothing
                                        // is displayed on screen
                                        virtualConsole.selectedPage = -1
                                    }
                                }
                            }

                            CustomPopupDialog
                            {
                                id: pinErrorPopup
                                title: qsTr("Error")
                                message: qsTr("Invalid PIN entered")
                            }
                        }
                }

                // filler
                Rectangle { Layout.fillWidth: true }

                IconButton
                {
                    width: vcToolbar.height
                    height: width
                    imgSource: "qrc:/grid.svg"
                    tooltip: qsTr("Enable/Disable widgets snapping")
                    checkable: true
                    checked: virtualConsole.snapping
                    onToggled: virtualConsole.snapping = checked
                }

                ZoomItem
                {
                    width: UISettings.iconSizeMedium * 2
                    implicitHeight: vcToolbar.height - 2
                    fontColor: UISettings.bgStrong
                    onZoomOutClicked: { virtualConsole.setPageScale(-0.1) }
                    onZoomInClicked: { virtualConsole.setPageScale(0.1) }
                }
            }

        }
        Loader
        {
            id: pageLoader
            z: 0
            anchors.top: vcToolbar.bottom
            width: centerView.width
            height: parent.height - vcToolbar.height
            source: "qrc:/VCPageArea.qml"

            onLoaded:
            {
                // set the page
                pageLoader.item.page = selectedPage
            }
        }
    }


    CustomPopupDialog
    {
        id: addMatrixPopup
        z: 99

        property var matrixParent: null
        property VCFrame targetFrame: null
        property string wType: ""
        property point wPos

        title: qsTr("Widget matrix setup")

        contentItem:
            GridLayout
            {
                width: parent.width
                height: UISettings.iconSizeDefault * rows
                columns: 4
                rowSpacing: 5
                columnSpacing: 5

                // row 1
                RobotoText  { label: qsTr("Columns") }
                CustomSpinBox
                {
                    id: mxColumns
                    from: 1
                    to: 99
                    value: 5
                }

                RobotoText  { label: qsTr("Rows") }
                CustomSpinBox
                {
                    id: mxRows
                    from: 1
                    to: 99
                }

                // row 2
                RobotoText  { label: qsTr("Width") }
                CustomSpinBox
                {
                    id: wWidth
                    from: 1
                    to: 999
                    suffix: "px"
                    value: addMatrixPopup.wType === "buttonmatrix" ? screenPixelDensity * 17 : screenPixelDensity * 10
                }

                RobotoText  { label: qsTr("Height") }
                CustomSpinBox
                {
                    id: wHeight
                    from: 1
                    to: 999
                    suffix: "px"
                    value: addMatrixPopup.wType === "buttonmatrix" ? screenPixelDensity * 17 : screenPixelDensity * 35
                }

                // row 3
                Row
                {
                    Layout.columnSpan: 4
                    Layout.fillWidth: true
                    spacing: 10

                    RobotoText  { label: qsTr("Frame type") }
                    CustomCheckBox
                    {
                        id: normalFrameCheck
                        height: UISettings.iconSizeMedium
                        width: height
                        checked: true
                        autoExclusive: true
                    }
                    RobotoText  { label: qsTr("Normal") }
                    CustomCheckBox
                    {
                        id: soloFrameCheck
                        height: UISettings.iconSizeMedium
                        width: height
                        autoExclusive: true
                    }
                    RobotoText  { label: qsTr("Solo") }
                }
            }

        onAccepted: targetFrame.addWidgetMatrix(matrixParent, wType, wPos, Qt.size(mxColumns.value, mxRows.value),
                                                Qt.size(wWidth.value, wHeight.value), soloFrameCheck.checked)
    }
}
