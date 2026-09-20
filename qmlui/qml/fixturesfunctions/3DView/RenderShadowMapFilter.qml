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

import QtQuick
import Qt3D.Core
import Qt3D.Render

LayerFilter
{
    property Entity fixtureItem

    /* A dark emitter has nothing to cast a shadow of, and disabling the filter
       here drops the whole branch below it - the render target bind and the
       depth clear go with it. Matching an invalid pass name, which is what this
       used to do, kept the scene from being drawn but still left Qt3D building
       a render view for the leaf, so every dark emitter in the project cost a
       render target switch and a 1024x1024 depth clear on every frame. That was
       affordable while a fixture meant one emitter; a multi cell bar has one
       per cell, so a rig with a few pixel battens in it was clearing a couple
       of hundred megabytes of depth buffer per frame to draw nothing.

       SpotlightConeEntity gates the shading cone on the same condition, so
       nothing samples a shadow map while it is not being kept up to date, and
       this node sits ahead of the camera selector in the frame graph, so a
       cell that comes on gets its map rendered before it is read in the very
       same frame. */
    enabled: fixtureItem && fixtureItem.lightIntensity ? true : false

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
                    matchAny: FilterKey { name: "pass"; value: "shadows" }

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
