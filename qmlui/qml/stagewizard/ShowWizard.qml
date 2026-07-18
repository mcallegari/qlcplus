/*
  Q Light Controller Plus
  ShowWizard.qml

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

import "."

/* Full-screen wizard overlay. Activated by setting visible = true from MainView.qml */
Rectangle
{
    id: wizardRoot
    anchors.fill: parent
    color: "#1A1A2E"
    z: 100

    property int step: stageWizard ? stageWizard.currentStep : 0

    // Venue & Stage (index 2) is bypassed when the wizard only builds
    // functions/VC for existing groups (no new fixtures to lay out).
    // Re-evaluated whenever the group set or selection changes.
    property bool skipVenue: false
    function refreshSkipVenue()
    {
        skipVenue = stageWizard ? !stageWizard.hasNewGroups() : false
    }
    Connections
    {
        target: stageWizard
        function onGroupsModelChanged()     { wizardRoot.refreshSkipVenue() }
        function onGroupSelectionChanged()  { wizardRoot.refreshSkipVenue() }
    }
    Component.onCompleted: refreshSkipVenue()

    // Signal the Loader in MainView to unload us
    signal closeRequested()

    readonly property var stepTitles: [
        qsTr("Show Type"),
        qsTr("Fixture Groups & Roles"),
        qsTr("Venue & Stage"),
        qsTr("Effects"),
        qsTr("Controller"),
        qsTr("Summary")
    ]

    // ── Stubs for FixtureDrag.js which looks up these IDs at runtime ──────────
    // previewLoader.item.contextName must not be "2D" or "REMAP" so the drag
    // falls through to plain fixtureManager.addFixture() at position 0,0.
    QtObject
    {
        id: previewLoader
        property real y: 0
        property QtObject item: previewContext
    }
    // Top-level (not nested) so it can reference stepLoader without a pragma.
    // contextItem is the currently loaded step; step 2 exposes dropFixtureAt().
    QtObject
    {
        id: previewContext
        property string contextName: "WIZARD"
        property var    contextItem: stepLoader.item
    }
    QtObject { id: leftSidePanel; property real width: 0 }
    QtObject { id: viewToolbar;   property real height: 0 }

    // ── Close / open ─────────────────────────────────────────────────────────
    function open()
    {
        wizardRoot.visible = true
        if (stageWizard)
            stageWizard.reset()
    }

    function close()
    {
        wizardRoot.closeRequested()
    }

    Connections
    {
        target: stageWizard
        function onGenerationComplete()
        {
            wizardRoot.close()
        }
    }

    // ── Background decoration ─────────────────────────────────────────────────
    Canvas
    {
        id: bgCanvas
        anchors.fill: parent
        onPaint:
        {
            var ctx = getContext("2d")
            ctx.clearRect(0, 0, width, height)
            // Subtle diagonal gradient stripes
            var grad = ctx.createLinearGradient(0, 0, width, height)
            grad.addColorStop(0,   "#1A1A2E")
            grad.addColorStop(0.5, "#16213E")
            grad.addColorStop(1,   "#0F3460")
            ctx.fillStyle = grad
            ctx.fillRect(0, 0, width, height)
        }
    }

    // ── Header bar ────────────────────────────────────────────────────────────
    Rectangle
    {
        id: header
        width: parent.width
        height: UISettings.bigItemHeight * 0.5
        color: "transparent"

        RowLayout
        {
            anchors.fill: parent
            anchors.leftMargin: UISettings.iconSizeMedium
            anchors.rightMargin: UISettings.iconSizeMedium
            spacing: 0

            // Logo / title
            RobotoText
            {
                label: qsTr("Show Wizard")
                fontSize: UISettings.textSizeDefault * 1.8
                fontBold: true
                labelColor: "#0978FF"
            }

            Item { Layout.fillWidth: true }

            // Step title
            RobotoText
            {
                implicitWidth: width
                label: wizardRoot.stepTitles[wizardRoot.step] || ""
                fontSize: UISettings.textSizeDefault * 1.4
                labelColor: "#AAAACC"
            }

            Item { width: UISettings.iconSizeMedium }

            // Close button
            IconButton
            {
                faSource: FontAwesome.fa_xmark
                faColor: UISettings.fgMain
                tooltip: qsTr("Cancel wizard")
                onClicked: wizardRoot.close()
            }
        }
    }

    // ── Step indicator ────────────────────────────────────────────────────────
    WizardStepIndicator
    {
        id: stepIndicator
        anchors.top: header.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        currentStep: wizardRoot.step
        totalSteps: stageWizard ? stageWizard.totalSteps : 6
        stepTitles: wizardRoot.stepTitles
        skippedStep: wizardRoot.skipVenue ? 2 : -1
    }

    // ── Step content area ──────────────────────────────────────────────────────
    Item
    {
        id: contentArea
        anchors.top: stepIndicator.bottom
        anchors.topMargin: UISettings.listItemHeight
        anchors.bottom: footer.top
        anchors.bottomMargin: UISettings.listItemHeight * 0.5
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.leftMargin: UISettings.iconSizeMedium * 2
        anchors.rightMargin: UISettings.iconSizeMedium * 2

        // Swap content with a cross-fade when step changes
        Loader
        {
            id: stepLoader
            anchors.fill: parent
            opacity: 0

            // Map step index → QML source
            property var sources: [
                "qrc:/WizardStep1ShowType.qml",
                "qrc:/WizardStep2Fixtures.qml",
                "qrc:/WizardStep3Venue.qml",
                "qrc:/WizardStep4Effects.qml",
                "qrc:/WizardStep5Controller.qml",
                "qrc:/WizardStep6Summary.qml"
            ]

            source: sources[wizardRoot.step] || ""

            NumberAnimation on opacity
            {
                id: fadeIn
                to: 1.0
                duration: 220
                easing.type: Easing.OutCubic
            }
        }

        // Re-trigger fade when source changes
        Connections
        {
            target: stageWizard
            function onCurrentStepChanged() { stepLoader.opacity = 0; fadeIn.restart() }
        }
    }

    // ── Footer navigation ─────────────────────────────────────────────────────
    Rectangle
    {
        id: footer
        anchors.bottom: parent.bottom
        width: parent.width
        height: UISettings.bigItemHeight * 0.5
        color: "#0D0D1A"

        RowLayout
        {
            anchors.fill: parent
            anchors.leftMargin: UISettings.iconSizeMedium * 2
            anchors.rightMargin: UISettings.iconSizeMedium * 2

            // Back
            GenericButton
            {
                label: qsTr("← Back")
                visible: wizardRoot.step > 0
                enabled: !stageWizard.isGenerating
                bgColor: "#333355"
                hoverColor: "#222244"
                fgColor: "white"
                width: UISettings.bigItemHeight * 2
                height: UISettings.bigItemHeight * 0.4
                onClicked: stageWizard.currentStep = wizardRoot.step - 1
            }

            Item { Layout.fillWidth: true }

            // Progress text
            RobotoText
            {
                visible: stageWizard && stageWizard.isGenerating
                label: qsTr("Generating…")
                labelColor: "#0978FF"
                fontSize: UISettings.textSizeDefault
            }

            Item { Layout.fillWidth: true }

            // Next / Generate
            GenericButton
            {
                id: nextBtn
                label: wizardRoot.step < (stageWizard ? stageWizard.totalSteps - 1 : 5)
                       ? qsTr("Next →")
                       : qsTr("Generate ✦")
                enabled: stageWizard ? (stageWizard.canGoNext && !stageWizard.isGenerating) : false
                bgColor: enabled ? "#0550AA" : "#444455"
                hoverColor: "#0978FF"
                fgColor: "white"
                width: UISettings.bigItemHeight * 2
                height: UISettings.bigItemHeight * 0.4

                onClicked:
                {
                    var isLast = wizardRoot.step >= (stageWizard.totalSteps - 1)
                    if (isLast)
                        stageWizard.generate()
                    else
                        stageWizard.currentStep = wizardRoot.step + 1
                }
            }
        }
    }
}
