/*
  Q Light Controller Plus
  3DView.qml

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
    property alias cameraZ: sceneEntity.cameraZ

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
        cameraZ -= amount
    }

    Scene3D
    {
        id: scene3d
        objectName: "scene3DItem"
        z: 1
        anchors.fill: parent
        aspects: ["input", "logic"]

        function updateFrameGraph(create)
        {
            var ic
            var iHead
            var headEntity
            var component, component2
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

            console.log("BUILDING FRAME GRAPH")

            if (fixtures.length)
            {
                component = Qt.createComponent("RenderShadowMapFilter.qml");
                if (component.status === Component.Error)
                    console.log("Error loading component:", component.errorString())
            }

            for (ic = 0; ic < fixtures.length; ++ic)
            {
                fixtureItem = fixtures[ic]

                if (fixtureItem.useShadows)
                {
                    for (iHead = 0; iHead < fixtureItem.headsNumber; iHead++)
                    {
                        headEntity = fixtureItem.getHead(iHead)

                        component.createObject(frameGraph.myShadowFrameGraphNode,
                        {
                            "fixtureItem": headEntity,
                            "layers": sceneEntity.deferredLayer
                        });
                    }
                }
            }

            component = Qt.createComponent("FillGBufferFilter.qml");
            if (component.status === Component.Error)
                console.log("Error loading component:", component.errorString())

            component.createObject(frameGraph.myCameraSelector,
            {
                "gBuffer": gBufferTarget,
                "layers": sceneEntity.deferredLayer,
            });

            component = Qt.createComponent("RenderSelectionBoxesFilter.qml");
            if (component.status === Component.Error)
                console.log("Error loading component:", component.errorString())

            component.createObject(frameGraph.myCameraSelector,
            {
                "gBuffer": gBufferTarget,
                "layers": sceneEntity.selectionLayer,
            });

            var texChainTargets = [texChainTarget0, texChainTarget1, texChainTarget2, texChainTarget3, texChainTarget4]         
            var texChainTextures = [texChainTexture0, texChainTexture1, texChainTexture2, texChainTexture3, texChainTexture4]

            var TEX_CHAIN_LEN = texChainTargets.length

            var texChainDownsampleEntities = [
                screenQuadDownsampleEntity0, screenQuadDownsampleEntity1, screenQuadDownsampleEntity2, screenQuadDownsampleEntity3,
                 screenQuadDownsampleEntity4
            ]

            var texChainUpsampleEntities = [
                screenQuadUpsampleEntity0, screenQuadUpsampleEntity1, screenQuadUpsampleEntity2, screenQuadUpsampleEntity3,
                 screenQuadUpsampleEntity4
            ]

            component = Qt.createComponent("GrabBrightFilter.qml");
            if (component.status === Component.Error)
                console.log("Error loading component:", component.errorString());

            component.createObject(frameGraph.myCameraSelector,
            {
                "gBuffer": gBufferTarget,
                "screenQuadLayer": screenQuadGrabBrightEntity.quadLayer,
                "outRenderTarget": texChainTargets[0]
            });

            var m_width = 1024.0
            var m_height = 1024.0
            var dim

            component = Qt.createComponent("DownsampleFilter.qml");
            if (component.status === Component.Error)
                console.log("Error loading component:", component.errorString());

            for (ic = 0; ic < (TEX_CHAIN_LEN - 1); ++ic)
            {
                dim = (1 << (ic + 1))

                component.createObject(frameGraph.myCameraSelector,
                {
                    "inTex": texChainTextures[ic],
                    "screenQuadLayer": texChainDownsampleEntities[ic].quadLayer,
                    "outRenderTarget": texChainTargets[ic + 1],
                    "pixelSize": Qt.vector4d(1.0 / (m_width  / dim), 1.0 / (m_height / dim), 0, 0)
                });
            }

            component = Qt.createComponent("UpsampleFilter.qml");
            if (component.status === Component.Error)
                console.log("Error loading component:", component.errorString());

            for (ic = 0; ic < (TEX_CHAIN_LEN - 1); ++ic)
            {
                dim = (1 << (TEX_CHAIN_LEN - 2 - ic))
                component.createObject(frameGraph.myCameraSelector,
                {
                    "inTex": texChainTextures[TEX_CHAIN_LEN - 1 - ic],
                    "screenQuadLayer": texChainUpsampleEntities[ic].quadLayer,
                    "outRenderTarget": texChainTargets[TEX_CHAIN_LEN - ic - 2],
                    "pixelSize": Qt.vector4d(1.0 / (m_width  / dim), 1.0 / (m_height / dim), 0, 0),
                    "index":  Qt.vector4d( (TEX_CHAIN_LEN - 1 - ic), 0.0, 0, 0),
                });
            }

            component = Qt.createComponent("DirectionalLightFilter.qml");
            if (component.status === Component.Error)
                console.log("Error loading component:", component.errorString())
            component.createObject(frameGraph.myCameraSelector,
            {
                "gBuffer": gBufferTarget,
                "screenQuadLayer": screenQuadEntity.layer,
                "frameTarget": frameTarget
            });

            if (fixtures.length)
            {
                component = Qt.createComponent("SpotlightShadingFilter.qml");
                if (component.status === Component.Error)
                    console.log("Error loading component:", component.errorString())
            }

            for (ic = 0; ic < fixtures.length; ++ic)
            {
                fixtureItem = fixtures[ic]

                if (fixtureItem.useScattering === false)
                    continue

                for (iHead = 0; iHead < fixtureItem.headsNumber; iHead++)
                {
                    headEntity = fixtureItem.getHead(iHead)

                    component.createObject(frameGraph.myCameraSelector,
                    {
                        "gBuffer": gBufferTarget,
                        "shadowTex": headEntity.depthTex,
                        "useShadows": fixtureItem.useShadows,
                        "spotlightShadingLayer": headEntity.spotlightShadingLayer,
                        "frameTarget": frameTarget
                    });
                }
            }

            if (fixtures.length)
            {
                component = Qt.createComponent("OutputFrontDepthFilter.qml");
                if (component.status === Component.Error)
                    console.log("Error loading component:", component.errorString())

                component2 = Qt.createComponent("SpotlightScatteringFilter.qml");
                if (component2.status === Component.Error)
                    console.log("Error loading component:", component2.errorString())
            }

            for (ic = 0; ic < fixtures.length; ++ic)
            {
                fixtureItem = fixtures[ic]

                if (fixtureItem.useScattering === false)
                    continue

                for (iHead = 0; iHead < fixtureItem.headsNumber; iHead++)
                {
                    headEntity = fixtureItem.getHead(iHead)

                    component.createObject(frameGraph.myCameraSelector,
                    {
                        "frontDepth": depthTarget,
                        "outputDepthLayer": headEntity.outputDepthLayer
                    });

                    component2.createObject(frameGraph.myCameraSelector,
                    {
                        "fixtureItem": headEntity,
                        "frontDepth": depthTarget,
                        "gBuffer": gBufferTarget,
                        "spotlightScatteringLayer": headEntity.spotlightScatteringLayer,
                        "shadowTex": headEntity.depthTex,
                        "frameTarget": frameTarget,
                        "useShadows": fixtureItem.useShadows
                    });
                }
            }

            component = Qt.createComponent("GammaCorrectFilter.qml");
            if (component.status === Component.Error)
                console.log("Error loading component:", component.errorString());

            component.createObject(frameGraph.myCameraSelector,
            {
                "hdrTexture": frameTarget.color,
                "bloomTexture": texChainTextures[0],
                "outRenderTarget": hdr0RenderTarget,
                "screenQuadGammaCorrectLayer": screenQuadGammaCorrectEntity.layer
            });

            component = Qt.createComponent("FXAAFilter.qml");
            if (component.status === Component.Error)
                console.log("Error loading component:", component.errorString())

            component.createObject(frameGraph.myCameraSelector,
            {
                "inTexture": hdr0ColorTexture,
                "outRenderTarget": hdr1RenderTarget,
                "screenQuadFXAALayer": screenQuadFXAAEntity.quadLayer       
            });

            component = Qt.createComponent("BlitFilter.qml");
            if (component.status === Component.Error)
                console.log("Error loading component:", component.errorString())

            component.createObject(frameGraph.myCameraSelector,
            {
                "inTexture": hdr1ColorTexture,
                "screenQuadBlitLayer": screenQuadBlitEntity.quadLayer
            });
        }

        Entity
        {
            objectName: "scene3DEntity"
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

            GenericScreenQuadEntity
            {
                id: screenQuadFXAAEntity
                quadLayer: Layer { }
                quadEffect: FXAAEffect { }
            }
    
            GenericScreenQuadEntity
            {
                id: screenQuadBlitEntity
                quadLayer: Layer { }
                quadEffect: BlitEffect { }
            }

            GenericScreenQuadEntity
            {
                id: screenQuadGrabBrightEntity
                quadLayer: Layer { }
                quadEffect: GrabBrightEffect { }
            }

            GenericScreenQuadEntity
            {
                id: screenQuadDownsampleEntity0
                quadLayer: Layer { }
                quadEffect: DownsampleEffect { }
            }

            GenericScreenQuadEntity
            {
                id: screenQuadDownsampleEntity1
                quadLayer: Layer { }
                quadEffect: DownsampleEffect { }
            }

            GenericScreenQuadEntity
            {
                id: screenQuadDownsampleEntity2
                quadLayer: Layer { }
                quadEffect: DownsampleEffect { }
            }

            GenericScreenQuadEntity
            {
                id: screenQuadDownsampleEntity3
                quadLayer: Layer { }
                quadEffect: DownsampleEffect { }
            }

            GenericScreenQuadEntity
            {
                id: screenQuadDownsampleEntity4
                quadLayer: Layer { }
                quadEffect: DownsampleEffect { }
            }

            GenericScreenQuadEntity
            {
                id: screenQuadUpsampleEntity0
                quadLayer: Layer { }
                quadEffect: UpsampleEffect { }
            }

            GenericScreenQuadEntity
            {
                id: screenQuadUpsampleEntity1
                quadLayer: Layer { }
                quadEffect: UpsampleEffect { }
            }

            GenericScreenQuadEntity
            {
                id: screenQuadUpsampleEntity2
                quadLayer: Layer { }
                quadEffect: UpsampleEffect { }
            }

            GenericScreenQuadEntity
            {
                id: screenQuadUpsampleEntity3
                quadLayer: Layer { }
                quadEffect: UpsampleEffect { }
            }

            GenericScreenQuadEntity
            {
                id: screenQuadUpsampleEntity4
                quadLayer: Layer { }
                quadEffect: UpsampleEffect { }
            }

            ScreenQuadEntity { id: screenQuadEntity }

            GBuffer { id: gBufferTarget }

            FrameTarget { id: frameTarget }

            Texture2D
            {
                id: texChainTexture0
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

            RenderTarget
            {
                id: texChainTarget0
                attachments: [
                    RenderTargetOutput
                    {
                        attachmentPoint: RenderTargetOutput.Color0
                        texture: texChainTexture0
                    }
                ]
            }

            Texture2D
            {
                id: texChainTexture1
                width: 512
                height: 512
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

            RenderTarget
            {
                id: texChainTarget1
                attachments: [
                    RenderTargetOutput
                    {
                        attachmentPoint: RenderTargetOutput.Color0
                        texture: texChainTexture1
                    }
                ]
            }

            Texture2D
            {
                id: texChainTexture2
                width: 256
                height: 256
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

            RenderTarget
            {
                id: texChainTarget2
                attachments: [
                    RenderTargetOutput
                    {
                        attachmentPoint: RenderTargetOutput.Color0
                        texture: texChainTexture2
                    }
                ]
            }

            Texture2D
            {
                id: texChainTexture3
                width: 128
                height: 128
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

            RenderTarget
            {
                id: texChainTarget3
                attachments: [
                    RenderTargetOutput
                    {
                        attachmentPoint: RenderTargetOutput.Color0
                        texture: texChainTexture3
                    }
                ]
            }

            Texture2D
            {
                id: texChainTexture4
                width: 64
                height: 64
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

            RenderTarget
            {
                id: texChainTarget4
                attachments: [
                    RenderTargetOutput
                    {
                        attachmentPoint: RenderTargetOutput.Color0
                        texture: texChainTexture4
                    }
                ] // attachments
            }   

            property Texture2D hdr0ColorTexture:
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

            RenderTarget
            {
                id: hdr0RenderTarget
                attachments: [
                    RenderTargetOutput
                    {
                        attachmentPoint: RenderTargetOutput.Color0
                        texture: hdr0ColorTexture
                    }
                ]
            }

            property Texture2D hdr1ColorTexture:
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

            RenderTarget
            {
                id: hdr1RenderTarget
                attachments: [
                    RenderTargetOutput
                    {
                        attachmentPoint: RenderTargetOutput.Color0
                        texture: hdr1ColorTexture
                    }
                ]
            }

            DepthTarget { id: depthTarget }

            components: [
                DeferredRenderer
                {
                    id: frameGraph
                    camera: sceneEntity.camera
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
