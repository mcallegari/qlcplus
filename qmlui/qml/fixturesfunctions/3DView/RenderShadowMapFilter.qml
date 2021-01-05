/*
  Q Light Controller Plus
  RenderShadowMapFilter.qml

  Copyright (c) Eric Arnebäck

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

import QtQuick 2.0

LayerFilter
{
    property Entity fixtureItem

    CameraSelector
    {
        RenderTargetSelector
        {
            id: targetSelector
            target: fixtureItem ? fixtureItem.shadowMap : null

            ClearBuffers
            {
                buffers: ClearBuffers.DepthBuffer

                RenderPassFilter
                {
                    id: geometryPass
                    matchAny: FilterKey { name: "pass"; value: { return fixtureItem.lightIntensity ? "shadows" : "invalid" } }

                    parameters: [
                        Parameter { name: "lightViewMatrix"; value: fixtureItem ? fixtureItem.lightViewMatrix : Qt.matrix4x4() },
                        Parameter { name: "lightProjectionMatrix"; value: fixtureItem ? fixtureItem.lightProjectionMatrix : Qt.matrix4x4() },
                        Parameter { name: "lightViewProjectionMatrix"; value: fixtureItem ? fixtureItem.lightViewProjectionMatrix : Qt.matrix4x4() }
                    ]
                }
            }
        }
    }
}
