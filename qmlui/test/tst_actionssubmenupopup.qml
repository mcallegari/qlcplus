import QtQuick
import QtTest
import "../qml" as QLC

TestCase
{
    id: testCase
    name: "ActionsSubmenuPopup"
    when: windowShown
    width: 640
    height: 320
    visible: true

    property string selectedAction: ""

    Item
    {
        id: narrowOwner
        width: 120
        height: 48
    }

    QLC.ActionsSubmenuPopup
    {
        id: submenuPopup
        parent: narrowOwner
        x: narrowOwner.width
        width: 260
        height: submenuEntry.height

        contentItem: QLC.ContextMenuEntry
        {
            id: submenuEntry
            objectName: "submenuEntry"
            width: submenuPopup.width
            entryText: "Server setup"
            onClicked: testCase.selectedAction = "server"
        }
    }

    function init()
    {
        selectedAction = ""
        submenuPopup.open()
        tryCompare(submenuPopup, "opened", true)
    }

    function cleanup()
    {
        submenuPopup.close()
    }

    function test_entryOutsideOwnerHitAreaIsClickable()
    {
        mouseClick(submenuEntry, submenuEntry.width / 2,
                   submenuEntry.height / 2, Qt.LeftButton)
        compare(selectedAction, "server")
    }
}
