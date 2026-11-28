import QtQuick
import QtQuick.Layouts
import QtTest
import "../qml" as QLC

TestCase
{
    id: testCase
    name: "ResponsiveFormLabel"
    when: windowShown
    width: 420
    height: 160

    Item
    {
        id: owner
        width: 240
        height: 40

        GridLayout
        {
            id: renderingGrid
            width: owner.width
            columns: 2
            columnSpacing: 5

            QLC.ResponsiveFormLabel
            {
                id: fpsLabel
                label: "Frames pro Sekunde anzeigen"
                height: 28
                availableWidth: renderingGrid.width
                reservedControlWidth: 90
                layoutSpacing: renderingGrid.columnSpacing
            }

            Rectangle
            {
                id: renderingControl
                height: 28
                Layout.fillWidth: true
                Layout.minimumWidth: 90
            }
        }
    }

    function test_longLabelPreservesControlAndStaysInsideGrid()
    {
        wait(0)
        verify(fpsLabel.naturalWidth > fpsLabel.width)
        verify(fpsLabel.width > 0)
        fuzzyCompare(fpsLabel.width,
                     Math.min(fpsLabel.naturalWidth, fpsLabel.allowedWidth),
                     0.01)
        verify(fpsLabel.width <= fpsLabel.allowedWidth)
        verify(renderingControl.x >= fpsLabel.x + fpsLabel.width +
               renderingGrid.columnSpacing - 0.01)
        verify(renderingControl.width >= 90)
        verify(renderingControl.x + renderingControl.width <=
               renderingGrid.width + 0.01)
    }
}
