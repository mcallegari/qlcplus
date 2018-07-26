/*
  Q Light Controller Plus
  3DView.qml

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

import QtQuick 2.3

import QtQuick.Scene3D 2.0
import Qt3D.Core 2.0
import Qt3D.Render 2.0
import Qt3D.Input 2.0
import Qt3D.Extras 2.0

Rectangle
{
    anchors.fill: parent
    color: "black"
    clip: true

    property string contextName: "3D"
    property alias contextItem: scene3d
    property real cameraZ: -7.0

    Component.onDestruction: if(contextManager) contextManager.enableContext("3D", false, scene3d)

    function hasSettings()
    {
        return true;
    }

    function showSettings(show)
    {
        threeDSettings.visible = show
    }

    function setZoom(amount)
    {
        cameraZ += amount
    }

    Scene3D
    {
        id: scene3d
        objectName: "scene3DItem"
        z: 1
        anchors.fill: parent
        aspects: ["input", "logic"]

        function updateSceneGraph(create)
        {
            var ic
            var component
            var mynode
            var fixtures = []
            var fixtureItem

            for (ic = 0; ic < frameGraph.myShadowFrameGraphNode.childNodes.length; ++ic)
            {
                if (frameGraph.myShadowFrameGraphNode.childNodes[ic] !== null)
                    frameGraph.myShadowFrameGraphNode.childNodes[ic].destroy()
            }

            for (ic = 0; ic < frameGraph.myCameraSelector.childNodes.length; ++ic)
            {
                if (frameGraph.myCameraSelector.childNodes[ic] !== null)
                    frameGraph.myCameraSelector.childNodes[ic].destroy()
            }

            for (ic = 0; ic < sceneEntity.childNodes.length; ++ic)
            {
                if (sceneEntity.childNodes[ic] === null)
                    continue

                if (sceneEntity.childNodes[ic].objectName === "fixture3DItem")
                    fixtures.push(sceneEntity.childNodes[ic])
            }

            if (create === false)
                return

            for (ic = 0; ic < fixtures.length; ++ic)
            {
                fixtureItem = fixtures[ic]

                if(fixtureItem.useShadows) {
                    component = Qt.createComponent("RenderShadowMapFilter.qml");
                    if (component.status === Component.Error)
                        console.log("Error loading component:", component.errorString());

                    mynode = component.createObject(frameGraph.myShadowFrameGraphNode,
                    {
                        "sceneDeferredLayer": sceneEntity.deferredLayer,
                        "fixtureItem": fixtureItem
                    });
                }
            }

            console.log("BUILDING FRAME GRAPH")

            component = Qt.createComponent("FillGBufferFilter.qml");
            if (component.status === Component.Error)
                console.log("Error loading component:", component.errorString());
            
            mynode = component.createObject(frameGraph.myCameraSelector,
            {
                "gBuffer": gBufferTarget,
                "sceneDeferredLayer": sceneEntity.deferredLayer,
            });

            component = Qt.createComponent("RenderSelectionBoxesFilter.qml");
            if (component.status === Component.Error)
                console.log("Error loading component:", component.errorString());

            mynode = component.createObject(frameGraph.myCameraSelector,
            {
                "gBuffer": gBufferTarget,
                "layer": sceneEntity.selectionLayer,
            });

            component = Qt.createComponent("DirectionalLightFilter.qml");
            if (component.status === Component.Error)
                console.log("Error loading component:", component.errorString());
            mynode = component.createObject(frameGraph.myCameraSelector,
            {
                "gBuffer": gBufferTarget,
                "screenQuadLayer": screenQuadEntity.layer,
                "frameTarget": frameTarget
            });

            for (ic = 0; ic < fixtures.length; ++ic)
            {
                fixtureItem = fixtures[ic]

                component = Qt.createComponent("SpotlightShadingFilter.qml");
                if (component.status === Component.Error)
                    console.log("Error loading component:", component.errorString());

                mynode = component.createObject(frameGraph.myCameraSelector,
                {
                    "gBuffer": gBufferTarget,
                    "shadowTex": fixtureItem.shadowMap.depth,
                    "useShadows": fixtureItem.useShadows,              
                    "spotlightShadingLayer": fixtureItem.spotlightShadingLayer,
                    "frameTarget": frameTarget
                });
            }

            for (ic = 0; ic < fixtures.length; ++ic)
            {
                fixtureItem = fixtures[ic]

                if (fixtureItem.useScattering)
                {
                    component = Qt.createComponent("OutputFrontDepthFilter.qml");
                    if (component.status === Component.Error)
                        console.log("Error loading component:", component.errorString());

                    mynode = component.createObject(frameGraph.myCameraSelector,
                    {
                        "frontDepth": depthTarget,
                        "outputDepthLayer": fixtureItem.outputDepthLayer
                    });

                    component = Qt.createComponent("SpotlightScatteringFilter.qml");
                    if (component.status === Component.Error)
                        console.log("Error loading component:", component.errorString());

                    mynode = component.createObject(frameGraph.myCameraSelector,
                    {
                        "fixtureItem": fixtureItem,
                        "frontDepth": depthTarget,
                        "gBuffer": gBufferTarget,
                        "spotlightScatteringLayer": fixtureItem.spotlightScatteringLayer,
                        "shadowTex": fixtureItem.shadowMap.depth,
                        "frameTarget": frameTarget,
                        "useShadows": fixtureItem.useShadows
                    });
                }
            }  

            component = Qt.createComponent("GammaCorrectFilter.qml");
            if (component.status === Component.Error)
                console.log("Error loading component:", component.errorString());

            mynode = component.createObject(frameGraph.myCameraSelector,
            {
                "hdrTexture": frameTarget.color,
                "outRenderTarget": hdr0RenderTarget,                
                "screenQuadGammaCorrectLayer": screenQuadGammaCorrectEntity.layer
            });
            
            component = Qt.createComponent("FXAAFilter.qml");
            if (component.status === Component.Error)
                console.log("Error loading component:", component.errorString());

            mynode = component.createObject(frameGraph.myCameraSelector,
            {
                "inTexture": hdr0ColorTexture,
                "outRenderTarget": hdr1RenderTarget,                
                "screenQuadFXAALayer": screenQuadFXAAEntity.layer
            });

            component = Qt.createComponent("BlitFilter.qml");
            if (component.status === Component.Error)
                console.log("Error loading component:", component.errorString());

            mynode = component.createObject(frameGraph.myCameraSelector,
            {
                "inTexture": hdr1ColorTexture,
                "screenQuadBlitLayer": screenQuadBlitEntity.layer
            });
        }

        Entity
        {
            Component.onCompleted: contextManager.enableContext("3D", true, scene3d)

            OrbitCameraController
            {
                id: camController
                camera: sceneEntity.camera
                linearSpeed: 40.0
                lookSpeed: 300.0
            }

            SceneEntity
            {
                id: sceneEntity
                viewSize: Qt.size(scene3d.width, scene3d.height)
            }
            ScreenQuadGammaCorrectEntity { id: screenQuadGammaCorrectEntity }
            ScreenQuadFXAAEntity { id: screenQuadFXAAEntity }
            ScreenQuadBlitEntity { id: screenQuadBlitEntity }

            ScreenQuadEntity { id: screenQuadEntity }

            GBuffer { id: gBufferTarget }

            FrameTarget { id: frameTarget }

            RenderTarget
            {
                id: hdr0RenderTarget
                attachments: [
                    RenderTargetOutput
                    {
                        objectName: "color"
                        attachmentPoint: RenderTargetOutput.Color0
                        texture:
                            Texture2D
                            {
                                id: hdr0ColorTexture
                                width: 1024
                                height: 1024
                                format: Texture.RGBA32F
                                generateMipMaps: false
                                magnificationFilter: Texture.Linear
                                minificationFilter: Texture.Linear
                                wrapMode
                                {   
                                    x: WrapMode.ClampToEdge
                                    y: WrapMode.ClampToEdge
                                }
                            }
                    }
                ] // outputs
            }            

            RenderTarget
            {
                id: hdr1RenderTarget
                attachments: [
                    RenderTargetOutput
                    {
                        objectName: "color"
                        attachmentPoint: RenderTargetOutput.Color0
                        texture:
                            Texture2D
                            {
                                id: hdr1ColorTexture
                                width: 1024
                                height: 1024
                                format: Texture.RGBA32F
                                generateMipMaps: false
                                magnificationFilter: Texture.Linear
                                minificationFilter: Texture.Linear
                                wrapMode
                                {   
                                    x: WrapMode.ClampToEdge
                                    y: WrapMode.ClampToEdge
                                }
                            }
                    }
                ] // outputs
            }   

            DepthTarget { id: depthTarget }

            components : [
                DeferredRenderer
                {
                    id: frameGraph
                    camera : sceneEntity.camera
                },
                InputSettings {}
            ]
        } // Entity
    } // scene3d

    Rectangle
    {
        visible: View3D.frameCountEnabled
        z: 4
        opacity: 0.6
        color: UISettings.bgMain
        width: height
        height: UISettings.bigItemHeight

        Column
        {
            RobotoText { height: UISettings.bigItemHeight / 4; label: "FPS: " + View3D.FPS }
            RobotoText { height: UISettings.bigItemHeight / 4; label: "Min: " + View3D.minFPS }
            RobotoText { height: UISettings.bigItemHeight / 4; label: "Max: " + View3D.maxFPS }
            RobotoText { height: UISettings.bigItemHeight / 4; label: "Avg: " + View3D.avgFPS }
        }
    }

    SettingsView3D
    {
        id: threeDSettings
        visible: false
        x: parent.width - width
        z: 5
    }
}
