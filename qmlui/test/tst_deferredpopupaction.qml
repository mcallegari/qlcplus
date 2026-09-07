import QtQuick
import QtQuick.Controls
import QtTest

TestCase
{
    id: testCase
    name: "DeferredPopupAction"

    property Component actionComponent
    property var events: []
    property int eventCount: 0

    Component
    {
        id: popupComponent

        Popup
        {
            exit: Transition
            {
                NumberAnimation
                {
                    property: "opacity"
                    from: 1
                    to: 0
                    duration: 80
                }
            }
        }
    }

    function initTestCase()
    {
        actionComponent = Qt.createComponent("../qml/DeferredPopupAction.qml")
        compare(actionComponent.status, Component.Ready,
                actionComponent.errorString())
    }

    function test_runsActionOnlyAfterPopupClosed()
    {
        events = []
        eventCount = 0
        const action = actionComponent.createObject(testCase)
        const popup = popupComponent.createObject(testCase)
        action.dismissRequested.connect(function() {
            events.push("dismiss")
            ++eventCount
            popup.close()
        })
        action.actionRequested.connect(function() {
            events.push("action")
            ++eventCount
        })
        popup.closed.connect(function() {
            events.push("closed")
            ++eventCount
            action.completeDismissal()
        })

        popup.open()
        tryCompare(popup, "opened", true)

        action.trigger()
        compare(eventCount, 1)
        compare(events[0], "dismiss")

        tryCompare(testCase, "eventCount", 3)
        compare(events[1], "closed")
        compare(events[2], "action")

        action.completeDismissal()
        compare(eventCount, 3)
        popup.destroy()
        action.destroy()
    }
}
