import QtQuick 2.0

Rectangle {
    width: textBox.width + 10
    height: 40

    color: "transparent"

    property string label
    property color labelColor: "white"
    property int fontSize: 16
    property bool fontBold: false
    property bool wrapText: false

    Text {
        id: textBox
        x: 5
        width: wrapText ? parent.width : Text.paintedWidth
        anchors.verticalCenter: parent.verticalCenter
        text: label
        font.family: "RobotoCondensed"
        font.pointSize: fontSize
        font.bold: fontBold
        color: labelColor
        wrapMode: wrapText ? Text.Wrap : Text.NoWrap
    }
}

