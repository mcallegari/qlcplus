import QtQml

QtObject
{
    property string action: ""
    readonly property bool hasAction: action.length > 0

    signal openRequested()
    signal newRequested()
    signal exitRequested(bool discardChanges)
    signal recentRequested(string filePath)

    function dispatch(discardChanges)
    {
        const pendingAction = action
        action = ""

        if (pendingAction === "#OPEN")
            openRequested()
        else if (pendingAction === "#NEW")
            newRequested()
        else if (pendingAction === "#EXIT")
            exitRequested(discardChanges)
        else if (pendingAction.length > 0)
            recentRequested(pendingAction)
    }

    function cancel()
    {
        action = ""
    }
}
