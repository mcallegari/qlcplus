/*
  Q Light Controller Plus
  InputProfileEditor.qml

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
import QtQuick.Controls
import QtQuick.Controls.Basic
import QtQml.Models

import org.qlcplus.classes 1.0
import "."

ColumnLayout
{
    id: peContainer

    property bool isEditing: false
    property var profileEditorRef: (typeof profileEditor === "undefined") ? null : profileEditor
    property bool hasProfileEditor: profileEditorRef !== null
    property int currentTab: 0
    property int selectedColorValue: -1
    property int selectedMidiChannelValue: -1
    property bool showExtraPress: false
    property bool showMovement: false
    property bool showAbsRel: false
    property bool showFeedback: false

    property bool canEditChannel: currentTab === 0 && channelList.selectedChannelNumber >= 0
    property bool canRemoveItem: currentTab === 0 ? channelList.selectedChannelNumber >= 0 :
                              (currentTab === 1 ? selectedColorValue >= 0 :
                               (currentTab === 2 ? selectedMidiChannelValue >= 0 : false))
    property string addTooltip: currentTab === 0 ? qsTr("Add a new channel") :
                               (currentTab === 1 ? qsTr("Add a new color") : qsTr("Add a new MIDI channel"))
    property string removeTooltip: currentTab === 0 ? qsTr("Delete the selected channel") :
                                  (currentTab === 1 ? qsTr("Delete the selected color") : qsTr("Delete the selected MIDI channel"))
    property string editTooltip: qsTr("Edit the selected channel")

    onCurrentTabChanged:
    {
        if (currentTab !== 0)
        {
            channelList.selectedChannelNumber = -1
            channelList.selectedChannel = null
            updateOptions(QLCInputChannel.NoType)
        }
        if (currentTab !== 1)
            selectedColorValue = -1
        if (currentTab !== 2)
            selectedMidiChannelValue = -1
    }

    function showWarning()
    {
        messagePopup.title = qsTr("!! Warning !!")
        messagePopup.message = qsTr("You are trying to edit a bundled input profile.<br>" +
                                    "If you modify and save it, a new file will be stored in<br><i>" +
                                    ioManager.profileUserFolder + "</i><br>and will override the bundled file.")
        messagePopup.standardButtons = Dialog.Ok
        messagePopup.open()
    }

    function showWizard()
    {
        messagePopup.title = qsTr("Channel wizard activated")
        messagePopup.message = qsTr("You have enabled the input channel wizard. After " +
                                    "clicking OK, wiggle your mapped input profile's " +
                                    "controls. They should appear into the list. " +
                                    "Click the wizard button again to stop channel " +
                                    "auto-detection.<br><br>Note that the wizard cannot " +
                                    "tell the difference between a knob and a slider " +
                                    "so you will have to do the change manually.")
        messagePopup.standardButtons = Dialog.Ok
        messagePopup.open()
    }

    function showSaveFirst()
    {
        messagePopup.title = qsTr("Unsaved changes")
        messagePopup.message = qsTr("Do you wish to save the current profile first?\nChanges will be lost if you don't save them.")
        messagePopup.standardButtons = Dialog.Yes | Dialog.No | Dialog.Cancel
        messagePopup.open()
    }

    function updateOptions(type)
    {
        var extraPress = false
        var movement = false
        var absRel = false
        var feedback = false

        switch (type)
        {
            case QLCInputChannel.Button:
                extraPress = true
                feedback = true
            break
            case QLCInputChannel.Slider:
            case QLCInputChannel.Knob:
                movement = true
                absRel = true
                sensitivitySpin.from = 10
                sensitivitySpin.to = 100
            break
            case QLCInputChannel.Encoder:
                movement = true
                sensitivitySpin.from = 1
                sensitivitySpin.to = 20
            break
            default:
            break
        }

        showExtraPress = extraPress
        showMovement = movement
        showAbsRel = absRel
        showFeedback = feedback
    }

    function selectedChannel()
    {
        return channelList.selectedChannelNumber;
    }

    function addNewChannel()
    {
        if (!hasProfileEditor)
            return
        channelList.selectedChannelNumber = -1
        inputChannelEditor.open()
        inputChannelEditor.initialize(-1, profileEditorRef.type)
    }

    function editSelectedChannel()
    {
        if (!hasProfileEditor)
            return
        inputChannelEditor.open()
        inputChannelEditor.initialize(channelList.selectedChannelNumber - 1, profileEditorRef.type)
    }

    function removeSelectedChannel()
    {
        if (!hasProfileEditor)
            return
        profileEditorRef.removeChannel(channelList.selectedChannelNumber - 1)
        channelList.selectedChannelNumber = -1
    }

    function removeSelectedColor()
    {
        if (!hasProfileEditor || selectedColorValue < 0)
            return
        profileEditorRef.removeColor(selectedColorValue)
        selectedColorValue = -1
    }

    function removeSelectedMidiChannel()
    {
        if (!hasProfileEditor || selectedMidiChannelValue < 0)
            return
        profileEditorRef.removeMidiChannel(selectedMidiChannelValue)
        selectedMidiChannelValue = -1
    }

    function addItem()
    {
        if (!hasProfileEditor)
            return
        if (currentTab === 0)
            addNewChannel()
        else if (currentTab === 1)
            addColorPopup.open()
        else if (currentTab === 2)
            addMidiChannelPopup.open()
    }

    function removeItem()
    {
        if (currentTab === 0)
            removeSelectedChannel()
        else if (currentTab === 1)
            removeSelectedColor()
        else if (currentTab === 2)
            removeSelectedMidiChannel()
    }

    function setCurrentTab(tabIndex)
    {
        currentTab = tabIndex

        if (tabIndex === 0)
            inputMappingTab.checked = true
        else if (tabIndex === 1)
            colorsTab.checked = true
        else if (tabIndex === 2)
            midiChannelsTab.checked = true
    }

    Connections
    {
        target: profileEditorRef
        ignoreUnknownSignals: true

        function onTypeChanged()
        {
            if (peContainer.currentTab === 2 && target && target.type !== QLCInputProfile.MIDI)
                peContainer.setCurrentTab(0)
        }
    }

    Component.onDestruction: ioManager.finishInputProfile()

    CustomPopupDialog
    {
        id: messagePopup
        standardButtons: Dialog.Ok
        title: qsTr("!! Warning !!")

        onClicked: (role) =>
        {
            if (role === Dialog.Yes)
            {
                ioManager.saveInputProfile()
            }
            else if (role === Dialog.No)
            {
                ioManager.finishInputProfile()
                isEditing = false
                close()
            }
            else if (role === Dialog.Ok || role === Dialog.Cancel)
            {
                close()
            }
        }
    }

    PopupInputChannelEditor
    {
        id: inputChannelEditor

        onAccepted:
        {
            if (!hasProfileEditor)
                return
            if (profileEditorRef.saveChannel(channelList.selectedChannelNumber - 1, currentChannelNumber) < 0)
            {
                messagePopup.title = qsTr("!! Warning !!")
                messagePopup.message = qsTr("Channel " + (currentChannelNumber + 1) + " already exists!")
                messagePopup.standardButtons = Dialog.Ok
                messagePopup.open()
            }
        }
    }

    CustomPopupDialog
    {
        id: addColorPopup
        title: qsTr("Add Color")
        standardButtons: Dialog.Ok | Dialog.Cancel
        width: Math.min(mainView.width * 0.8, 520)

        property color pickedColor: "#ffffff"

        onOpened:
        {
            colorValueSpin.value = 0
            colorLabelEdit.text = ""
            pickedColor = "#ffffff"
        }

        onClicked: (role) =>
        {
            if (role === Dialog.Ok)
            {
                if (hasProfileEditor)
                {
                    profileEditorRef.addColor(colorValueSpin.value, colorLabelEdit.text, pickedColor)
                    selectedColorValue = colorValueSpin.value
                }
            }
            close()
        }

        contentItem:
            ColumnLayout
            {
                width: parent.width
                spacing: 6

                GridLayout
                {
                    columns: 2
                    columnSpacing: 5
                    rowSpacing: 4
                    Layout.fillWidth: true

                    RobotoText
                    {
                        label: qsTr("Value")
                    }
                    CustomSpinBox
                    {
                        id: colorValueSpin
                        Layout.fillWidth: true
                        from: 0
                        to: 255
                    }

                    RobotoText
                    {
                        label: qsTr("Label")
                    }
                    CustomTextEdit
                    {
                        id: colorLabelEdit
                        Layout.fillWidth: true
                    }

                    RobotoText
                    {
                        label: qsTr("Color")
                    }
                    Rectangle
                    {
                        width: UISettings.bigItemHeight
                        height: UISettings.listItemHeight
                        color: addColorPopup.pickedColor
                        border.width: 2
                        border.color: UISettings.fgMedium
                    }
                }

                ColorTool
                {
                    Layout.alignment: Qt.AlignHCenter
                    currentRGB: addColorPopup.pickedColor
                    currentWAUV: "black"
                    showCloseButton: false
                    showPalette: false

                    onToolColorChanged: (r, g, b, w, a, uv) =>
                    {
                        addColorPopup.pickedColor = Qt.rgba(r, g, b, 1.0)
                    }
                }
            }
    }

    CustomPopupDialog
    {
        id: addMidiChannelPopup
        title: qsTr("Add MIDI Channel")
        standardButtons: Dialog.Ok | Dialog.Cancel
        width: mainView.width / 3

        onOpened:
        {
            midiChannelSpin.value = 1
            midiChannelLabelEdit.text = ""
        }

        onClicked: (role) =>
        {
            if (role === Dialog.Ok)
            {
                if (hasProfileEditor)
                {
                    profileEditorRef.addMidiChannel(midiChannelSpin.value - 1, midiChannelLabelEdit.text)
                    selectedMidiChannelValue = midiChannelSpin.value - 1
                }
            }
            close()
        }

        contentItem:
            GridLayout
            {
                width: parent.width
                columns: 2
                columnSpacing: 5
                rowSpacing: 4

                RobotoText
                {
                    label: qsTr("Channel")
                }
                CustomSpinBox
                {
                    id: midiChannelSpin
                    Layout.fillWidth: true
                    from: 1
                    to: 16
                }

                RobotoText
                {
                    label: qsTr("Name")
                }
                CustomTextEdit
                {
                    id: midiChannelLabelEdit
                    Layout.fillWidth: true
                }
            }
    }

    Rectangle
    {
        implicitWidth: peContainer.width
        implicitHeight: infoBox.height
        z: 2
        color: UISettings.bgMedium

        GridLayout
        {
            id: infoBox
            width: peContainer.width
            columns: 2

            RobotoText
            {
                implicitHeight: UISettings.listItemHeight
                label: qsTr("Manufacturer")
            }
            CustomTextEdit
            {
                Layout.fillWidth: true
                text: peContainer.visible && hasProfileEditor ? profileEditorRef.manufacturer : ""
                onTextEdited: if (hasProfileEditor) profileEditorRef.manufacturer = text
            }
            RobotoText
            {
                implicitHeight: UISettings.listItemHeight
                label: qsTr("Model")
            }
            CustomTextEdit
            {
                Layout.fillWidth: true
                text: peContainer.visible && hasProfileEditor ? profileEditorRef.model : ""
                onTextEdited: if (hasProfileEditor) profileEditorRef.model = text
            }
            RobotoText
            {
                implicitHeight: UISettings.listItemHeight
                label: qsTr("Type")
            }
            CustomComboBox
            {
                Layout.fillWidth: true
                model: [
                    { mLabel: "MIDI", mValue: 0 },
                    { mLabel: "OS2L", mValue: 1 },
                    { mLabel: "OSC", mValue: 2 },
                    { mLabel: "HID", mValue: 3 },
                    { mLabel: "DMX", mValue: 4 },
                    { mLabel: "ENTTEC", mValue: 5 }
                ]
                currValue: peContainer.visible && hasProfileEditor ? profileEditorRef.type : 0
                onValueChanged: if (hasProfileEditor) profileEditorRef.type = currentValue
            }

            GroupBox
            {
                title: qsTr("MIDI Global Settings")
                Layout.columnSpan: 2
                Layout.fillWidth: true
                visible: peContainer.visible && hasProfileEditor ? profileEditorRef.type === QLCInputProfile.MIDI : false
                font.family: UISettings.robotoFontName
                font.pixelSize: UISettings.textSizeDefault
                palette.windowText: UISettings.fgMain

                RowLayout
                {
                    width: parent.width

                    CustomCheckBox
                    {
                        implicitHeight: UISettings.listItemHeight
                        implicitWidth: implicitHeight
                        checked: peContainer.visible && hasProfileEditor ? profileEditorRef.midiNoteOff : false
                        onToggled: if (hasProfileEditor) profileEditorRef.midiNoteOff = checked
                    }
                    RobotoText
                    {
                        Layout.fillWidth: true
                        wrapText: true
                        label: qsTr("When MIDI notes are used, send a Note Off when value is 0")
                    }
                }
            } // GroupBox
        } // GridLayout
    } // Rectangle

    Rectangle
    {
        id: tabsBar
        implicitWidth: peContainer.width
        height: UISettings.listItemHeight
        z: 2
        gradient:
            Gradient
            {
                id: tabsGradient
                GradientStop { position: 0; color: UISettings.toolbarStartSub }
                GradientStop { position: 1; color: UISettings.toolbarEnd }
            }

        RowLayout
        {
            anchors.fill: parent
            spacing: 0

            MenuBarEntry
            {
                id: inputMappingTab
                Layout.fillWidth: true
                Layout.preferredWidth: 1
                Layout.minimumWidth: 0
                entryText: qsTr("Input Mapping")
                autoExclusive: true
                checked: true
                checkedColor: UISettings.toolbarSelectionSub
                bgGradient: tabsGradient
                mFontSize: UISettings.textSizeDefault
                onClicked: peContainer.setCurrentTab(0)
            }

            MenuBarEntry
            {
                id: colorsTab
                Layout.fillWidth: true
                Layout.preferredWidth: 1
                Layout.minimumWidth: 0
                entryText: qsTr("Colors")
                autoExclusive: true
                checkedColor: UISettings.toolbarSelectionSub
                bgGradient: tabsGradient
                mFontSize: UISettings.textSizeDefault
                onClicked: peContainer.setCurrentTab(1)
            }

            MenuBarEntry
            {
                id: midiChannelsTab
                visible: peContainer.visible && hasProfileEditor && profileEditorRef.type === QLCInputProfile.MIDI
                Layout.fillWidth: true
                Layout.preferredWidth: 1
                Layout.minimumWidth: 0
                entryText: qsTr("MIDI Channels")
                autoExclusive: true
                checkedColor: UISettings.toolbarSelectionSub
                bgGradient: tabsGradient
                mFontSize: UISettings.textSizeDefault
                onClicked: peContainer.setCurrentTab(2)
            }
        }
    }

    ListView
    {
        id: channelList
        implicitWidth: peContainer.width
        Layout.fillHeight: peContainer.currentTab === 0
        visible: peContainer.currentTab === 0
        boundsBehavior: Flickable.StopAtBounds
        clip: true
        z: 1

        model: peContainer.visible && hasProfileEditor ? profileEditorRef.channels : null

        ScrollBar.vertical: CustomScrollBar { }

        property int selectedChannelNumber: -1
        property QLCInputChannel selectedChannel: null

        header:
            RowLayout
            {
                width: channelList.width
                height: UISettings.listItemHeight

                RobotoText
                {
                    width: UISettings.bigItemHeight
                    height: UISettings.listItemHeight
                    label: qsTr("Channel")
                    color: UISettings.sectionHeader
                }
                Rectangle { width: 1; height: UISettings.listItemHeight; color: UISettings.fgMedium }

                RobotoText
                {
                    Layout.fillWidth: true
                    height: UISettings.listItemHeight
                    label: qsTr("Name")
                    color: UISettings.sectionHeader
                }
                Rectangle { width: 1; height: UISettings.listItemHeight; color: UISettings.fgMedium }

                RobotoText
                {
                    width: UISettings.bigItemHeight * 1.5
                    height: UISettings.listItemHeight
                    label: qsTr("Type")
                    color: UISettings.sectionHeader
                }
            }

        delegate:
            Item
            {
                width: channelList.width
                height: UISettings.listItemHeight

                property QLCInputChannel channel: modelData.cRef

                Rectangle
                {
                    anchors.fill: parent
                    color: UISettings.highlight
                    visible: channelList.selectedChannelNumber === modelData.chNumber
                }

                RowLayout
                {
                    anchors.fill: parent

                    RobotoText
                    {
                        width: UISettings.bigItemHeight
                        height: UISettings.listItemHeight
                        label: modelData.chNumber
                    }
                    Rectangle { width: 1; height: UISettings.listItemHeight; color: UISettings.fgMedium }

                    RobotoText
                    {
                        Layout.fillWidth: true
                        height: UISettings.listItemHeight
                        label: channel ? channel.name : ""
                    }
                    Rectangle { width: 1; height: UISettings.listItemHeight; color: UISettings.fgMedium }

                    IconTextEntry
                    {
                        width: UISettings.bigItemHeight * 1.5
                        height: UISettings.listItemHeight
                        tLabel: channel ? channel.typeString : ""
                        iSrc: channel ? channel.iconResource(channel.type, true) : ""
                    }
                }

                MouseArea
                {
                    anchors.fill: parent
                    onClicked:
                    {
                        channelList.selectedChannelNumber = modelData.chNumber
                        channelList.selectedChannel = channel
                        updateOptions(channel.type)
                    }
                }
            }
    } // ListView

    ListView
    {
        id: colorTableList
        implicitWidth: peContainer.width
        Layout.fillHeight: peContainer.currentTab === 1
        visible: peContainer.currentTab === 1
        boundsBehavior: Flickable.StopAtBounds
        clip: true
        z: 1

        model: peContainer.visible && hasProfileEditor ? profileEditorRef.colorTable : null

        ScrollBar.vertical: CustomScrollBar { }

        onModelChanged: peContainer.selectedColorValue = -1

        header:
            RowLayout
            {
                width: colorTableList.width
                height: UISettings.listItemHeight

                RobotoText
                {
                    width: UISettings.bigItemHeight * 0.7
                    height: UISettings.listItemHeight
                    label: qsTr("Value")
                    color: UISettings.sectionHeader
                }
                Rectangle { width: 1; height: UISettings.listItemHeight; color: UISettings.fgMedium }

                RobotoText
                {
                    Layout.fillWidth: true
                    height: UISettings.listItemHeight
                    label: qsTr("Label")
                    color: UISettings.sectionHeader
                }
                Rectangle { width: 1; height: UISettings.listItemHeight; color: UISettings.fgMedium }

                RobotoText
                {
                    width: UISettings.bigItemHeight
                    height: UISettings.listItemHeight
                    label: qsTr("Color")
                    color: UISettings.sectionHeader
                }
            }

        delegate:
            Item
            {
                width: colorTableList.width
                height: UISettings.listItemHeight

                Rectangle
                {
                    anchors.fill: parent
                    color: UISettings.highlight
                    visible: peContainer.selectedColorValue === modelData.value
                }

                RowLayout
                {
                    anchors.fill: parent

                    RobotoText
                    {
                        width: UISettings.bigItemHeight * 0.7
                        height: UISettings.listItemHeight
                        label: modelData.value
                    }
                    Rectangle { width: 1; height: UISettings.listItemHeight; color: UISettings.fgMedium }

                    RobotoText
                    {
                        Layout.fillWidth: true
                        height: UISettings.listItemHeight
                        label: modelData.label
                    }
                    Rectangle { width: 1; height: UISettings.listItemHeight; color: UISettings.fgMedium }

                    Rectangle
                    {
                        width: UISettings.bigItemHeight
                        height: UISettings.listItemHeight
                        color: modelData.color
                        border.width: 1
                        border.color: UISettings.fgMedium
                    }
                }

                MouseArea
                {
                    anchors.fill: parent
                    onClicked: peContainer.selectedColorValue = modelData.value
                }
            }
    } // ListView

    ListView
    {
        id: midiChannelList
        implicitWidth: peContainer.width
        Layout.fillHeight: peContainer.currentTab === 2
        visible: peContainer.currentTab === 2
        boundsBehavior: Flickable.StopAtBounds
        clip: true
        z: 1

        model: peContainer.visible && hasProfileEditor ? profileEditorRef.midiChannelTable : null

        ScrollBar.vertical: CustomScrollBar { }

        onModelChanged: peContainer.selectedMidiChannelValue = -1

        header:
            RowLayout
            {
                width: midiChannelList.width
                height: UISettings.listItemHeight

                RobotoText
                {
                    width: UISettings.bigItemHeight
                    height: UISettings.listItemHeight
                    label: qsTr("Channel")
                    color: UISettings.sectionHeader
                }
                Rectangle { width: 1; height: UISettings.listItemHeight; color: UISettings.fgMedium }

                RobotoText
                {
                    Layout.fillWidth: true
                    height: UISettings.listItemHeight
                    label: qsTr("Name")
                    color: UISettings.sectionHeader
                }
            }

        delegate:
            Item
            {
                width: midiChannelList.width
                height: UISettings.listItemHeight

                Rectangle
                {
                    anchors.fill: parent
                    color: UISettings.highlight
                    visible: peContainer.selectedMidiChannelValue === modelData.value
                }

                RowLayout
                {
                    anchors.fill: parent

                    RobotoText
                    {
                        width: UISettings.bigItemHeight
                        height: UISettings.listItemHeight
                        label: modelData.channel
                    }
                    Rectangle { width: 1; height: UISettings.listItemHeight; color: UISettings.fgMedium }

                    RobotoText
                    {
                        Layout.fillWidth: true
                        height: UISettings.listItemHeight
                        label: modelData.label
                    }
                }

                MouseArea
                {
                    anchors.fill: parent
                    onClicked: peContainer.selectedMidiChannelValue = modelData.value
                }
            }
    } // ListView

    GroupBox
    {
        id: extraPressGroup
        title: qsTr("Behaviour")
        Layout.fillWidth: true
        visible: peContainer.currentTab === 0 && peContainer.showExtraPress
        font.family: UISettings.robotoFontName
        font.pixelSize: UISettings.textSizeDefault
        palette.windowText: UISettings.fgMain

        RowLayout
        {
            width: parent.width

            CustomCheckBox
            {
                implicitHeight: UISettings.listItemHeight
                implicitWidth: implicitHeight
                checked: channelList.selectedChannel ? channelList.selectedChannel.sendExtraPress : false
                onToggled: channelList.selectedChannel.sendExtraPress = checked
            }
            RobotoText
            {
                Layout.fillWidth: true
                wrapText: true
                label: qsTr("Generate an extra Press/Release when toggled")
            }
        }
    } // GroupBox

    GroupBox
    {
        id: movementGroup
        title: qsTr("Behaviour")
        Layout.fillWidth: true
        visible: peContainer.currentTab === 0 && peContainer.showMovement
        font.family: UISettings.robotoFontName
        font.pixelSize: UISettings.textSizeDefault
        palette.windowText: UISettings.fgMain

        RowLayout
        {
            width: parent.width

            RobotoText
            {
                id: movementLabel
                implicitHeight: UISettings.listItemHeight
                visible: peContainer.showAbsRel
                label: qsTr("Movement")
            }
            CustomComboBox
            {
                id: movementCombo
                implicitHeight: UISettings.listItemHeight
                visible: peContainer.showAbsRel
                model: [
                    { mLabel: "Absolute", mValue: QLCInputChannel.Absolute },
                    { mLabel: "Relative", mValue: QLCInputChannel.Relative }
                ]
                currentIndex: channelList.selectedChannel ? channelList.selectedChannel.movementType : QLCInputChannel.Absolute
                onValueChanged: channelList.selectedChannel.movementType = currentValue
            }
            RobotoText
            {
                implicitHeight: UISettings.listItemHeight
                label: qsTr("Sensitivity")
            }
            CustomSpinBox
            {
                id: sensitivitySpin
                implicitHeight: UISettings.listItemHeight
                from: 10
                to: 100
                value: channelList.selectedChannel ? channelList.selectedChannel.movementSensitivity : 20
                onValueModified: channelList.selectedChannel.movementSensitivity = value
            }
        }
    } // GroupBox

    GroupBox
    {
        id: feedbackGroup
        title: qsTr("Custom Feedback")
        Layout.fillWidth: true
        visible: peContainer.currentTab === 0 && peContainer.showFeedback
        font.family: UISettings.robotoFontName
        font.pixelSize: UISettings.textSizeDefault
        palette.windowText: UISettings.fgMain

        RowLayout
        {
            width: parent.width

            RobotoText
            {
                implicitHeight: UISettings.listItemHeight
                label: qsTr("Lower value")
            }
            CustomSpinBox
            {
                implicitHeight: UISettings.listItemHeight
                from: 0
                to: 255
                value: channelList.selectedChannel ? channelList.selectedChannel.lowerValue : 0
                onValueModified: channelList.selectedChannel.lowerValue = value
            }
            RobotoText
            {
                implicitHeight: UISettings.listItemHeight
                label: qsTr("Upper value")
            }
            CustomSpinBox
            {
                implicitHeight: UISettings.listItemHeight
                from: 0
                to: 255
                value: channelList.selectedChannel ? channelList.selectedChannel.upperValue : 255
                onValueModified: channelList.selectedChannel.upperValue = value
            }
        }
    } // GroupBox

} // ColumnLayout
