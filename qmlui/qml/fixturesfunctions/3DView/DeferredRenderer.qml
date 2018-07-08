/*
  Q Light Controller Plus
  DeferredRenderer.qml

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

RenderSettings
{
    pickingSettings.pickMethod: PickingSettings.TrianglePicking
    renderPolicy: RenderSettings.Always

    property GBuffer gBuffer
    property ForwardTarget forward
    property alias camera: sceneCameraSelector.camera
    property alias sceneDeferredLayer: sceneDeferredLayerFilter.layers
    property alias sceneSelectionLayer: sceneSelectionLayerFilter.layers
    property Layer screenQuadLayer
    //property alias debugLayer: debugLayerFilter.layers

    property real windowWidth: 0
    property real windowHeight: 0

    activeFrameGraph:
        Viewport
        {
            id: root
            normalizedRect: Qt.rect(0.0, 0.0, 1.0, 1.0)

            RenderSurfaceSelector
            {
                id: surfaceSelector

                CameraSelector
                {
                    id: sceneCameraSelector

                    // Fill G-Buffer
                    LayerFilter
                    {
                        id: sceneDeferredLayerFilter
                        RenderTargetSelector
                        {
                            id: gBufferTargetSelector
                            target: gBuffer

                            ClearBuffers
                            {
                                buffers: ClearBuffers.ColorDepthBuffer

                                RenderPassFilter
                                {
                                    id: geometryPass
                                    matchAny: FilterKey { name: "pass"; value: "geometry" }
                                }
                            }
                        }
                    }

                    // Fill selection FBO
                    LayerFilter
                    {
                        id: sceneSelectionLayerFilter
                        RenderTargetSelector
                        {
                            id: selectionTargetSelector
                            target: forward

                            ClearBuffers
                            {
                                buffers: ClearBuffers.ColorBuffer

                                RenderPassFilter
                                {
                                    id: selectionPass
                                    matchAny: FilterKey { name: "pass"; value: "geometry" }
                                }
                            }
                        }
                    }

                    // Lights pass technique
                    TechniqueFilter
                    {
                        parameters: [
                            Parameter { name: "color"; value: gBuffer.color },
                            Parameter { name: "position"; value: gBuffer.position },
                            Parameter { name: "normal"; value: gBuffer.normal },
                            Parameter { name: "depth"; value: gBuffer.depth },
                            Parameter { name: "winSize"; value: Qt.size(windowWidth, windowHeight) }
                        ]

                        RenderStateSet
                        {
                            // Render FullScreen Quad
                            renderStates: [
                                BlendEquation { blendFunction: BlendEquation.Add },
                                BlendEquationArguments
                                {
                                    sourceRgb: BlendEquationArguments.SourceAlpha
                                    destinationRgb: BlendEquationArguments.DestinationColor
                                }
                            ]
                            LayerFilter
                            {
                                //id: screenQuadLayerFilter
                                layers: screenQuadLayer

                                ClearBuffers
                                {
                                    buffers: ClearBuffers.ColorDepthBuffer
                                    RenderPassFilter
                                    {
                                        matchAny: FilterKey { name: "pass"; value: "lights" }
                                        parameters: Parameter { name: "winSize"; value: Qt.size(windowWidth, windowHeight) }

                                    }
                                }
                            }
/*
                            // RenderDebug layer
                            LayerFilter
                            {
                                id: debugLayerFilter
                                Viewport
                                {
                                    normalizedRect: Qt.rect(0.5, 0.5, 0.5, 0.5)
                                    RenderPassFilter
                                    {
                                        matchAny: FilterKey { name: "pass"; value: "lights" }
                                        parameters: Parameter { name: "winSize"; value: Qt.size(windowWidth * 0.5, windowHeight * 0.5) }
                                    }
                                }
                            }
*/
                        }
                    } // TechniqueFilter

                    // Mesh selection boxes forward pass technique
                    TechniqueFilter
                    {
                        parameters: [
                            Parameter { name: "color"; value: forward.color },
                            Parameter { name: "depth"; value: gBuffer.depth },
                            Parameter { name: "winSize"; value: Qt.size(windowWidth, windowHeight) }
                        ]

                        RenderStateSet
                        {
                            // Add to FullScreen Quad
                            renderStates: [
                                BlendEquation { blendFunction: BlendEquation.Add },
                                BlendEquationArguments
                                {
                                    sourceRgb: BlendEquationArguments.SourceAlpha
                                    destinationRgb: BlendEquationArguments.DestinationColor
                                }
                            ]
                            LayerFilter
                            {
                                layers: screenQuadLayer

                                RenderPassFilter
                                {
                                    matchAny: FilterKey { name: "pass"; value: "forward" }
                                    parameters: Parameter { name: "winSize"; value: Qt.size(windowWidth, windowHeight) }

                                }
                            }
                        }
                    } // TechniqueFilter

                } // CameraSelector
            } // RenderSurfaceSelector
        } // Viewport
}
