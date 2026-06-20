/*
  Q Light Controller Plus
  Position3DMarker.qml

  Copyright (c) Massimo Callegari

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0.txt

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
*/

import Qt3D.Core
import Qt3D.Render
import Qt3D.Extras

Entity
{
    id: markerEntity

    property Effect geometryPassEffect
    property Layer selectionLayer

    property vector3d extents: Qt.vector3d(0.18, 0.18, 0.18)
    property vector3d center: Qt.vector3d(0, 0, 0)
    property vector4d color: Qt.vector4d(0.55, 0.0, 0.0, 2.0)

    property bool isSelected: true

    SphereMesh
    {
        id: markerMesh
        radius: markerEntity.extents.x / 2
    }

    Material
    {
        id: markerMaterial
        effect: geometryPassEffect

        parameters: [
            Parameter { name: "diffuse"; value: color },
            Parameter { name: "specular"; value: Qt.color("black") },
            Parameter { name: "shininess"; value: 1.0 },
            Parameter { name: "bloom"; value: 0 }
        ]
    }

    Transform
    {
        id: markerTransform
        translation: markerEntity.center
    }

    components: [
        markerMesh,
        markerMaterial,
        markerTransform,
        isSelected ? selectionLayer : null
    ]
}
