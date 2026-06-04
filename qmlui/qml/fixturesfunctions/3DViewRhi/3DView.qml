/*
  Q Light Controller Plus
  3DView.qml (RHI backend)
*/

import QtQuick
import QtQuick.Controls
import RhiQmlItem 1.0
import org.qlcplus.classes 1.0
import ".."

Rectangle {
    id: root
    anchors.fill: parent
    color: "black"
    clip: true

    property string contextName: "3D"
    property alias contextItem: renderer
    property real unitScale: (View2D && View2D.gridUnits === MonitorProperties.Feet) ? 0.3048 : 1.0
    property vector3d envMeters: {
        var env = contextManager ? contextManager.environmentSize : Qt.vector3d(5, 3, 5)
        return Qt.vector3d(env.x * unitScale, env.y * unitScale, env.z * unitScale)
    }

    function hasSettings() { return true }
    function showSettings(show) { settingsPanel.visible = show }
    function setZoom(amount) { renderer.zoomAlongView(-amount) }

    Component.onCompleted: {
        renderer.cameraPosition = View3D.cameraPosition
        renderer.cameraTarget = View3D.cameraViewCenter
        if (contextManager)
            contextManager.enableContext("3D", true, renderer)
    }

    Component.onDestruction: {
        if (contextManager)
            contextManager.enableContext("3D", false, renderer)
    }

    RhiQmlItem {
        id: renderer
        objectName: "sceneRenderer"
        anchors.fill: parent
        focus: true

        freeCameraEnabled: true
        positionPicking: contextManager ? contextManager.positionPicking : false
        beamModel: Scene.SoftHaze
        volumetricEnabled: true
        shadowsEnabled: true
        smokeNoiseEnabled: true
        bloomIntensity: 2.0
        bloomRadius: 2.0

        ambientLight: Qt.vector3d(1.0, 1.0, 1.0)
        ambientIntensity: View3D.ambientIntensity
        smokeAmount: View3D.smokeAmount

        Camera {
            position: renderer.cameraPosition
            target: renderer.cameraTarget
            fov: 60
            nearPlane: 0.01
            farPlane: 500
        }

        Light {
            type: Light.Directional
            direction: Qt.vector3d(0.2, -1.0, -0.3)
            color: Qt.vector3d(1.0, 1.0, 1.0)
            intensity: View3D.ambientIntensity
            castShadows: true
        }

        StageSimpleRhi {
            envMeters: root.envMeters
            active: View3D.stageIndex === 0
        }

        StageBoxRhi {
            envMeters: root.envMeters
            active: View3D.stageIndex === 1
        }

        StageRockRhi {
            envMeters: root.envMeters
            active: View3D.stageIndex === 2
        }

        StageTheatreRhi {
            envMeters: root.envMeters
            active: View3D.stageIndex === 3
        }

        WheelHandler {
            target: renderer
            onWheel: function(wheel) {
                var step = wheel.angleDelta.y / 120.0
                var dx = renderer.cameraTarget.x - renderer.cameraPosition.x
                var dy = renderer.cameraTarget.y - renderer.cameraPosition.y
                var dz = renderer.cameraTarget.z - renderer.cameraPosition.z
                var dist = Math.sqrt(dx * dx + dy * dy + dz * dz)
                dist = Math.max(dist, 0.5)
                var factor = Math.pow(1.12, step)
                renderer.zoomAlongView((factor - 1.0) * dist)
            }
        }

        onCameraPositionChanged: View3D.cameraPosition = cameraPosition
        onCameraTargetChanged: View3D.cameraViewCenter = cameraTarget

        onMeshPicked: function(item, worldPos, hit, modifiers) {
            if (contextManager && contextManager.positionPicking && hit)
                contextManager.setPositionPickPoint(worldPos)

            if (!contextManager)
            {
                handlePick(item, hit, modifiers)
                return
            }

            var multi = (modifiers & Qt.ShiftModifier) !== 0
            if (hit && item && item.itemID !== undefined && item.selectable !== false)
            {
                var itemId = Number(View3D.sceneItemMonitorID(item))
                var isSelected = (item.isSelected === true)
                var select = !isSelected
                var isGeneric = View3D.isGenericSceneItem(item)
                var isFixture = View3D.isFixtureSceneItem(item)
                var domain = isGeneric ? "generic" : (isFixture ? "fixture" : "")
                console.log("[RHI pick]", "itemId:", itemId, "isSelected:", isSelected,
                            "rawItemID:", item.itemID,
                            "resolvedMonitorItemID:", itemId,
                            "select:", select, "domain:", domain,
                            "modifiers:", modifiers, "multi:", multi)

                if (isGeneric && itemId >= 0)
                {
                    if (select)
                        contextManager.resetFixtureSelection()
                    View3D.setItemSelection(itemId, select, modifiers)
                }
                else if (isFixture && itemId >= 0)
                {
                    if (select)
                        View3D.setItemSelection(-1, false, modifiers)
                    contextManager.setItemSelection(itemId, select, modifiers)
                }
                else
                    console.warn("[RHI pick] unknown selection domain for item", item)
            }
        }
    }

    Rectangle {
        visible: View3D.frameCountEnabled
        z: 4
        opacity: 0.6
        color: UISettings.bgMedium
        width: height
        height: UISettings.bigItemHeight

        Column {
            RobotoText { height: UISettings.bigItemHeight / 4; label: "FPS: " + View3D.FPS }
            RobotoText { height: UISettings.bigItemHeight / 4; label: "Min: " + View3D.minFPS }
            RobotoText { height: UISettings.bigItemHeight / 4; label: "Max: " + View3D.maxFPS }
            RobotoText { height: UISettings.bigItemHeight / 4; label: "Avg: " + View3D.avgFPS }
        }
    }

    ViewGizmo {
        id: viewGizmo
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.margins: 8
        renderer: renderer
        z: 6
    }

    SettingsView3DRhi {
        id: settingsPanel
        visible: false
        x: parent.width - width
        z: 5
    }
}
