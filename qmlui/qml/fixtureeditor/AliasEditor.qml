/*
  Q Light Controller Plus
  AliasEditor.qml

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

import org.qlcplus.classes 1.0
import "."

Rectangle
{
    id: editorRoot
    color: "transparent"

    property EditorRef editorView: null
    property AliasEdit editor: null

    // called by the generic side editor loader (see EditorView.qml)
    function setItemIndex(channelName, capIndex)
    {
        editor = editorView.requestAliasEditor(channelName, capIndex)
    }

    // fallback so the loader's generic path never crashes
    function setItemName(name)
    {
    }

    ColumnLayout
    {
        anchors.fill: parent
        spacing: 5

        // header: capability being edited
        RobotoText
        {
            Layout.fillWidth: true
            height: UISettings.listItemHeight
            color: UISettings.sectionHeader
            leftMargin: 5
            wrapText: true
            fontBold: true
            fontSize: UISettings.textSizeDefault * 1.1
            labelColor: UISettings.fgMain
            label: editor ? editor.capabilityName : ""
        }

        RobotoText
        {
            Layout.fillWidth: true
            height: UISettings.listItemHeight
            wrapText: true
            fontItalic: true
            fontSize: UISettings.textSizeDefault * 0.8
            labelColor: UISettings.fgMedium
            label: qsTr("While this capability is active, the source channel is replaced by the target channel in the selected mode.")
        }

        // toolbar
        Rectangle
        {
            Layout.fillWidth: true
            height: UISettings.iconSizeDefault
            gradient: Gradient
            {
                GradientStop { position: 0; color: UISettings.toolbarStartSub }
                GradientStop { position: 1; color: UISettings.toolbarEnd }
            }

            RowLayout
            {
                anchors.fill: parent

                IconButton
                {
                    faSource: FontAwesome.fa_plus
                    faColor: "limegreen"
                    tooltip: qsTr("Add an alias")
                    enabled: editor ? editor.applicableModes.length > 0 && editor.allChannels.length > 0 : false
                    onClicked:
                    {
                        if (!editor)
                            return
                        var modes = editor.applicableModes
                        var channels = editor.allChannels
                        if (modes.length === 0 || channels.length === 0)
                            return
                        editor.addAlias(modes[0], channels[0])
                    }
                }

                Rectangle { Layout.fillWidth: true; color: "transparent" }

                GenericButton
                {
                    id: applyAllButton
                    label: qsTr("Apply to all modes")
                    height: UISettings.iconSizeDefault
                    Layout.preferredWidth: contentWidth + UISettings.bigItemHeight
                    //Layout.rightMargin: 5
                    enabled: editor ? editor.applicableModes.length > 0 : false
                    onClicked:
                    {
                        if (editor)
                            editor.applyToAllModes("")
                    }
                }
            }
        }

        // column headers
        RowLayout
        {
            Layout.fillWidth: true
            height: UISettings.listItemHeight
            spacing: 5

            RobotoText
            {
                Layout.preferredWidth: editorRoot.width * 0.3
                height: UISettings.listItemHeight
                labelColor: UISettings.sectionHeader
                label: qsTr("In mode")
            }
            RobotoText
            {
                Layout.preferredWidth: editorRoot.width * 0.28
                height: UISettings.listItemHeight
                labelColor: UISettings.sectionHeader
                label: qsTr("Replace")
            }
            RobotoText
            {
                Layout.fillWidth: true
                height: UISettings.listItemHeight
                labelColor: UISettings.sectionHeader
                label: qsTr("With")
            }
            // spacer matching warning + remove buttons
            Item { width: UISettings.listItemHeight * 2; height: 1 }
        }

        ListView
        {
            id: aliasRows
            Layout.fillWidth: true
            Layout.fillHeight: true
            boundsBehavior: Flickable.StopAtBounds
            clip: true
            model: editor ? editor.aliasList : null

            delegate:
                Item
                {
                    id: aliasRowDelegate
                    width: aliasRows.width
                    height: UISettings.listItemHeight

                    // capture the row roles/index up-front: inside the combos below
                    // the implicit "model"/"index" identifiers refer to the combo's
                    // own delegate context, not to this alias row
                    property int rowIndex: index
                    property string rowMode: model.mode
                    property string rowSource: model.sourceChannel
                    property string rowTarget: model.targetChannel
                    property string warning: editor ? editor.aliasWarning(index) : ""

                    RowLayout
                    {
                        anchors.fill: parent
                        spacing: 5

                        // mode
                        CustomComboBox
                        {
                            Layout.preferredWidth: editorRoot.width * 0.3
                            height: UISettings.listItemHeight
                            textRole: ""
                            model: editor ? editor.applicableModes : null
                            currentIndex: editor ? editor.applicableModes.indexOf(aliasRowDelegate.rowMode) : -1
                            onActivated: (idx) =>
                            {
                                if (editor)
                                    editor.setAliasMode(aliasRowDelegate.rowIndex, editor.applicableModes[idx])
                            }
                        }

                        // source channel (fixed: it is the owning channel)
                        RobotoText
                        {
                            Layout.preferredWidth: editorRoot.width * 0.28
                            height: UISettings.listItemHeight
                            label: aliasRowDelegate.rowSource
                        }

                        // target channel
                        CustomComboBox
                        {
                            Layout.fillWidth: true
                            height: UISettings.listItemHeight
                            textRole: ""
                            model: editor ? editor.allChannels : null
                            currentIndex: editor ? editor.allChannels.indexOf(aliasRowDelegate.rowTarget) : -1
                            onActivated: (idx) =>
                            {
                                if (editor)
                                    editor.setAliasTarget(aliasRowDelegate.rowIndex, editor.allChannels[idx])
                            }
                        }

                        IconButton
                        {
                            height: UISettings.listItemHeight
                            width: height
                            border.width: 0
                            visible: aliasRowDelegate.warning !== ""
                            faSource: FontAwesome.fa_triangle_exclamation
                            faColor: "yellow"
                            tooltip: aliasRowDelegate.warning
                        }

                        IconButton
                        {
                            height: UISettings.listItemHeight
                            width: height
                            faSource: FontAwesome.fa_minus
                            faColor: "crimson"
                            tooltip: qsTr("Remove this alias")
                            onClicked: if (editor) editor.removeAliasAtIndex(aliasRowDelegate.rowIndex)
                        }
                    }

                    Rectangle
                    {
                        width: parent.width
                        height: 1
                        y: parent.height - 1
                        color: UISettings.fgMedium
                    }
                }

            ScrollBar.vertical: CustomScrollBar { }
        }
    }
}
