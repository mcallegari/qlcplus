import QtQuick
import QtTest

TestCase
{
    id: testCase
    name: "PendingWorkspaceAction"

    property Component actionComponent

    function initTestCase()
    {
        actionComponent = Qt.createComponent("../qml/PendingWorkspaceAction.qml")
        compare(actionComponent.status, Component.Ready,
                actionComponent.errorString())
    }

    function test_dispatchesEverySupportedActionAndClearsIt()
    {
        const action = actionComponent.createObject(testCase)
        const events = []
        const recentPath = "/Users/example/Documents/Shows/Very Long Project.qxw"

        action.openRequested.connect(function() { events.push("open") })
        action.newRequested.connect(function() { events.push("new") })
        action.exitRequested.connect(function(discardChanges) {
            events.push("exit:" + discardChanges)
        })
        action.recentRequested.connect(function(filePath) {
            events.push("recent:" + filePath)
        })

        action.action = "#OPEN"
        action.dispatch(false)
        compare(action.action, "")

        action.action = "#NEW"
        action.dispatch(false)

        action.action = "#EXIT"
        action.dispatch(true)

        action.action = recentPath
        action.dispatch(false)

        compare(events, ["open", "new", "exit:true", "recent:" + recentPath])
        compare(action.action, "")
        action.destroy()
    }

    function test_actionSurvivesUntilSaveActuallyDispatchesIt()
    {
        const action = actionComponent.createObject(testCase)
        let openCount = 0
        action.openRequested.connect(function() { ++openCount })

        action.action = "#OPEN"
        wait(20)
        compare(action.action, "#OPEN")
        compare(openCount, 0)

        action.dispatch(false)
        compare(openCount, 1)
        compare(action.action, "")
        action.destroy()
    }

    function test_cancelClearsWithoutDispatching()
    {
        const action = actionComponent.createObject(testCase)
        let signalCount = 0
        action.openRequested.connect(function() { ++signalCount })

        action.action = "#OPEN"
        action.cancel()

        compare(action.action, "")
        compare(signalCount, 0)
        action.destroy()
    }
}
