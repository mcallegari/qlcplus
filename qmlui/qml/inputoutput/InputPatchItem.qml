/*
  Q Light Controller Plus
  InputPatchItem.qml

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

import org.qlcplus.classes 1.0
import "GenericHelpers.js" as Helpers
import "."

Rectangle
{
    id: ipRoot
    width: parent.width
    height: UISettings.bigItemHeight * 0.9
    color: "transparent"

    property int universeID
    property InputPatch patch
    property Universe universe
    property OutputPatch feedbackPatch: universe && universe.hasFeedback ? universe.feedbackPatch() : null

    signal removeProfile()

    Rectangle
    {
        id: profileBox
        width: parent.width
        height: parent.height
        visible: patch ? (patch.profileName === "None" ? false : true) : false

        border.width: 2
        border.color: UISettings.borderColorDark
        color: "#269ABA"
        radius: 10

        IconButton
        {
            y: 4
            x: parent.width - width - 10
            height: UISettings.bigItemHeight * 0.25
            width: height
            faSource: FontAwesome.fa_xmark
            faColor: "white"
            tooltip: qsTr("Remove this input profile")
            onClicked: ipRoot.removeProfile()
        }

        RobotoText
        {
            x: 10
            y: 3
            height: UISettings.bigItemHeight * 0.3
            width: parent.width - 20
            label: patch ? patch.profileName : ""
            labelColor: "black"
            fontSize: UISettings.textSizeDefault
        }
    }

    Rectangle
    {
        id: patchBox
        width: profileBox.visible ? parent.width - 10 : parent.width
        height: profileBox.visible ? parent.height - UISettings.bigItemHeight * 0.3 - 5 : parent.height
        y: profileBox.visible ? UISettings.bigItemHeight * 0.3 : 0
        x: profileBox.visible ? 5 : 0
        z: 1
        radius: 3
        color: UISettings.bgLighter
        border.width: 2
        border.color: UISettings.borderColorDark

        /* LED kind-of signal indicator */
        Rectangle
        {
            id: valueChangeBox
            x: parent.width - width - 8
            y: profileBox.visible ? 5 : 10
            z: 1
            width: UISettings.iconSizeMedium * 0.75
            height: width
            radius: height / 2
            border.width: 2
            border.color: UISettings.bgMedium
            color: UISettings.bgLight

            ColorAnimation on color
            {
                id: cAnim
                from: "#00FF00"
                to: UISettings.bgLight
                duration: 500
                running: false
            }

            Connections
            {
                id: valChangedSignal
                target: patch
                function onInputValueChanged(inputUniverse, channel, value, key)
                {
                    cAnim.restart()
                }
            }
        }

        RowLayout
        {
            x: 8
            width: parent.width - 16 - (fbLineButton.visible ? fbLineButton.width + 4 : 0)
            anchors.verticalCenter: parent.verticalCenter
            spacing: 3

            Image
            {
                height: ipRoot.height * 0.75
                width: height
                source: patch ? Helpers.pluginIconFromName(patch.pluginName) : ""
                sourceSize: Qt.size(width, height)
                fillMode: Image.Stretch
            }
            RobotoText
            {
                height: ipRoot.height
                Layout.fillWidth: true
                label: patch ? patch.inputName : ""
                labelColor: "black"
                wrapText: true
                fontSize: UISettings.textSizeDefault
            }
        }

        /* feedback destination indicator/selector, anchored where the
         * feedback wire (drawn in PatchWireBox) joins this patch box,
         * i.e. the bottom right corner. Shown only when feedback is
         * enabled on this universe. Lets the user pick a different
         * line of the same plugin to send feedback to. */
        RobotoText
        {
            anchors.right: fbLineButton.left
            anchors.bottom: parent.bottom
            anchors.verticalCenter: fbLineButton.verticalCenter
            anchors.rightMargin: 5
            anchors.bottomMargin: 5
            label: feedbackPatch ? feedbackPatch.outputName : ""
            labelColor: UISettings.fgMain
            fontItalic: true
            fontSize: UISettings.textSizeDefault * 0.8
            visible: (universe && universe.hasFeedback && feedbackPatch && patch)
                     ? feedbackPatch.outputName !== patch.inputName : false
        }

        IconPopupButton
        {
            id: fbLineButton
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.rightMargin: 2
            anchors.bottomMargin: 5
            implicitWidth: UISettings.iconSizeMedium * 0.85
            implicitHeight: UISettings.iconSizeMedium * 0.75
            visible: universe ? universe.hasFeedback : false

            property string tooltipText: qsTr("Select the line to send feedback to")

            // IconPopupButton drives its inner button's tooltip from the
            // current selection text (see onDisplayTextChanged); override
            // it back to a static, descriptive tooltip every time it changes
            onDisplayTextChanged: contentItem.tooltip = tooltipText
            Component.onCompleted: contentItem.tooltip = tooltipText

            property var sourcesList: ipRoot.universe && ipRoot.universe.hasFeedback
                                        ? ioManager.universeFeedbackSources(ipRoot.universeID) : []

            model:
            {
                let mdl = []
                for (let i = 0; i < sourcesList.length; i++)
                {
                    mdl.push({ mLabel: sourcesList[i].name,
                                faIcon: FontAwesome.fa_arrow_right_arrow_left,
                                mValue: i })
                }
                return mdl
            }

            // reflect the line currently patched as feedback, without
            // triggering a write back to the engine (see onValueChanged)
            currValue:
            {
                for (let i = 0; i < sourcesList.length; i++)
                    if (sourcesList[i].checked)
                        return i
                return -1
            }

            onValueChanged: (value) =>
            {
                if (value < 0 || value >= sourcesList.length)
                    return
                if (sourcesList[value].checked)
                    return // just reflecting current state, not a user pick
                ioManager.setFeedbackLine(ipRoot.universeID, sourcesList[value].plugin, sourcesList[value].line)
            }
        }
    }
}

