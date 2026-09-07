import QtQuick
import QtTest
import "../qml/inputoutput" as QLC

TestCase
{
    id: testCase
    name: "PluginsList"
    when: windowShown
    width: 420
    height: 320

    property string configuredPlugin: ""

    property QtObject ioManager: QtObject
    {
        signal ioSourcesChanged()

        property var outputSources: [
            {
                universe: 0,
                name: "127.0.0.1",
                line: 0,
                plugin: "ArtNet",
                available: true,
                canConfigure: false,
                status: ""
            },
            {
                universe: 0,
                name: "DMX USB",
                line: -1,
                plugin: "DMX USB",
                available: false,
                canConfigure: true,
                status: "No available output lines"
            }
        ]

        function universeInputSources(universe)
        {
            return []
        }

        function universeOutputSources(universe)
        {
            return outputSources
        }

        function configurePluginByName(pluginName)
        {
            testCase.configuredPlugin = pluginName
        }
    }

    QLC.PluginsList
    {
        id: pluginsList
        width: parent.width
        height: parent.height
    }

    function init()
    {
        configuredPlugin = ""
        ioManager.outputSources = [
            {
                universe: 0,
                name: "127.0.0.1",
                line: 0,
                plugin: "ArtNet",
                available: true,
                canConfigure: false,
                status: ""
            },
            {
                universe: 0,
                name: "DMX USB",
                line: -1,
                plugin: "DMX USB",
                available: false,
                canConfigure: true,
                status: "No available output lines"
            }
        ]
        pluginsList.loadSources(false)
        wait(0)
    }

    function test_unavailablePluginStaysVisibleAndConfigurable()
    {
        compare(pluginsList.sourcesCount, 2)

        const dmxUsbItem = findChild(pluginsList, "pluginSource_1")
        verify(dmxUsbItem !== null)
        compare(dmxUsbItem.pluginName, "DMX USB")
        compare(dmxUsbItem.sourceAvailable, false)

        pluginsList.activateSource(dmxUsbItem.pluginName,
                                   dmxUsbItem.sourceAvailable,
                                   dmxUsbItem.canConfigure)
        compare(configuredPlugin, "DMX USB")
    }

    function test_configurationChangeRefreshesOpenList()
    {
        ioManager.outputSources = [
            {
                universe: 0,
                name: "127.0.0.1",
                line: 0,
                plugin: "ArtNet",
                available: true,
                canConfigure: false,
                status: ""
            },
            {
                universe: 0,
                name: "FT232R USB UART (S/N: AG0KM73D)",
                line: 0,
                plugin: "DMX USB",
                available: true,
                canConfigure: true,
                status: ""
            }
        ]

        ioManager.ioSourcesChanged()
        tryCompare(pluginsList, "sourcesCount", 2)

        const dmxUsbItem = findChild(pluginsList, "pluginSource_1")
        verify(dmxUsbItem !== null)
        compare(dmxUsbItem.sourceAvailable, true)
        compare(dmxUsbItem.lineName, "FT232R USB UART (S/N: AG0KM73D)")
    }

    function test_loadedPluginsWithoutOutputLinesStayInDeviceList()
    {
        const missingPlugins = [ "DMX USB", "HID", "MIDI", "Peperoni", "uDMX" ]
        const sources = [
            {
                universe: 0,
                name: "127.0.0.1",
                line: 0,
                plugin: "ArtNet",
                available: true,
                canConfigure: false,
                status: ""
            }
        ]

        for (let i = 0; i < missingPlugins.length; i++)
        {
            sources.push({
                universe: 0,
                name: missingPlugins[i],
                line: -1,
                plugin: missingPlugins[i],
                available: false,
                canConfigure: i === 0,
                status: "No output lines detected"
            })
        }

        ioManager.outputSources = sources
        ioManager.ioSourcesChanged()
        tryCompare(pluginsList, "sourcesCount", 6)

        for (let i = 0; i < missingPlugins.length; i++)
        {
            const item = findChild(pluginsList, "pluginSource_" + (i + 1))
            verify(item !== null)
            compare(item.pluginName, missingPlugins[i])
            compare(item.sourceAvailable, false)
        }
    }
}
