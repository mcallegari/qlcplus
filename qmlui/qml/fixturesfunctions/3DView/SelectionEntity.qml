/*
  Q Light Controller Plus
  SelectionEntity.qml

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

import Qt3D.Core 2.0
import Qt3D.Render 2.0

Entity
{
    id: selectionEntity

    property Effect geometryPassEffect
    property Layer selectionLayer
    property GeometryRenderer selectionMesh

    property vector3d extents: Qt.vector3d(0.5, 0.5, 0.5)
    property vector3d center: Qt.vector3d(0, 0, 0)
    property vector4d color: Qt.vector4d(1.0, 1.0, 0.0, 2.0)
    property vector3d modScale: Qt.vector3d(1.0, 1.0, 1.0)

    property bool isSelected: false

    function bindItemTransform(itemID, t)
    {
        isSelected = contextManager.isFixtureSelected(itemID)
        selectionTransform.translation = Qt.binding(function() { return center.plus(t.translation) })
        selectionTransform.rotation = Qt.binding(function() { return t.rotation })
    }

    Material
    {
        id: selectionMaterial
        effect: geometryPassEffect

        parameters: [
            Parameter { name: "diffuse"; value: color },
            Parameter { name: "specular"; value: "black" },
            Parameter { name: "shininess"; value: 1.0 },
            Parameter { name: "bloom"; value: 0 }
        ]
    }

    Transform
    {
        id: selectionTransform
        //translation: center
        scale3D: Qt.vector3d((extents.x + (extents.x * 0.05)) * modScale.x,
                             (extents.y + (extents.y * 0.05)) * modScale.y,
                             (extents.z + (extents.z * 0.05)) * modScale.z)
    }

    components: [
        selectionMesh,
        selectionMaterial,
        selectionTransform,
        isSelected ? selectionLayer : null
    ]
}
