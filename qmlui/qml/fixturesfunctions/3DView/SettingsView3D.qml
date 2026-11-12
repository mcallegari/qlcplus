/*
  Q Light Controller Plus
  SettingsView3D.qml

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
import QtQuick.Dialogs
import QtQuick.Controls

import org.qlcplus.classes 1.0
import "."

Rectangle
{
    id: settingsRoot
    width: mainView.width / 5
    height: parent.height

    color: UISettings.bgStrong
    border.width: 1
    border.color: UISettings.bgStrong

    property vector3d envSize: contextManager ? contextManager.environmentSize : Qt.vector3d(0, 0, 0)

    property int selFixturesCount: contextManager ? contextManager.selectedFixturesCount : 0
    property int selGenericCount: View3D.genericSelectedCount
    property bool fxPropsVisible: selFixturesCount + selGenericCount ? true : false
    property vector3d currentPosition
    property vector3d currentRotation
    property vector3d currentScale
    property vector3d lastPosition
    property vector3d lastRotation
    property vector3d lastScale
    property bool isUpdating: false

    onSelGenericCountChanged:
    {
        if (selGenericCount === 0)
            itemColorTool.visible = false
    }

    // refresh on every selection change of either kind, even when the
    // count stays the same (e.g. switching from one fixture to another)
    Connections
    {
        target: contextManager
        function onSelectedFixturesChanged() { settingsRoot.refreshTransformValues() }
    }
    Connections
    {
        target: View3D
        function onGenericSelectedCountChanged() { settingsRoot.refreshTransformValues() }
    }

    /** Show the values of what is selected in the 3D view, considering both
     *  fixtures and generic items: absolute values when a single element
     *  is selected, relative values (starting from zero) otherwise */
    function refreshTransformValues()
    {
        var pos = Qt.vector3d(0, 0, 0)
        var rot = Qt.vector3d(0, 0, 0)
        var scl = Qt.vector3d(100.0, 100.0, 100.0)
        // read the counts from the backends, as the bound properties
        // might not be updated yet when this is invoked
        var fxCount = contextManager ? contextManager.selectedFixturesCount : 0
        var genCount = View3D.genericSelectedCount

        if (fxCount === 1 && genCount === 0)
        {
            pos = contextManager.fixturesPosition
            rot = contextManager.fixturesRotation
        }
        else if (fxCount === 0 && genCount === 1)
        {
            pos = View3D.genericItemsPosition
            rot = View3D.genericItemsRotation
            scl = View3D.genericItemsScale
        }

        isUpdating = true
        lastPosition = pos
        lastRotation = rot
        lastScale = scl
        currentPosition = pos
        currentRotation = rot
        currentScale = scl
        isUpdating = false
    }

    onCurrentScaleChanged: console.log("Current scale " + currentScale)

    ColorTool
    {
        id: itemColorTool
        parent: mainView
        x: rightPanel.x - settingsRoot.width - width
        y: UISettings.bigItemHeight
        visible: false
        showPalette: false

        onToolColorChanged:
            function(r, g, b, w, a, uv)
            {
                View3D.genericItemsColor = Qt.rgba(r, g, b, 1.0)
            }
        onClose: visible = false
    }

    function toggleItemColorTool()
    {
        if (itemColorTool.visible)
        {
            itemColorTool.visible = false
            return
        }

        itemColorTool.visible = true
        // ColorTool picks fixture colors when shown. Override with the item one
        var col = View3D.genericItemsColor
        itemColorTool.currentRGB = col.valid ? col : Qt.rgba(1, 1, 1, 1)
    }

    PopupRenameItems
    {
        id: itemRenamePopup
        title: qsTr("Rename item")
        onAccepted: View3D.genericItemsName = editText
    }

    function refreshPositionValues(generic)
    {
        isUpdating = true
        currentPosition = generic ? View3D.genericItemsPosition : contextManager.fixturesPosition
        isUpdating = false
    }

    // last clicked row of the generic items list, used for Shift range selection
    property int lastItemIndex: -1

    // The 3D view owns the generic items selection and updates the list
    // "isSelected" role, so both views stay in sync whichever one is clicked
    function selectGenericItem(index, selected, modifiers)
    {
        if (modifiers & Qt.ShiftModifier && lastItemIndex >= 0)
        {
            var from = Math.min(index, lastItemIndex)
            var to = Math.max(index, lastItemIndex)
            for (var i = from; i <= to; i++)
                View3D.setItemSelectionByIndex(i, true, Qt.ControlModifier)
            return
        }

        if (modifiers & Qt.ControlModifier)
            View3D.setItemSelectionByIndex(index, !selected, Qt.ControlModifier)
        else
            View3D.setItemSelectionByIndex(index, true, 0)

        lastItemIndex = index
    }

    // catch wheel events over the whole panel so they don't
    // fall through to the 3D view behind and zoom it in/out.
    // This sits below the Flickable, which consumes its own wheel
    // events for scrolling; anything not consumed there is stopped here.
    MouseArea
    {
        anchors.fill: parent
        onWheel: (wheel) => { wheel.accepted = true }
    }

    Flickable
    {
        x: 5
        width: settingsRoot.width - 10
        height: parent.height
        contentHeight: settingsColumn.height
        boundsBehavior: Flickable.StopAtBounds

        Column
        {
            id: settingsColumn
            width: parent.width - (sbar.visible ? sbar.width : 0)
            spacing: 2

            SectionBox
            {
                width: parent.width
                sectionLabel: qsTr("Environment")
                sectionContents:
                    GridLayout
                    {
                        width: parent.width
                        columns: 2
                        columnSpacing: 5
                        rowSpacing: 2

                        // row 1
                        RobotoText { height: UISettings.listItemHeight; label: qsTr("Type") }
                        CustomComboBox
                        {
                            Layout.fillWidth: true
                            height: UISettings.listItemHeight

                            textRole: ""
                            model: View3D.stagesList
                            currValue: View3D.stageIndex
                            onValueChanged: View3D.stageIndex = value
                        }

                        // row 2
                        RobotoText { height: UISettings.listItemHeight; label: qsTr("Width") }
                        CustomSpinBox
                        {
                            height: UISettings.listItemHeight
                            Layout.fillWidth: true
                            from: 1
                            to: 50
                            suffix: View2D && View2D.gridUnits === MonitorProperties.Feet ? "ft" : "m"
                            value: envSize.x
                            onValueModified:
                            {
                                if (settingsRoot.visible && contextManager)
                                    contextManager.environmentSize = Qt.vector3d(value, envSize.y, envSize.z)
                            }
                        }

                        // row 3
                        RobotoText { height: UISettings.listItemHeight; label: qsTr("Height") }
                        CustomSpinBox
                        {
                            height: UISettings.listItemHeight
                            Layout.fillWidth: true
                            from: 1
                            to: 50
                            suffix: View2D && View2D.gridUnits === MonitorProperties.Feet ? "ft" : "m"
                            value: envSize.y
                            onValueModified:
                            {
                                if (settingsRoot.visible && contextManager)
                                    contextManager.environmentSize = Qt.vector3d(envSize.x, value, envSize.z)
                            }
                        }

                        // row 4
                        RobotoText { height: UISettings.listItemHeight; label: qsTr("Depth") }
                        CustomSpinBox
                        {
                            height: UISettings.listItemHeight
                            Layout.fillWidth: true
                            from: 1
                            to: 100
                            suffix: View2D && View2D.gridUnits === MonitorProperties.Feet ? "ft" : "m"
                            value: envSize.z
                            onValueModified:
                            {
                                if (settingsRoot.visible && contextManager)
                                    contextManager.environmentSize = Qt.vector3d(envSize.x, envSize.y, value)
                            }
                        }
                    } // GridLayout
            } // Section box - Environment

            SectionBox
            {
                width: parent.width
                isExpanded: false
                sectionLabel: qsTr("Rendering")
                sectionContents:
                    GridLayout
                    {
                        width: parent.width
                        columns: 2
                        columnSpacing: 5
                        rowSpacing: 2

                        Component.onCompleted:
                        {
                            ambIntSpin.value = View3D.ambientIntensity * 100
                            smokeSpin.value = View3D.smokeAmount * 100
                        }

                        // row 1
                        RobotoText { height: UISettings.listItemHeight; label: qsTr("Quality") }
                        CustomComboBox
                        {
                            Layout.fillWidth: true
                            height: UISettings.listItemHeight
                            model: [
                                { mLabel: qsTr("Low"), mValue: MainView3D.LowQuality },
                                { mLabel: qsTr("Medium"), mValue: MainView3D.MediumQuality },
                                { mLabel: qsTr("High"), mValue: MainView3D.HighQuality },
                                { mLabel: qsTr("Ultra"), mValue: MainView3D.UltraQuality }
                            ]
                            currentIndex: View3D.renderQuality
                            onCurrentIndexChanged: View3D.renderQuality = currentIndex
                        }

                        // row 2
                        RobotoText { height: UISettings.listItemHeight; label: qsTr("Ambient light") }
                        CustomSpinBox
                        {
                            id: ambIntSpin
                            Layout.fillWidth: true
                            height: UISettings.listItemHeight
                            from: 0
                            to: 100
                            suffix: "%"
                            onValueModified: View3D.ambientIntensity = value / 100
                        }

                        // row 3
                        RobotoText { height: UISettings.listItemHeight; label: qsTr("Smoke amount") }
                        CustomSpinBox
                        {
                            id: smokeSpin
                            Layout.fillWidth: true
                            height: UISettings.listItemHeight
                            from: 0
                            to: 100
                            suffix: "%"
                            onValueModified: View3D.smokeAmount = value / 100
                        }

                        // row 4
                        RobotoText { height: UISettings.listItemHeight; label: qsTr("Show FPS") }
                        CustomCheckBox
                        {
                            implicitHeight: UISettings.listItemHeight
                            implicitWidth: implicitHeight
                            checked: View3D ? View3D.frameCountEnabled : false
                            onToggled: View3D.frameCountEnabled = checked
                        }

                        // row 5
                        RobotoText { height: UISettings.listItemHeight; label: qsTr("Show fixture groups") }
                        CustomCheckBox
                        {
                            implicitHeight: UISettings.listItemHeight
                            implicitWidth: implicitHeight
                            checked: contextManager ? contextManager.showFixtureGroups : false
                            onToggled: contextManager.showFixtureGroups = checked
                        }

                    } // GridLayout
            } // SectionBox - Rendering

            SectionBox
            {
                width: parent.width
                visible: fxPropsVisible
                sectionLabel: qsTr("Position")
                sectionContents:
                    GridLayout
                    {
                        width: parent.width
                        columns: 2
                        columnSpacing: 5
                        rowSpacing: 2

                        function updatePosition(x, y, z)
                        {
                            if (isUpdating)
                                return;

                            var newPos = Qt.vector3d(x - lastPosition.x, y - lastPosition.y, z - lastPosition.z)
                            if (selFixturesCount > 0)
                                contextManager.fixturesPosition = newPos
                            if (selGenericCount > 0)
                                View3D.genericItemsPosition = newPos
                            lastPosition = Qt.vector3d(x, y, z)
                            // keep in sync with the spin boxes, so a later refresh
                            // to the same value is not ignored
                            isUpdating = true
                            currentPosition = lastPosition
                            isUpdating = false
                        }

                        // row 1
                        RobotoText
                        {
                            height: UISettings.listItemHeight
                            width: UISettings.bigItemHeight
                            textHAlign: Qt.AlignRight
                            label: "X"
                        }
                        CustomSpinBox
                        {
                            id: xPosSpin
                            height: UISettings.listItemHeight
                            Layout.fillWidth: true
                            from: -100000
                            to: 100000
                            stepSize: 10
                            suffix: "mm"
                            value: currentPosition.x
                            onValueModified: updatePosition(value, yPosSpin.value, zPosSpin.value)
                        }

                        // row 2
                        RobotoText
                        {
                            height: UISettings.listItemHeight
                            width: UISettings.bigItemHeight
                            textHAlign: Qt.AlignRight
                            label: "Y"
                        }
                        CustomSpinBox
                        {
                            id: yPosSpin
                            height: UISettings.listItemHeight
                            Layout.fillWidth: true
                            from: -100000
                            to: 100000
                            stepSize: 10
                            suffix: "mm"
                            value: currentPosition.y
                            onValueModified: updatePosition(xPosSpin.value, value, zPosSpin.value)
                        }

                        // row 3
                        RobotoText
                        {
                            height: UISettings.listItemHeight
                            width: UISettings.bigItemHeight
                            textHAlign: Qt.AlignRight
                            label: "Z"
                        }
                        CustomSpinBox
                        {
                            id: zPosSpin
                            height: UISettings.listItemHeight
                            Layout.fillWidth: true
                            from: -100000
                            to: 100000
                            stepSize: 10
                            suffix: "mm"
                            value: currentPosition.z
                            onValueModified: updatePosition(xPosSpin.value, yPosSpin.value, value)
                        }
                    } // GridLayout
            } // SectionBox - Position

            SectionBox
            {
                width: parent.width
                isExpanded: false
                visible: fxPropsVisible
                sectionLabel: qsTr("Rotation")
                sectionContents:
                    GridLayout
                    {
                        width: parent.width
                        columns: 2
                        columnSpacing: 5
                        rowSpacing: 2

                        function updateRotation(x, y, z)
                        {
                            if (isUpdating)
                                return;

                            var newRot = Qt.vector3d(x - lastRotation.x, y - lastRotation.y, z - lastRotation.z)
                            if (selFixturesCount > 0)
                                contextManager.fixturesRotation = newRot
                            if (selGenericCount > 0)
                                View3D.genericItemsRotation = newRot
                            lastRotation = Qt.vector3d(x, y, z)
                            isUpdating = true
                            currentRotation = lastRotation
                            isUpdating = false
                        }

                        // row 1
                        RobotoText
                        {
                            height: UISettings.listItemHeight
                            width: UISettings.bigItemHeight
                            textHAlign: Qt.AlignRight
                            label: "X"
                        }
                        CustomSpinBox
                        {
                            id: xRotSpin
                            height: UISettings.listItemHeight
                            Layout.fillWidth: true
                            from: -359
                            to: 359
                            suffix: "°"
                            value: currentRotation.x
                            onValueModified: updateRotation(value, yRotSpin.value, zRotSpin.value)
                        }

                        // row 2
                        RobotoText
                        {
                            height: UISettings.listItemHeight
                            width: UISettings.bigItemHeight
                            textHAlign: Qt.AlignRight
                            label: "Y"
                        }
                        CustomSpinBox
                        {
                            id: yRotSpin
                            height: UISettings.listItemHeight
                            Layout.fillWidth: true
                            from: -359
                            to: 359
                            suffix: "°"
                            value: currentRotation.y
                            onValueModified: updateRotation(xRotSpin.value, value, zRotSpin.value)
                        }

                        // row 3
                        RobotoText
                        {
                            height: UISettings.listItemHeight
                            width: UISettings.bigItemHeight
                            textHAlign: Qt.AlignRight
                            label: "Z"
                        }
                        CustomSpinBox
                        {
                            id: zRotSpin
                            height: UISettings.listItemHeight
                            Layout.fillWidth: true
                            from: -359
                            to: 359
                            suffix: "°"
                            value: currentRotation.z
                            onValueModified: updateRotation(xRotSpin.value, yRotSpin.value, value)
                        }
                    } // GridLayout
            } // SectionBox - Rotation

            SectionBox
            {
                width: parent.width
                isExpanded: false
                visible: selGenericCount ? true : false
                sectionLabel: qsTr("Scale")
                sectionContents:
                    GridLayout
                    {
                        width: parent.width
                        columns: 3
                        columnSpacing: 5
                        rowSpacing: 2

                        function updateScale(x, y, z)
                        {
                            if (isUpdating)
                                return;

                            if (selGenericCount == 1)
                            {
                                View3D.genericItemsScale = Qt.vector3d(x, y, z)
                            }
                            else
                            {
                                var newScale = Qt.vector3d(x - lastScale.x, y - lastScale.y, z - lastScale.z)
                                View3D.genericItemsScale = newScale
                                lastScale = Qt.vector3d(x, y, z)
                            }
                            isUpdating = true
                            currentScale = scaleLocked.checked ? Qt.vector3d(x, x, x) : Qt.vector3d(x, y, z)
                            isUpdating = false
                        }

                        // row 1
                        RobotoText
                        {
                            height: UISettings.listItemHeight
                            width: UISettings.bigItemHeight
                            textHAlign: Qt.AlignRight
                            label: "X"
                        }
                        CustomSpinBox
                        {
                            id: xScaleSpin
                            height: UISettings.listItemHeight
                            Layout.fillWidth: true
                            from: 1
                            to: 10000
                            suffix: "%"
                            value: currentScale.x
                            onValueModified:
                            {
                                if (scaleLocked.checked)
                                    updateScale(value, value, value)
                                else
                                    updateScale(value, yScaleSpin.value, zScaleSpin.value)
                            }
                        }

                        Rectangle
                        {
                            Layout.rowSpan: 3
                            Layout.fillHeight: true
                            color: "transparent"
                            width: UISettings.iconSizeMedium
                            clip: true

                            Rectangle
                            {
                                color: "transparent"
                                x: -width / 2
                                y: UISettings.listItemHeight / 2
                                width: parent.width
                                height: parent.height - UISettings.listItemHeight
                                border.width: 1
                                border.color: "white"
                            }

                            IconButton
                            {
                                id: scaleLocked
                                anchors.centerIn: parent
                                width: UISettings.iconSizeMedium
                                height: width
                                imgSource: "qrc:/lock.svg"
                                checkable: true
                                checked: View3D ? View3D.scaleLocked : true
                                onToggled: View3D.scaleLocked = checked

                                // clicking a checkable Button assigns "checked" and
                                // therefore breaks the binding above, so the value
                                // has to be restored explicitly when it changes on
                                // the C++ side (e.g. on project load)
                                Connections
                                {
                                    target: View3D
                                    function onScaleLockedChanged()
                                    {
                                        scaleLocked.checked = View3D.scaleLocked
                                    }
                                }
                            }
                        }

                        // row 2
                        RobotoText
                        {
                            height: UISettings.listItemHeight
                            width: UISettings.bigItemHeight
                            textHAlign: Qt.AlignRight
                            label: "Y"
                        }
                        CustomSpinBox
                        {
                            id: yScaleSpin
                            height: UISettings.listItemHeight
                            Layout.fillWidth: true
                            from: 1
                            to: 10000
                            suffix: "%"
                            value: currentScale.y
                            onValueModified:
                            {
                                if (scaleLocked.checked)
                                    updateScale(value, value, value)
                                else
                                    updateScale(xScaleSpin.value, value, zScaleSpin.value)
                            }
                        }

                        // row 3
                        RobotoText
                        {
                            height: UISettings.listItemHeight
                            width: UISettings.bigItemHeight
                            textHAlign: Qt.AlignRight
                            label: "Z"
                        }
                        CustomSpinBox
                        {
                            id: zScaleSpin
                            height: UISettings.listItemHeight
                            Layout.fillWidth: true
                            from: 1
                            to: 10000
                            suffix: "%"
                            value: currentScale.z
                            onValueModified:
                            {
                                if (scaleLocked.checked)
                                    updateScale(value, value, value)
                                else
                                    updateScale(xScaleSpin.value, yScaleSpin.value, value)
                            }
                        }
                    } // GridLayout
            } // SectionBox - Scale

            SectionBox
            {
                width: parent.width
                isExpanded: false
                sectionLabel: qsTr("Custom items")

                FileDialog
                {
                    id: meshDialog
                    visible: false
                    title: qsTr("Select a mesh file")
                    currentFolder: View3D.meshDirectory
                    nameFilters: [ qsTr("3D files") + " (*.obj *.dae *.3ds *.py *.stl *.blend)", qsTr("All files") + " (*)" ]

                    onAccepted: View3D.createGenericItem(selectedFile, -1)
                }

                sectionContents:
                    ColumnLayout
                    {
                        width: parent.width

                        Rectangle
                        {
                            width: parent.width
                            height: UISettings.iconSizeMedium

                            gradient: Gradient
                            {
                                GradientStop { position: 0; color: UISettings.toolbarStartSub }
                                GradientStop { position: 1; color: UISettings.toolbarEnd }
                            }

                            RowLayout
                            {
                                width: parent.width
                                height: UISettings.iconSizeMedium

                                IconButton
                                {
                                    height: UISettings.iconSizeMedium
                                    width: height
                                    faSource: FontAwesome.fa_plus
                                    faColor: "limegreen"
                                    tooltip: qsTr("Add a new item to the scene")
                                    onClicked: meshDialog.open()
                                }
                                IconButton
                                {
                                    enabled: selGenericCount
                                    height: UISettings.iconSizeMedium
                                    width: height
                                    faSource: FontAwesome.fa_minus
                                    faColor: "crimson"
                                    tooltip: qsTr("Remove the selected items")
                                    onClicked: View3D.removeSelectedGenericItems()
                                }
                                IconButton
                                {
                                    enabled: selGenericCount
                                    height: UISettings.iconSizeMedium
                                    width: height
                                    faSource: FontAwesome.fa_compress
                                    faColor: UISettings.fgMain
                                    tooltip: qsTr("Normalize the selected items")
                                    onClicked: View3D.normalizeSelectedGenericItems()
                                }
                                IconButton
                                {
                                    enabled: selGenericCount
                                    height: UISettings.iconSizeMedium
                                    width: height
                                    checked: View3D.genericSelectedLocked
                                    faSource: checked ? FontAwesome.fa_lock : FontAwesome.fa_lock_open
                                    faColor: checked ? "#00FF00" : UISettings.fgMain
                                    tooltip: qsTr("Lock/Unlock the selected items")
                                    onClicked: View3D.toggleGenericItemsLock()
                                }
                                IconButton
                                {
                                    // a name can be set only on a single item
                                    enabled: selGenericCount === 1
                                    height: UISettings.iconSizeMedium
                                    width: height
                                    imgSource: "qrc:/rename.svg"
                                    tooltip: qsTr("Rename the selected item")
                                    onClicked:
                                    {
                                        itemRenamePopup.editText = View3D.genericItemsName
                                        itemRenamePopup.open()
                                    }
                                }
                                IconButton
                                {
                                    enabled: selGenericCount
                                    height: UISettings.iconSizeMedium
                                    width: height
                                    checked: itemColorTool.visible
                                    faSource: FontAwesome.fa_palette
                                    faColor: UISettings.fgMain
                                    tooltip: qsTr("Set the base color of the selected items")
                                    onClicked: settingsRoot.toggleItemColorTool()
                                }
                                IconButton
                                {
                                    enabled: selGenericCount
                                    height: UISettings.iconSizeMedium
                                    width: height
                                    faSource: FontAwesome.fa_eraser
                                    faColor: UISettings.fgMain
                                    tooltip: qsTr("Restore the original colors of the selected items")
                                    onClicked: View3D.resetGenericItemsColor()
                                }
                                Rectangle
                                {
                                    Layout.fillWidth: true
                                    height: UISettings.iconSizeMedium
                                    color: "transparent"
                                }
                            }
                        }

                        ListView
                        {
                            id: itemsList
                            width: parent.width
                            // show all the items and let the Flickable scrollbar handle
                            // scrolling, so there are not two nested scrollbars
                            height: count * UISettings.listItemHeight
                            interactive: false
                            model: View3D.genericItemsList
                            clip: true

                            delegate:
                                Rectangle
                                {
                                    // account for the Flickable scrollbar so the lock
                                    // icon on the right edge stays visible
                                    width: itemsList.width - (sbar.visible ? sbar.width : 0)
                                    height: UISettings.listItemHeight
                                    color: isSelected ? UISettings.highlight : "transparent"

                                    IconTextEntry
                                    {
                                        width: parent.width
                                        height: UISettings.listItemHeight

                                        tLabel: name
                                        faSource: FontAwesome.fa_cube
                                        faColor: UISettings.fgMain

                                        MouseArea
                                        {
                                            anchors.fill: parent
                                            onClicked: (mouse) =>
                                            {
                                                settingsRoot.selectGenericItem(index, isSelected, mouse.modifiers)
                                            }
                                        }
                                    }

                                    Row
                                    {
                                        anchors.right: parent.right
                                        anchors.rightMargin: 6
                                        anchors.verticalCenter: parent.verticalCenter
                                        spacing: 4

                                        Text
                                        {
                                            visible: isLocked
                                            anchors.verticalCenter: parent.verticalCenter
                                            color: "#00FF00"
                                            font.family: UISettings.fontAwesomeFontName
                                            font.pixelSize: UISettings.listItemHeight * 0.6
                                            text: FontAwesome.fa_lock
                                        }

                                        // custom base color, if set
                                        Rectangle
                                        {
                                            visible: model.itemColor.valid
                                            anchors.verticalCenter: parent.verticalCenter
                                            width: UISettings.listItemHeight * 0.6
                                            height: width
                                            radius: 2
                                            color: model.itemColor.valid ? model.itemColor : "transparent"
                                            border.width: 1
                                            border.color: UISettings.borderColorDark
                                        }
                                    }
                                }
                        }
                    }
            }
        } // Column
        ScrollBar.vertical: CustomScrollBar { id: sbar }
    } // Flickable
}
