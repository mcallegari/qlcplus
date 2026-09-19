import QtQuick
import QtTest

TestCase
{
    id: testCase
    name: "RobotoText"

    function test_longLabelCanBeMiddleElidedWithoutChangingItsValue()
    {
        const component = Qt.createComponent("../qml/RobotoText.qml")
        compare(component.status, Component.Ready, component.errorString())

        const fullPath = "/Users/example/Documents/very/long/path/to/a/show/project.qxw"
        const label = component.createObject(testCase, {
            label: fullPath,
            maximumWidth: 120,
            textElide: Text.ElideMiddle
        })

        verify(label !== null, component.errorString())
        compare(label.label, fullPath)
        compare(label.width, 120)
        verify(label.naturalWidth > label.width)
        compare(label.textElide, Text.ElideMiddle)
        label.destroy()
    }
}
