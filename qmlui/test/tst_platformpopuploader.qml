import QtQuick
import QtTest

import "../qml"

TestCase
{
    id: testCase
    name: "PlatformPopupLoader"

    property bool customPopupCreated: false

    Component
    {
        id: popupFactory

        QtObject
        {
            Component.onCompleted: testCase.customPopupCreated = true
        }
    }

    PlatformPopupLoader
    {
        id: popupLoader
        sourceComponent: popupFactory
    }

    function test_linuxPopupIsNotCreatedOnNativeDialogPlatforms()
    {
        if (Qt.platform.os === "linux")
            skip("Linux intentionally uses the custom folder browser")

        compare(popupLoader.active, false)
        compare(popupLoader.status, Loader.Null)
        compare(popupLoader.item, null)
        compare(customPopupCreated, false)
    }
}
