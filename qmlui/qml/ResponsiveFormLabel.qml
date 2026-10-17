import QtQuick
import QtQuick.Layouts
import "."

RobotoText
{
    property real availableWidth: 0
    property real reservedControlWidth: 0
    property real layoutSpacing: 0
    readonly property real allowedWidth:
        Math.max(0, availableWidth - reservedControlWidth - layoutSpacing)

    maximumWidth: allowedWidth
    textElide: Text.ElideRight
    Layout.minimumWidth: 0
    Layout.maximumWidth: allowedWidth
}
