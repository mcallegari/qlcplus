import QtQuick
import QtQuick.Layouts
import "."

GridLayout
{
    id: root
    columns: 4
    columnSpacing: 4
    rowSpacing: 0
    Layout.fillWidth: true
    Layout.minimumWidth: 0

    property int firstColorIndex: -1
    property int secondColorIndex: -1
    property bool firstResetVisible: false
    property bool secondResetVisible: false
    property string firstResetTooltip: ""
    property string secondResetTooltip: ""
    property alias firstColor: firstColorButton.color
    property alias secondColor: secondColorButton.color
    signal colorClicked(int colorIndex, var previewButton)
    signal resetClicked(int colorIndex)

    Rectangle
    {
        id: firstColorButton
        objectName: "rgbColorButton_" + root.firstColorIndex
        visible: root.firstColorIndex >= 0
        Layout.fillWidth: true
        Layout.minimumWidth: 0
        Layout.fillHeight: true
        radius: 5
        border.width: 2
        border.color: firstColorMouse.containsMouse ? "white" : UISettings.bgLight

        MouseArea
        {
            id: firstColorMouse
            anchors.fill: parent
            hoverEnabled: true
            onClicked: root.colorClicked(root.firstColorIndex, firstColorButton)
        }
    }

    Item
    {
        visible: root.firstColorIndex >= 0
        Layout.preferredWidth: root.height
        Layout.minimumWidth: root.height
        Layout.maximumWidth: root.height
        Layout.fillHeight: true

        IconButton
        {
            objectName: "rgbColorReset_" + root.firstColorIndex
            anchors.fill: parent
            visible: root.firstResetVisible
            faSource: FontAwesome.fa_xmark
            faColor: "darkred"
            tooltip: root.firstResetTooltip
            onClicked:
            {
                firstColorButton.color = "transparent"
                root.resetClicked(root.firstColorIndex)
            }
        }
    }

    Rectangle
    {
        id: secondColorButton
        objectName: "rgbColorButton_" + root.secondColorIndex
        visible: root.secondColorIndex >= 0
        Layout.fillWidth: true
        Layout.minimumWidth: 0
        Layout.fillHeight: true
        radius: 5
        border.width: 2
        border.color: secondColorMouse.containsMouse ? "white" : UISettings.bgLight

        MouseArea
        {
            id: secondColorMouse
            anchors.fill: parent
            hoverEnabled: true
            onClicked: root.colorClicked(root.secondColorIndex, secondColorButton)
        }
    }

    Item
    {
        visible: root.secondColorIndex >= 0
        Layout.preferredWidth: root.height
        Layout.minimumWidth: root.height
        Layout.maximumWidth: root.height
        Layout.fillHeight: true

        IconButton
        {
            objectName: "rgbColorReset_" + root.secondColorIndex
            anchors.fill: parent
            visible: root.secondResetVisible
            faSource: FontAwesome.fa_xmark
            faColor: "darkred"
            tooltip: root.secondResetTooltip
            onClicked:
            {
                secondColorButton.color = "transparent"
                root.resetClicked(root.secondColorIndex)
            }
        }
    }
}
