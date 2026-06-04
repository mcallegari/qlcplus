/*
  Q Light Controller Plus
  SettingsView3DRhi.qml
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Dialogs
import org.qlcplus.classes 1.0
import ".."

Rectangle {
    id: settingsRoot
    width: mainView.width / 5
    height: parent.height

    color: UISettings.bgStrong
    border.width: 1
    border.color: UISettings.bgStrong

    property vector3d envSize: contextManager ? contextManager.environmentSize : Qt.vector3d(5, 3, 5)
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

    function fixtureRotationToUi(rot)
    {
        return rot
    }

    function fixtureRotationFromUi(rot)
    {
        return rot
    }

    onSelFixturesCountChanged: {
        isUpdating = true
        var pos = contextManager.fixturesPosition
        var rot = fixtureRotationToUi(contextManager.fixturesRotation)
        if (selFixturesCount + selGenericCount > 1)
        {
            lastPosition = Qt.vector3d(0, 0, 0)
            lastRotation = Qt.vector3d(0, 0, 0)
            pos = lastPosition
            rot = lastRotation
        }

        currentPosition = pos
        currentRotation = rot
        isUpdating = false
    }

    onSelGenericCountChanged: {
        isUpdating = true
        var pos = View3D.genericItemsPosition
        var rot = View3D.genericItemsRotation
        var scl = View3D.genericItemsScale
        if (selFixturesCount + selGenericCount > 1)
        {
            lastPosition = Qt.vector3d(0, 0, 0)
            lastRotation = Qt.vector3d(0, 0, 0)
            lastScale = Qt.vector3d(100.0, 100.0, 100.0)
            pos = lastPosition
            rot = lastRotation
            scl = lastScale
        }

        currentPosition = pos
        currentRotation = rot
        currentScale = scl
        isUpdating = false
    }

    Connections {
        target: contextManager
        enabled: settingsRoot.visible && !!contextManager

        function onFixturesPositionChanged()
        {
            if (isUpdating || selFixturesCount !== 1 || selGenericCount !== 0)
                return
            isUpdating = true
            currentPosition = contextManager.fixturesPosition
            isUpdating = false
        }

        function onFixturesRotationChanged()
        {
            if (isUpdating || selFixturesCount !== 1 || selGenericCount !== 0)
                return
            isUpdating = true
            currentRotation = fixtureRotationToUi(contextManager.fixturesRotation)
            isUpdating = false
        }
    }

    Connections {
        target: View3D
        enabled: settingsRoot.visible && !!View3D

        function onGenericItemsPositionChanged()
        {
            if (isUpdating || selFixturesCount !== 0 || selGenericCount !== 1)
                return
            isUpdating = true
            currentPosition = View3D.genericItemsPosition
            isUpdating = false
        }

        function onGenericItemsRotationChanged()
        {
            if (isUpdating || selFixturesCount !== 0 || selGenericCount !== 1)
                return
            isUpdating = true
            currentRotation = View3D.genericItemsRotation
            isUpdating = false
        }

        function onGenericItemsScaleChanged()
        {
            if (isUpdating || selFixturesCount !== 0 || selGenericCount !== 1)
                return
            isUpdating = true
            currentScale = View3D.genericItemsScale
            isUpdating = false
        }
    }

    ModelSelector {
        id: giSelector
    }

    Flickable {
        x: 5
        width: settingsRoot.width - 10
        height: parent.height
        contentHeight: settingsColumn.height
        boundsBehavior: Flickable.StopAtBounds

        Column {
            id: settingsColumn
            width: parent.width - (sbar.visible ? sbar.width : 0)
            spacing: 2

            SectionBox {
                width: parent.width
                sectionLabel: qsTr("Environment")
                sectionContents: GridLayout {
                    width: parent.width
                    columns: 2
                    columnSpacing: 5
                    rowSpacing: 2

                    RobotoText { height: UISettings.listItemHeight; label: qsTr("Type") }
                    CustomComboBox {
                        Layout.fillWidth: true
                        height: UISettings.listItemHeight
                        textRole: ""
                        model: View3D.stagesList
                        currValue: View3D.stageIndex
                        onValueChanged: function(newValue) { View3D.stageIndex = newValue }
                    }

                    RobotoText { height: UISettings.listItemHeight; label: qsTr("Width") }
                    CustomSpinBox {
                        height: UISettings.listItemHeight
                        Layout.fillWidth: true
                        from: 1
                        to: 50
                        suffix: View2D && View2D.gridUnits === MonitorProperties.Feet ? "ft" : "m"
                        value: envSize.x
                        onValueModified: if (contextManager) contextManager.environmentSize = Qt.vector3d(value, envSize.y, envSize.z)
                    }

                    RobotoText { height: UISettings.listItemHeight; label: qsTr("Height") }
                    CustomSpinBox {
                        height: UISettings.listItemHeight
                        Layout.fillWidth: true
                        from: 1
                        to: 50
                        suffix: View2D && View2D.gridUnits === MonitorProperties.Feet ? "ft" : "m"
                        value: envSize.y
                        onValueModified: if (contextManager) contextManager.environmentSize = Qt.vector3d(envSize.x, value, envSize.z)
                    }

                    RobotoText { height: UISettings.listItemHeight; label: qsTr("Depth") }
                    CustomSpinBox {
                        height: UISettings.listItemHeight
                        Layout.fillWidth: true
                        from: 1
                        to: 100
                        suffix: View2D && View2D.gridUnits === MonitorProperties.Feet ? "ft" : "m"
                        value: envSize.z
                        onValueModified: if (contextManager) contextManager.environmentSize = Qt.vector3d(envSize.x, envSize.y, value)
                    }
                }
            }

            SectionBox {
                width: parent.width
                sectionLabel: qsTr("Rendering")
                sectionContents: GridLayout {
                    width: parent.width
                    columns: 2
                    columnSpacing: 5
                    rowSpacing: 2

                    RobotoText { height: UISettings.listItemHeight; label: qsTr("Ambient light") }
                    CustomSpinBox {
                        Layout.fillWidth: true
                        height: UISettings.listItemHeight
                        from: 0
                        to: 100
                        suffix: "%"
                        value: Math.round(View3D.ambientIntensity * 100)
                        onValueModified: View3D.ambientIntensity = value / 100
                    }

                    RobotoText { height: UISettings.listItemHeight; label: qsTr("Smoke amount") }
                    CustomSpinBox {
                        Layout.fillWidth: true
                        height: UISettings.listItemHeight
                        from: 0
                        to: 100
                        suffix: "%"
                        value: Math.round(View3D.smokeAmount * 100)
                        onValueModified: View3D.smokeAmount = value / 100
                    }

                    RobotoText { height: UISettings.listItemHeight; label: qsTr("Show FPS") }
                    CustomCheckBox {
                        implicitHeight: UISettings.listItemHeight
                        implicitWidth: implicitHeight
                        checked: View3D.frameCountEnabled
                        onToggled: View3D.frameCountEnabled = checked
                    }
                }
            }

            SectionBox {
                width: parent.width
                visible: fxPropsVisible
                sectionLabel: qsTr("Position")
                sectionContents: GridLayout {
                    width: parent.width
                    columns: 2
                    columnSpacing: 5
                    rowSpacing: 2

                    function updatePosition(x, y, z)
                    {
                        if (isUpdating)
                            return;

                        if (selFixturesCount == 1 && selGenericCount == 0)
                        {
                            contextManager.fixturesPosition = Qt.vector3d(x, y, z)
                        }
                        else if (selFixturesCount == 0 && selGenericCount == 1)
                        {
                            View3D.genericItemsPosition = Qt.vector3d(x, y, z)
                        }
                        else
                        {
                            var newPos = Qt.vector3d(x - lastPosition.x, y - lastPosition.y, z - lastPosition.z)
                            contextManager.fixturesPosition = newPos
                            View3D.genericItemsPosition = newPos
                            lastPosition = Qt.vector3d(x, y, z)
                        }
                    }

                    RobotoText {
                        height: UISettings.listItemHeight
                        width: UISettings.bigItemHeight
                        textHAlign: Qt.AlignRight
                        label: "X"
                    }
                    CustomSpinBox {
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

                    RobotoText {
                        height: UISettings.listItemHeight
                        width: UISettings.bigItemHeight
                        textHAlign: Qt.AlignRight
                        label: "Y"
                    }
                    CustomSpinBox {
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

                    RobotoText {
                        height: UISettings.listItemHeight
                        width: UISettings.bigItemHeight
                        textHAlign: Qt.AlignRight
                        label: "Z"
                    }
                    CustomSpinBox {
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
                }
            }

            SectionBox {
                width: parent.width
                isExpanded: false
                visible: fxPropsVisible
                sectionLabel: qsTr("Rotation")
                sectionContents: GridLayout {
                    width: parent.width
                    columns: 2
                    columnSpacing: 5
                    rowSpacing: 2

                    function updateRotation(x, y, z)
                    {
                        if (isUpdating)
                            return;

                        if (selFixturesCount == 1 && selGenericCount == 0)
                        {
                            contextManager.fixturesRotation = fixtureRotationFromUi(Qt.vector3d(x, y, z))
                        }
                        else if (selFixturesCount == 0 && selGenericCount == 1)
                        {
                            View3D.genericItemsRotation = Qt.vector3d(x, y, z)
                        }
                        else
                        {
                            var newRot = Qt.vector3d(x - lastRotation.x, y - lastRotation.y, z - lastRotation.z)
                            if (selFixturesCount > 0)
                                contextManager.fixturesRotation = fixtureRotationFromUi(newRot)
                            if (selGenericCount > 0)
                                View3D.genericItemsRotation = newRot
                            lastRotation = Qt.vector3d(x, y, z)
                        }
                    }

                    RobotoText {
                        height: UISettings.listItemHeight
                        width: UISettings.bigItemHeight
                        textHAlign: Qt.AlignRight
                        label: "X"
                    }
                    CustomSpinBox {
                        id: xRotSpin
                        height: UISettings.listItemHeight
                        Layout.fillWidth: true
                        from: -359
                        to: 359
                        suffix: "°"
                        value: currentRotation.x
                        onValueModified: updateRotation(value, yRotSpin.value, zRotSpin.value)
                    }

                    RobotoText {
                        height: UISettings.listItemHeight
                        width: UISettings.bigItemHeight
                        textHAlign: Qt.AlignRight
                        label: "Y"
                    }
                    CustomSpinBox {
                        id: yRotSpin
                        height: UISettings.listItemHeight
                        Layout.fillWidth: true
                        from: -359
                        to: 359
                        suffix: "°"
                        value: currentRotation.y
                        onValueModified: updateRotation(xRotSpin.value, value, zRotSpin.value)
                    }

                    RobotoText {
                        height: UISettings.listItemHeight
                        width: UISettings.bigItemHeight
                        textHAlign: Qt.AlignRight
                        label: "Z"
                    }
                    CustomSpinBox {
                        id: zRotSpin
                        height: UISettings.listItemHeight
                        Layout.fillWidth: true
                        from: -359
                        to: 359
                        suffix: "°"
                        value: currentRotation.z
                        onValueModified: updateRotation(xRotSpin.value, yRotSpin.value, value)
                    }
                }
            }

            SectionBox {
                width: parent.width
                isExpanded: false
                visible: selGenericCount ? true : false
                sectionLabel: qsTr("Scale")
                sectionContents: GridLayout {
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
                        if (scaleLocked.checked)
                            currentScale = Qt.vector3d(x, x, x)
                    }

                    RobotoText {
                        height: UISettings.listItemHeight
                        width: UISettings.bigItemHeight
                        textHAlign: Qt.AlignRight
                        label: "X"
                    }
                    CustomSpinBox {
                        id: xScaleSpin
                        height: UISettings.listItemHeight
                        Layout.fillWidth: true
                        from: 1
                        to: 10000
                        suffix: "%"
                        value: currentScale.x
                        onValueModified: {
                            if (scaleLocked.checked)
                                updateScale(value, value, value)
                            else
                                updateScale(value, yScaleSpin.value, zScaleSpin.value)
                        }
                    }

                    Rectangle {
                        Layout.rowSpan: 3
                        Layout.fillHeight: true
                        color: "transparent"
                        width: UISettings.iconSizeMedium
                        clip: true

                        Rectangle {
                            color: "transparent"
                            x: -width / 2
                            y: UISettings.listItemHeight / 2
                            width: parent.width
                            height: parent.height - UISettings.listItemHeight
                            border.width: 1
                            border.color: "white"
                        }

                        IconButton {
                            id: scaleLocked
                            anchors.centerIn: parent
                            width: UISettings.iconSizeMedium
                            height: width
                            imgSource: "qrc:/lock.svg"
                            checkable: true
                            checked: true
                        }
                    }

                    RobotoText {
                        height: UISettings.listItemHeight
                        width: UISettings.bigItemHeight
                        textHAlign: Qt.AlignRight
                        label: "Y"
                    }
                    CustomSpinBox {
                        id: yScaleSpin
                        height: UISettings.listItemHeight
                        Layout.fillWidth: true
                        from: 1
                        to: 10000
                        suffix: "%"
                        value: currentScale.y
                        onValueModified: {
                            if (scaleLocked.checked)
                                updateScale(value, value, value)
                            else
                                updateScale(xScaleSpin.value, value, zScaleSpin.value)
                        }
                    }

                    RobotoText {
                        height: UISettings.listItemHeight
                        width: UISettings.bigItemHeight
                        textHAlign: Qt.AlignRight
                        label: "Z"
                    }
                    CustomSpinBox {
                        id: zScaleSpin
                        height: UISettings.listItemHeight
                        Layout.fillWidth: true
                        from: 1
                        to: 10000
                        suffix: "%"
                        value: currentScale.z
                        onValueModified: {
                            if (scaleLocked.checked)
                                updateScale(value, value, value)
                            else
                                updateScale(xScaleSpin.value, yScaleSpin.value, value)
                        }
                    }
                }
            }

            SectionBox {
                width: parent.width
                isExpanded: false
                sectionLabel: qsTr("Custom items")

                FileDialog {
                    id: meshDialog
                    visible: false
                    title: qsTr("Select a mesh file")
                    currentFolder: View3D.meshDirectory
                    nameFilters: [qsTr("3D files") + " (*.obj *.dae *.3ds *.stl *.blend *.gltf *.glb)",
                        qsTr("All files") + " (*)"]

                    onAccepted: View3D.createGenericItem(selectedFile, -1)
                }

                sectionContents: ColumnLayout {
                    width: parent.width

                    Rectangle {
                        width: parent.width
                        height: UISettings.iconSizeMedium

                        gradient: Gradient {
                            GradientStop { position: 0; color: UISettings.toolbarStartSub }
                            GradientStop { position: 1; color: UISettings.toolbarEnd }
                        }

                        RowLayout {
                            width: parent.width
                            height: UISettings.iconSizeMedium

                            IconButton {
                                height: UISettings.iconSizeMedium
                                width: height
                                faSource: FontAwesome.fa_plus
                                faColor: "limegreen"
                                tooltip: qsTr("Add a new item to the scene")
                                onClicked: meshDialog.open()
                            }
                            IconButton {
                                enabled: selGenericCount
                                height: UISettings.iconSizeMedium
                                width: height
                                faSource: FontAwesome.fa_minus
                                faColor: "crimson"
                                tooltip: qsTr("Remove the selected items")
                                onClicked: View3D.removeSelectedGenericItems()
                            }
                            IconButton {
                                enabled: selGenericCount
                                height: UISettings.iconSizeMedium
                                width: height
                                faSource: FontAwesome.fa_compress
                                faColor: UISettings.fgMain
                                tooltip: qsTr("Normalize the selected items")
                                onClicked: View3D.normalizeSelectedGenericItems()
                            }
                            Rectangle {
                                Layout.fillWidth: true
                                height: UISettings.iconSizeMedium
                                color: "transparent"
                            }
                        }
                    }

                    ListView {
                        id: itemsList
                        width: parent.width
                        height: UISettings.bigItemHeight * 4
                        boundsBehavior: Flickable.StopAtBounds
                        model: View3D.genericItemsList

                        delegate: Rectangle {
                            width: itemsList.width
                            height: UISettings.listItemHeight
                            color: isSelected ? UISettings.highlight : "transparent"

                            IconTextEntry {
                                width: parent.width
                                height: UISettings.listItemHeight
                                tLabel: name
                                faSource: FontAwesome.fa_cube
                                faColor: UISettings.fgMain

                                MouseArea {
                                    anchors.fill: parent
                                    onClicked: {
                                        giSelector.selectItem(index, itemsList.model, mouse.modifiers)
                                        View3D.setItemSelection(itemID, isSelected, mouse.modifiers)
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        ScrollBar.vertical: CustomScrollBar { id: sbar }
    }
}
