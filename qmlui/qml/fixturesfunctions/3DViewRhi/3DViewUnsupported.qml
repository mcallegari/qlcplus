import QtQuick
import ".."

Rectangle {
    anchors.fill: parent
    color: UISettings.bgMedium

    function hasSettings() { return false }

    RobotoText {
        height: UISettings.bigItemHeight
        anchors.centerIn: parent
        textHAlign: Text.AlignHCenter
        label: "3D View (RHI backend) unavailable on this build/runtime."
    }
}
