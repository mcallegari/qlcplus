/*
  Q Light Controller Plus
  PopupChaserTempoConverter.qml

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

/** Convert Chasers between Time and Beats tempo at a chosen BPM, in place or
  * as copies used by the Show items */
CustomPopupDialog
{
    id: popupRoot
    width: mainView.width / 2
    title: qsTr("Convert Chaser tempo")
    standardButtons: Dialog.Ok | Dialog.Cancel

    /* The Chasers to convert. Empty means those of the Show items selected
       in the Show Manager */
    property var chaserIds: []
    /* True when converting to Beats tempo */
    property bool toBeats: true

    property var preview: ({ valid: false, lines: [], chaserIds: [] })
    property bool fromShow: chaserIds.length === 0
    property bool hasSections: showManager.isEditing && showManager.tempoSections.length > 0

    function options()
    {
        var bpmSource = bpmCombo.currValue
        var opts = {
            chaserIds: chaserIds,
            toBeats: toBeats,
            bpmMode: bpmSource === -1 ? "section" : "fixed",
            bpm: bpmSource >= 0 ? showManager.tempoSections[bpmSource].bpm : bpmSpin.realValue,
            resolution: resolutionCombo.currValue,
            clone: modeCombo.currValue === 1,
            allItems: scopeCombo.currValue === "selected" && itemsCombo.currValue === 1,
            perTempo: perTempoCheck.checked,
            scope: scopeCombo.currValue
        }
        return opts
    }

    function refresh()
    {
        if (!visible)
            return

        preview = showManager.tempoConversionPreview(options())

        // warn about the uses the engine doesn't know about
        var warnings = []
        for (var i = 0; i < preview.chaserIds.length; i++)
        {
            var fid = preview.chaserIds[i]
            if (modeCombo.currValue === 0)
            {
                var vcUses = virtualConsole.usageList(fid)
                if (vcUses.length && vcUses[0].classRef !== undefined)
                    warnings.push(qsTr("Also changes it for %1 Virtual Console widgets.").arg(vcUses.length))
            }
            // copies are not linked to the Speed Dials of the originals
            var dials = modeCombo.currValue === 0 ? virtualConsole.speedDialsUsing(fid) : []
            if (dials.length)
                warnings.push(qsTr("Controlled by the Speed Dial \"%1\", which sets its times in ms: " +
                                   "on a Beats tempo Chaser they would be read as beats. " +
                                   "Consider disconnecting it.").arg(dials.join("\", \"")))
        }
        warningText.text = warnings.join("\n")
    }

    onOpened:
    {
        scopeCombo.currValue = showManager.selectedItemsCount > 0 ? "selected" : "show"
        bpmCombo.currValue = hasSections ? -1 : -2
        bpmSpin.setValue(ioManager.bpmNumber * 100)
        refresh()
    }

    onAccepted:
    {
        if (preview.valid)
            showManager.applyTempoConversion(options())
    }

    contentItem:
        ColumnLayout
        {
            spacing: 5

            GridLayout
            {
                Layout.fillWidth: true
                columns: 2
                rowSpacing: 5
                columnSpacing: 5

                RobotoText { label: qsTr("Convert") }

                CustomComboBox
                {
                    id: directionCombo
                    Layout.fillWidth: true
                    Layout.preferredHeight: UISettings.listItemHeight
                    model: [
                        { mLabel: qsTr("Time tempo Chasers to Beats"), mValue: 1 },
                        { mLabel: qsTr("Beats tempo Chasers to Time"), mValue: 0 }
                    ]
                    currValue: popupRoot.toBeats ? 1 : 0
                    onValueChanged: (value) =>
                    {
                        popupRoot.toBeats = value === 1
                        popupRoot.refresh()
                    }
                }

                RobotoText
                {
                    visible: popupRoot.fromShow
                    label: qsTr("Chasers of")
                }

                CustomComboBox
                {
                    id: scopeCombo
                    visible: popupRoot.fromShow
                    Layout.fillWidth: true
                    Layout.preferredHeight: UISettings.listItemHeight
                    model: [
                        { mLabel: qsTr("Selected items"), mValue: "selected" },
                        { mLabel: qsTr("Whole Show"), mValue: "show" },
                        { mLabel: qsTr("All Shows"), mValue: "allShows" }
                    ]
                    currValue: "selected"
                    onValueChanged: popupRoot.refresh()
                }

                RobotoText { label: qsTr("Tempo") }

                RowLayout
                {
                    Layout.fillWidth: true

                    CustomComboBox
                    {
                        id: bpmCombo
                        Layout.fillWidth: true
                        Layout.preferredHeight: UISettings.listItemHeight
                        model:
                        {
                            var list = []
                            if (popupRoot.hasSections)
                                list.push({ mLabel: popupRoot.fromShow ? qsTr("Tempo section under each item")
                                                                       : qsTr("Tempo section at the Show Manager cursor"),
                                            mValue: -1 })
                            var sections = showManager.isEditing ? showManager.tempoSections : []
                            for (var i = 0; i < sections.length; i++)
                                list.push({ mLabel: qsTr("%1 (%2 BPM)").arg(sections[i].name).arg(sections[i].bpm), mValue: i })
                            list.push({ mLabel: qsTr("Custom BPM"), mValue: -2 })
                            return list
                        }
                        onValueChanged: popupRoot.refresh()
                    }

                    CustomDoubleSpinBox
                    {
                        id: bpmSpin
                        visible: bpmCombo.currValue === -2
                        Layout.preferredHeight: UISettings.listItemHeight
                        realFrom: 1
                        realTo: 999
                        realStep: 0.5
                        decimals: 2
                        suffix: ""
                        onRealValueChanged: popupRoot.refresh()
                    }
                }

                RobotoText
                {
                    visible: popupRoot.toBeats
                    label: qsTr("Round to")
                }

                CustomComboBox
                {
                    id: resolutionCombo
                    visible: popupRoot.toBeats
                    Layout.fillWidth: true
                    Layout.preferredHeight: UISettings.listItemHeight
                    model: [
                        { mLabel: qsTr("1 beat"), mValue: 1.0 },
                        { mLabel: qsTr("1/2 beat"), mValue: 0.5 },
                        { mLabel: qsTr("1/4 beat"), mValue: 0.25 },
                        { mLabel: qsTr("1/8 beat"), mValue: 0.125 }
                    ]
                    currValue: 0.25
                    onValueChanged: popupRoot.refresh()
                }

                RobotoText { label: qsTr("Apply to") }

                CustomComboBox
                {
                    id: modeCombo
                    Layout.fillWidth: true
                    Layout.preferredHeight: UISettings.listItemHeight
                    model: [
                        { mLabel: qsTr("The Chasers themselves"), mValue: 0 },
                        { mLabel: qsTr("New copies"), mValue: 1 }
                    ]
                    currValue: 0
                    onValueChanged: popupRoot.refresh()
                }

                RobotoText
                {
                    visible: modeCombo.currValue === 1 && popupRoot.fromShow && scopeCombo.currValue === "selected"
                    label: qsTr("Copies used by")
                }

                CustomComboBox
                {
                    id: itemsCombo
                    visible: modeCombo.currValue === 1 && popupRoot.fromShow && scopeCombo.currValue === "selected"
                    Layout.fillWidth: true
                    Layout.preferredHeight: UISettings.listItemHeight
                    model: [
                        { mLabel: qsTr("Selected items"), mValue: 0 },
                        { mLabel: qsTr("All items using them in this Show"), mValue: 1 }
                    ]
                    currValue: 0
                    onValueChanged: popupRoot.refresh()
                }

                RobotoText
                {
                    visible: modeCombo.currValue === 1 && popupRoot.fromShow && bpmCombo.currValue === -1
                    label: qsTr("Copies")
                }

                RowLayout
                {
                    visible: modeCombo.currValue === 1 && popupRoot.fromShow && bpmCombo.currValue === -1

                    CustomCheckBox
                    {
                        id: perTempoCheck
                        implicitWidth: UISettings.iconSizeMedium
                        implicitHeight: implicitWidth
                        checked: true
                        onToggled: popupRoot.refresh()
                    }
                    RobotoText
                    {
                        Layout.fillWidth: true
                        label: qsTr("One copy per tempo (otherwise one copy at the first item tempo)")
                    }
                }
            }

            // what the conversion will do
            Rectangle
            {
                Layout.fillWidth: true
                Layout.preferredHeight: UISettings.bigItemHeight * 2.5
                color: UISettings.bgStrong
                border.color: UISettings.bgLight

                Flickable
                {
                    anchors.fill: parent
                    anchors.margins: 5
                    clip: true
                    contentHeight: previewText.height
                    boundsBehavior: Flickable.StopAtBounds

                    Text
                    {
                        id: previewText
                        width: parent.width
                        wrapMode: Text.Wrap
                        color: UISettings.fgMain
                        font.family: UISettings.robotoFontName
                        font.pixelSize: UISettings.textSizeDefault * 0.9
                        text: popupRoot.preview.valid ? popupRoot.preview.lines.join("\n")
                                                      : popupRoot.preview.message
                    }
                }
            }

            Text
            {
                id: warningText
                Layout.fillWidth: true
                visible: text.length > 0
                wrapMode: Text.Wrap
                color: "orange"
                font.family: UISettings.robotoFontName
                font.pixelSize: UISettings.textSizeDefault * 0.9
            }
        }
}
