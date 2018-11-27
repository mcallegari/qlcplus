/*
  Q Light Controller Plus
  ScreenQuadEntity.qml

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

import QtQuick 2.7

import Qt3D.Core 2.0
import Qt3D.Render 2.0
import Qt3D.Extras 2.0

Entity
{
    objectName: "quadEntity"

    readonly property Layer layer: screenQuadLayer

    Entity
    {
        components : [
            Layer { id: screenQuadLayer },

            PlaneMesh
            {
                width: 2.0
                height: 2.0
                meshResolution: Qt.size(2, 2)
            },

            Transform
            {
                // We rotate the plane so that it faces us
                rotation: fromAxisAndAngle(Qt.vector3d(1, 0, 0), 90)
            },

            Material
            {
                Component.onCompleted: View3D.quadReady()
                effect: LightPassEffect { }
            }
        ]
    }
}
