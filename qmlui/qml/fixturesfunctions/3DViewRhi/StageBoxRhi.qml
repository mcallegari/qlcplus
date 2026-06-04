/*
  Q Light Controller Plus
  StageBoxRhi.qml
*/

import QtQuick
import RhiQmlItem 1.0

Item {
    id: stage

    required property vector3d envMeters
    property bool active: false

    visible: active

    Cube {
        selectable: false
        visible: stage.active
        position: Qt.vector3d(0.0, -0.11, 0.0)
        scale: Qt.vector3d(Math.max(0.2, stage.envMeters.x), 0.2, Math.max(0.2, stage.envMeters.z))
        baseColor: Qt.vector3d(0.72, 0.72, 0.72)
        roughness: 0.95
        metalness: 0.0
    }

    Cube {
        selectable: false
        visible: stage.active
        position: Qt.vector3d(0.0, Math.max(0.1, stage.envMeters.y * 0.5), -stage.envMeters.z * 0.5)
        scale: Qt.vector3d(stage.envMeters.x + 0.2, Math.max(0.2, stage.envMeters.y), 0.2)
        baseColor: Qt.vector3d(0.58, 0.58, 0.58)
        roughness: 0.95
        metalness: 0.0
    }

    Cube {
        selectable: false
        visible: stage.active
        position: Qt.vector3d(-stage.envMeters.x * 0.5, Math.max(0.1, stage.envMeters.y * 0.5), 0.0)
        scale: Qt.vector3d(0.2, Math.max(0.2, stage.envMeters.y), stage.envMeters.z)
        baseColor: Qt.vector3d(0.58, 0.58, 0.58)
        roughness: 0.95
        metalness: 0.0
    }

    Cube {
        selectable: false
        visible: stage.active
        position: Qt.vector3d(stage.envMeters.x * 0.5, Math.max(0.1, stage.envMeters.y * 0.5), 0.0)
        scale: Qt.vector3d(0.2, Math.max(0.2, stage.envMeters.y), stage.envMeters.z)
        baseColor: Qt.vector3d(0.58, 0.58, 0.58)
        roughness: 0.95
        metalness: 0.0
    }
}
