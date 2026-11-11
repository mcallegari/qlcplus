import QtQuick
import QtTest
import "../qml" as QLC

TestCase
{
    id: testCase
    name: "SectionBoxAccessibility"
    when: windowShown
    width: 320
    height: 160
    visible: true

    QLC.SectionBox
    {
        id: sectionBox
        width: 280
        sectionLabel: "Rendering"
        isExpanded: false
        sectionContents: Rectangle
        {
            width: sectionBox.width
            height: 80
        }
    }

    function test_headerCanBeActivatedFromKeyboard()
    {
        const header = findChild(sectionBox, "sectionHeader")
        verify(header !== null)
        verify(header.activeFocusOnTab)

        header.forceActiveFocus()
        verify(header.activeFocus)
        keyClick(Qt.Key_Space)
        verify(sectionBox.isExpanded)

        keyClick(Qt.Key_Return)
        verify(!sectionBox.isExpanded)
    }
}
