import QtQuick
import QtTest
import "../qml" as QLC

TestCase
{
    id: testCase
    name: "RGBMatrixColorRow"
    when: windowShown
    width: 420
    height: 180
    visible: true

    QLC.RGBMatrixColorRow
    {
        id: colorRow
        width: 200
        height: 28
        firstColorIndex: 2
        secondColorIndex: 3
        firstResetVisible: true
        secondResetVisible: true
        firstColor: "red"
        secondColor: "blue"
    }

    SignalSpy
    {
        id: resetSpy
        target: colorRow
        signalName: "resetClicked"
    }

    function init()
    {
        resetSpy.clear()
        colorRow.width = 200
        wait(0)
    }

    function test_rightResetRemainsInsideReducedWidth()
    {
        const rightReset = findChild(colorRow, "rgbColorReset_3")
        const firstSwatch = findChild(colorRow, "rgbColorButton_2")
        const secondSwatch = findChild(colorRow, "rgbColorButton_3")
        verify(rightReset !== null)
        verify(firstSwatch.width > 0)
        verify(secondSwatch.width > 0)
        const resetPosition = rightReset.mapToItem(colorRow, 0, 0)
        verify(resetPosition.x >= -0.01)
        verify(resetPosition.x + rightReset.width <= colorRow.width + 0.01)
    }

    function test_rightResetEmitsItsColorIndex()
    {
        const rightReset = findChild(colorRow, "rgbColorReset_3")
        verify(colorRow.visible, "row must be visible")
        verify(colorRow.secondResetVisible, "right reset flag must be true")
        verify(rightReset.parent.visible, "right reset slot must be visible")
        verify(rightReset.visible, "right reset button must be visible")
        verify(rightReset.enabled)
        verify(rightReset.width > 0)
        verify(rightReset.height > 0)
        mouseClick(rightReset, rightReset.width / 2,
                   rightReset.height / 2, Qt.LeftButton)
        compare(resetSpy.count, 1)
        compare(resetSpy.signalArguments[0][0], 3)
    }
}
