/*
  Q Light Controller Plus
  StageTheatreRhi.qml
*/

import QtQuick
import RhiQmlItem 1.0

Item {
    id: stage

    required property vector3d envMeters
    property bool active: false

    property real sideSpace: 3.0
    property real frontSpace: 2.0
    property real trussHalfSize: 0.15
    property real trussDistance: 2.0

    property real dimX: Math.max(0.2, envMeters.x)
    property real dimZ: Math.max(0.2, envMeters.z)
    property var dynamicTrusses: []

    property string truss2mPath: View3D.meshDirectory + "stage/truss_square_2m.glb"
    property string truss1mPath: View3D.meshDirectory + "stage/truss_square_1m.glb"

    visible: active

    Component.onCompleted: rebuildTrusses()
    onActiveChanged: rebuildTrusses()
    onEnvMetersChanged: rebuildTrusses()

    function resetDynamicTrusses() {
        for (var i = 0; i < dynamicTrusses.length; ++i) {
            var mesh = dynamicTrusses[i]
            if (mesh)
                mesh.destroy()
        }
        dynamicTrusses = []
    }

    function trussPath(lengthMeters) {
        return lengthMeters >= 2 ? truss2mPath : truss1mPath
    }

    function createTruss(path, pos, rot, lenScale) {
        var mesh = trussComp.createObject(stage, {
                                             path: path,
                                             position: pos,
                                             rotationDegrees: rot,
                                             scale: Qt.vector3d(1.0, 1.0, lenScale),
                                             selectable: false,
                                             visible: stage.active
                                         })
        if (mesh)
            dynamicTrusses.push(mesh)
        return mesh
    }

    function buildRow(zPos) {
        var remaining = dimX
        var cursor = -dimX * 0.5
        while (remaining > 0.01) {
            var seg = remaining >= 2.0 ? 2.0 : (remaining >= 1.0 ? 1.0 : remaining)
            var scaleLen = seg >= 2.0 ? (seg / 2.0) : seg
            var xPos = cursor + seg * 0.5
            createTruss(trussPath(seg), Qt.vector3d(xPos, envMeters.y + trussHalfSize, zPos), Qt.vector3d(0, 90, 0), scaleLen)
            cursor += seg
            remaining -= seg
        }
    }

    function rebuildTrusses() {
        resetDynamicTrusses()
        if (!active)
            return

        var maxRows = Math.max(1, Math.ceil(dimZ / trussDistance))
        var zPos = dimZ * 0.5 - trussHalfSize
        for (var i = 0; i < maxRows; ++i) {
            buildRow(zPos)
            zPos -= trussDistance
        }
    }

    Cube {
        selectable: false
        visible: stage.active
        position: Qt.vector3d(0.0, -0.11, 0.0)
        scale: Qt.vector3d(stage.dimX + (stage.sideSpace * 2.0), 0.2, stage.dimZ + stage.frontSpace)
        baseColor: Qt.vector3d(0.68, 0.68, 0.68)
        roughness: 0.92
        metalness: 0.02
    }

    Cube {
        selectable: false
        visible: stage.active
        position: Qt.vector3d(0.0, stage.envMeters.y * 0.5, -stage.dimZ * 0.5)
        scale: Qt.vector3d(stage.dimX + (stage.sideSpace * 2.0) + 0.2, Math.max(0.2, stage.envMeters.y), 0.2)
        baseColor: Qt.vector3d(0.52, 0.52, 0.52)
        roughness: 0.95
        metalness: 0.0
    }

    Cube {
        selectable: false
        visible: stage.active
        position: Qt.vector3d(-(stage.dimX + stage.sideSpace * 2.0) * 0.5, stage.envMeters.y * 0.5, 0.0)
        scale: Qt.vector3d(0.2, Math.max(0.2, stage.envMeters.y), stage.dimZ)
        baseColor: Qt.vector3d(0.52, 0.52, 0.52)
        roughness: 0.95
        metalness: 0.0
    }

    Cube {
        selectable: false
        visible: stage.active
        position: Qt.vector3d((stage.dimX + stage.sideSpace * 2.0) * 0.5, stage.envMeters.y * 0.5, 0.0)
        scale: Qt.vector3d(0.2, Math.max(0.2, stage.envMeters.y), stage.dimZ)
        baseColor: Qt.vector3d(0.52, 0.52, 0.52)
        roughness: 0.95
        metalness: 0.0
    }

    Cube {
        selectable: false
        visible: stage.active
        position: Qt.vector3d(-(stage.dimX + stage.sideSpace) * 0.5, stage.envMeters.y * 0.5, stage.dimZ * 0.5)
        scale: Qt.vector3d(stage.sideSpace, Math.max(0.2, stage.envMeters.y), 0.2)
        baseColor: Qt.vector3d(0.52, 0.52, 0.52)
        roughness: 0.95
        metalness: 0.0
    }

    Cube {
        selectable: false
        visible: stage.active
        position: Qt.vector3d((stage.dimX + stage.sideSpace) * 0.5, stage.envMeters.y * 0.5, stage.dimZ * 0.5)
        scale: Qt.vector3d(stage.sideSpace, Math.max(0.2, stage.envMeters.y), 0.2)
        baseColor: Qt.vector3d(0.52, 0.52, 0.52)
        roughness: 0.95
        metalness: 0.0
    }

    Component {
        id: trussComp
        Model {
            property bool isDynamic: true
        }
    }
}
