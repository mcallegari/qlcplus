import QtQml

QtObject
{
    property real mainMenuWidth: 0
    property real requestedSubmenuWidth: 0
    property real windowWidth: 0
    property real popupLeft: 0
    property real margin: 0

    readonly property real submenuX: Math.max(0, mainMenuWidth)
    readonly property real maximumSubmenuWidth: Math.max(
        0, windowWidth - popupLeft - submenuX - margin)
    readonly property real submenuWidth: Math.min(
        Math.max(0, requestedSubmenuWidth), maximumSubmenuWidth)
}
