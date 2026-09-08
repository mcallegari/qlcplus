import QtQuick
import QtTest

TestCase
{
    id: testCase
    name: "ActionsMenuGeometry"

    property Component geometryComponent

    function initTestCase()
    {
        geometryComponent = Qt.createComponent("../qml/ActionsMenuGeometry.qml")
        compare(geometryComponent.status, Component.Ready,
                geometryComponent.errorString())
    }

    function test_submenuStartsAfterMainMenuAndFitsWindow()
    {
        const geometry = geometryComponent.createObject(testCase, {
            mainMenuWidth: 190,
            requestedSubmenuWidth: 900,
            windowWidth: 1024,
            popupLeft: 1,
            margin: 8
        })

        compare(geometry.submenuX, 190)
        compare(geometry.maximumSubmenuWidth, 825)
        compare(geometry.submenuWidth, 825)
        verify(geometry.popupLeft + geometry.submenuX +
               geometry.submenuWidth + geometry.margin <= geometry.windowWidth)
        geometry.destroy()
    }

    function test_normalRequestedWidthIsPreserved()
    {
        const geometry = geometryComponent.createObject(testCase, {
            mainMenuWidth: 180,
            requestedSubmenuWidth: 420,
            windowWidth: 1200,
            popupLeft: 1,
            margin: 8
        })

        compare(geometry.submenuX, 180)
        compare(geometry.submenuWidth, 420)
        geometry.destroy()
    }
}
