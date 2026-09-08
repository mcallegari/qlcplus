import QtQuick
import QtTest

TestCase
{
    id: testCase
    name: "RecentFilesPopup"
    when: windowShown
    width: 800
    height: 500

    property Component popupComponent
    property var popup
    property string selectedPath: ""

    Item
    {
        id: narrowOwner
        width: 120
        height: 48
    }

    function initTestCase()
    {
        popupComponent = Qt.createComponent("../qml/RecentFilesPopup.qml")
        compare(popupComponent.status, Component.Ready,
                popupComponent.errorString())
    }

    function init()
    {
        selectedPath = ""
        popup = popupComponent.createObject(narrowOwner, {
            x: narrowOwner.width,
            y: 0,
            width: 460,
            recentFiles: [ "/tmp/short-owner/full-project-path.qxw" ]
        })
        verify(popup !== null)
        popup.fileSelected.connect(function(filePath) {
            selectedPath = filePath
        })
        popup.open()
        tryCompare(popup, "opened", true)
    }

    function cleanup()
    {
        popup.close()
        popup.destroy()
        popup = null
    }

    function test_entryOutsideOwnerHitAreaEmitsFullPath()
    {
        const entry = findChild(popup.contentItem, "recentFileEntry_0")
        verify(entry !== null)
        mouseClick(entry, entry.width / 2, entry.height / 2, Qt.LeftButton)
        compare(selectedPath, "/tmp/short-owner/full-project-path.qxw")
    }
}
