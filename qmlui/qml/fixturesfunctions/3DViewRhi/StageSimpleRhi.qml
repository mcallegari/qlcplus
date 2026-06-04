/*
  Q Light Controller Plus
  StageSimpleRhi.qml
*/

import QtQuick
import RhiQmlItem 1.0

Item {
    id: stage

    required property vector3d envMeters
    property bool active: true

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
}
