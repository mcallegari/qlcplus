/*
  Q Light Controller Plus
  StageRockRhi.qml
*/

import QtQuick
import RhiQmlItem 1.0

Item {
    id: stage

    required property vector3d envMeters
    property bool active: false

    // Keep the same naming/logic as the original Qt3D stage preset.
    property vector3d sizeMeters: Qt.vector3d(Math.max(0.2, envMeters.x),
                                              Math.max(0.2, envMeters.y),
                                              Math.max(0.2, envMeters.z))

    // If the model changes, this has to be changed accordingly.
    property real trussHalfSize: 0.15

    property string truss2mPath: View3D.meshDirectory + "stage/truss_square_2m.glb"
    property string truss1mPath: View3D.meshDirectory + "stage/truss_square_1m.glb"
    property string cornerPath: View3D.meshDirectory + "stage/truss_square_corner.glb"
    property bool cornerModelsPrimed: active
    property var dynamicTrusses: []

    visible: active

    Component.onCompleted: rebuildTrusses()
    onActiveChanged: {
        if (active)
            cornerModelsPrimed = true
        rebuildTrusses()
    }
    onSizeMetersChanged: rebuildTrusses()

    function resetDynamicTrusses() {
        for (var i = 0; i < dynamicTrusses.length; ++i) {
            var mesh = dynamicTrusses[i]
            if (mesh)
                mesh.destroy()
        }
        dynamicTrusses = []
    }

    function defaultColumnPositions() {
        return [ Qt.vector3d(-sizeMeters.x / 2 - trussHalfSize, sizeMeters.y + trussHalfSize, sizeMeters.z / 2 + trussHalfSize),
                 Qt.vector3d(sizeMeters.x / 2 + trussHalfSize, sizeMeters.y + trussHalfSize, sizeMeters.z / 2 + trussHalfSize),
                 Qt.vector3d(-sizeMeters.x / 2 - trussHalfSize, sizeMeters.y + trussHalfSize, -sizeMeters.z / 2 - trussHalfSize),
                 Qt.vector3d(sizeMeters.x / 2 + trussHalfSize, sizeMeters.y + trussHalfSize, -sizeMeters.z / 2 - trussHalfSize) ]
    }

    function createMesh(path, pos, rot) {
        var mesh = trussComp.createObject(stage, {
                                              path: path,
                                              position: pos,
                                              rotationDegrees: rot,
                                              selectable: false,
                                              visible: stage.active
                                          })
        if (mesh)
            dynamicTrusses.push(mesh)
        return mesh
    }

    function rebuildTrusses() {
        resetDynamicTrusses()

        if (!active)
            return
        if (sizeMeters.x <= 0 || sizeMeters.y <= 0 || sizeMeters.z <= 0)
            return

        var columnsArray = defaultColumnPositions()

        var i, len, vecPos

        // Create the four truss columns.
        for (i = 0; i < 4; i++) {
            len = sizeMeters.y
            vecPos = columnsArray[i]

            while (len > 0) {
                if (len >= 2) {
                    vecPos.y = sizeMeters.y - len + 1
                    createMesh(truss2mPath, vecPos, Qt.vector3d(90, 0, 0))
                    len -= 2
                } else {
                    vecPos.y = sizeMeters.y - len + 0.5
                    createMesh(truss1mPath, vecPos, Qt.vector3d(90, 0, 0))
                    len -= 1
                }
            }
        }

        columnsArray = defaultColumnPositions()

        // Create upper side trusses.
        for (i = 0; i < 2; i++) {
            len = sizeMeters.z
            vecPos = columnsArray[i]

            while (len > 0) {
                if (len >= 2) {
                    vecPos.z = (sizeMeters.z / 2) - len + 1
                    createMesh(truss2mPath, vecPos, Qt.vector3d(0, 0, 0))
                    len -= 2
                } else {
                    vecPos.z = (sizeMeters.z / 2) - len + 0.5
                    createMesh(truss1mPath, vecPos, Qt.vector3d(0, 0, 0))
                    len -= 1
                }
            }
        }

        columnsArray = defaultColumnPositions()

        // Create upper front/rear trusses.
        for (i = 0; i < 2; i++) {
            len = sizeMeters.x
            vecPos = columnsArray[i * 2]

            while (len > 0) {
                if (len >= 2) {
                    vecPos.x = (sizeMeters.x / 2) - len + 1
                    createMesh(truss2mPath, vecPos, Qt.vector3d(0, 90, 0))
                    len -= 2
                } else {
                    vecPos.x = (sizeMeters.x / 2) - len + 0.5
                    createMesh(truss1mPath, vecPos, Qt.vector3d(0, 90, 0))
                    len -= 1
                }
            }
        }
    }

    Cube {
        selectable: false
        visible: stage.active
        position: Qt.vector3d(0.0, -0.11, 0.0)
        scale: Qt.vector3d(sizeMeters.x, 0.2, sizeMeters.z)
        baseColor: Qt.vector3d(0.70, 0.70, 0.70)
        roughness: 0.92
        metalness: 0.02
    }

    Model {
        selectable: false
        visible: stage.active
        path: (stage.active || stage.cornerModelsPrimed) ? stage.cornerPath : ""
        position: Qt.vector3d(-sizeMeters.x / 2 - trussHalfSize,
                              sizeMeters.y + trussHalfSize,
                              sizeMeters.z / 2 + trussHalfSize)
    }

    Model {
        selectable: false
        visible: stage.active
        path: (stage.active || stage.cornerModelsPrimed) ? stage.cornerPath : ""
        position: Qt.vector3d(sizeMeters.x / 2 + trussHalfSize,
                              sizeMeters.y + trussHalfSize,
                              sizeMeters.z / 2 + trussHalfSize)
    }

    Model {
        selectable: false
        visible: stage.active
        path: (stage.active || stage.cornerModelsPrimed) ? stage.cornerPath : ""
        position: Qt.vector3d(-sizeMeters.x / 2 - trussHalfSize,
                              sizeMeters.y + trussHalfSize,
                              -sizeMeters.z / 2 - trussHalfSize)
    }

    Model {
        selectable: false
        visible: stage.active
        path: (stage.active || stage.cornerModelsPrimed) ? stage.cornerPath : ""
        position: Qt.vector3d(sizeMeters.x / 2 + trussHalfSize,
                              sizeMeters.y + trussHalfSize,
                              -sizeMeters.z / 2 - trussHalfSize)
    }

    Component {
        id: trussComp
        Model {
            property bool isDynamic: true
        }
    }
}
