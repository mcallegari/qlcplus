.import "../../js/GenericHelpers.js" as SharedHelpers

// Source-tree bridge for QML tests. The application resource collection keeps
// GenericHelpers.js and PluginDragItem.qml together at the qrc root.
function pluginIconFromName(pluginName)
{
    return SharedHelpers.pluginIconFromName(pluginName)
}
