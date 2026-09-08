import QtQml

QtObject
{
    id: coordinator

    property bool pending: false

    signal dismissRequested()
    signal actionRequested()

    function trigger()
    {
        pending = true
        dismissRequested()
    }

    function completeDismissal()
    {
        if (!pending)
            return

        pending = false
        actionRequested()
    }
}
