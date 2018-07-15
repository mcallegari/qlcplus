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

CameraSelector
{
    property alias sceneDeferredLayer: sceneDeferredLayerFilter.layers
    property alias depthTargetSelector: targetSelector.target
    property Fixture3DItem fixtureItem

    LayerFilter
    {
        id: sceneDeferredLayerFilter

        RenderTargetSelector
        {
            id: targetSelector

            ClearBuffers
            {
                buffers: ClearBuffers.DepthBuffer

                RenderPassFilter
                {
                    parameters: [
                        Parameter { name: "lightViewMatrix"; value: fixtureItem ? fixtureItem.lightViewMatrix : null },
                        Parameter { name: "lightProjectionMatrix"; value: fixtureItem ? fixtureItem.lightProjectionMatrix : null },
                        Parameter { name: "lightViewProjectionMatrix"; value: fixtureItem ? fixtureItem.lightViewProjectionMatrix : null }
                    ]

                    id: geometryPass
                    matchAny: FilterKey { name: "pass"; value: "shadows" }
                }
            }
        }
    }
}
