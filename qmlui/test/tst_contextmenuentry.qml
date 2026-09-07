import QtQuick
import QtTest
import "../qml" as QLC

TestCase
{
    id: testCase
    name: "ContextMenuEntry"
    when: windowShown
    width: 320
    height: 120
    visible: true

    QLC.ContextMenuEntry
    {
        id: menuEntry
        entryText: "Neue RGB-Matrix"
    }

    SignalSpy
    {
        id: clickSpy
        target: menuEntry
        signalName: "clicked"
    }

    function init()
    {
        clickSpy.clear()
    }

    function test_entryCanBeActivatedFromKeyboard()
    {
        verify(menuEntry.activeFocusOnTab)
        menuEntry.forceActiveFocus()
        verify(menuEntry.activeFocus)

        keyClick(Qt.Key_Space)
        compare(clickSpy.count, 1)

        keyClick(Qt.Key_Return)
        compare(clickSpy.count, 2)
    }

    function test_entireRowRespondsToMouse()
    {
        mouseClick(menuEntry, 4, menuEntry.height / 2, Qt.LeftButton)
        compare(clickSpy.count, 1)

        mouseClick(menuEntry, menuEntry.width - 4,
                   menuEntry.height / 2, Qt.LeftButton)
        compare(clickSpy.count, 2)
    }
}
