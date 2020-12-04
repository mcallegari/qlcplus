/*
  Q Light Controller Plus
  SceneEntity.qml

  Copyright (c) Massimo Callegari, Eric Arnebäck

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

import QtQuick 2.0

import Qt3D.Core 2.0
import Qt3D.Render 2.0
import Qt3D.Input 2.0
import Qt3D.Extras 2.0

Entity
{
    id: sceneRootEntity
    objectName: "sceneRootEntity"

    property size viewSize
    readonly property Layer deferredLayer: Layer { objectName: "sceneDeferredLayer" }
    readonly property Layer selectionLayer: Layer { objectName: "selectionLayer" }
    readonly property GeometryRenderer selectionMesh: SelectionGeometry { }
    readonly property Effect geometryPassEffect: GeometryPassEffect { }
    readonly property Effect spotlightShadingEffect: SpotlightShadingEffect { }
    readonly property Effect spotlightScatteringEffect: SpotlightScatteringEffect { }
    readonly property Effect outputFrontDepthEffect: OutputFrontDepthEffect { }

    property ConeMesh coneMesh:
        ConeMesh
        {
            length: 1.0

            bottomRadius: 1.0
            topRadius: 1.0

            rings: 2
            slices: 60
        }
}
