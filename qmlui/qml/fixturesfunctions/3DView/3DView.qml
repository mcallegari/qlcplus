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
        viewCamera.setZoom(-amount)
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

            // Global elements
            Camera
            {
                id: viewCamera

                projectionType: CameraLens.PerspectiveProjection
                fieldOfView: 45
                aspectRatio: viewSize.width / viewSize.height
                nearPlane: 1.0
                farPlane: 1000.0
                position: View3D.cameraPosition
                upVector: View3D.cameraUpVector
                viewCenter: View3D.cameraViewCenter

                function setZoom(amount)
                {
                    translate(Qt.vector3d(0, 0, -amount), Camera.DontTranslateViewCenter)
                }
            }

            MouseDevice
            {
                id: mDevice
            }

            MouseHandler
            {
                property point startPoint
                property int direction
                property int directionCounter
                property real dx
                property real dy

                property int selFixturesCount: contextManager ? contextManager.selectedFixturesCount : 0
                property int selGenericCount: View3D.genericSelectedCount

                sourceDevice: mDevice
                onPressed:
                {
                    directionCounter = 0
                    dx = 0
                    dy = 0
                    startPoint = Qt.point(mouse.x, mouse.y)
                }

                onPositionChanged:
                {
                    if (directionCounter < 3)
                    {
                        dx += (Math.abs(mouse.x - startPoint.x))
                        dy += (Math.abs(mouse.y - startPoint.y))
                        directionCounter++
                        return
                    }
                    else
                    {
                        direction = dx > dy ? Qt.Horizontal : Qt.Vertical
                    }

                    var newPos
                    var xDelta = mouse.x - startPoint.x
                    var yDelta = mouse.y - startPoint.y

                    if (mouse.buttons === Qt.LeftButton) // move items
                    {
                        xDelta = xDelta * viewCamera.position.z
                        yDelta = yDelta * viewCamera.position.z

                        if (selFixturesCount == 1 && selGenericCount == 0)
                        {
                            newPos = contextManager.fixturesPosition

                            if (direction == Qt.Horizontal)
                                contextManager.fixturesPosition = Qt.vector3d(newPos.x + xDelta, newPos.y, newPos.z)
                            else
                            {
                                if (mouse.modifiers & Qt.ShiftModifier)
                                    contextManager.fixturesPosition = Qt.vector3d(newPos.x, newPos.y, newPos.z + yDelta)
                                else
                                    contextManager.fixturesPosition = Qt.vector3d(newPos.x, newPos.y - yDelta, newPos.z)
                            }

                            threeDSettings.refreshPositionValues(false)
                        }
                        else if (selFixturesCount == 0 && selGenericCount == 1)
                        {
                            newPos = View3D.genericItemsPosition

                            if (direction == Qt.Horizontal)
                                View3D.genericItemsPosition = Qt.vector3d(newPos.x + xDelta, newPos.y, newPos.z)
                            else
                            {
                                if (mouse.modifiers & Qt.ShiftModifier)
                                    View3D.genericItemsPosition = Qt.vector3d(newPos.x, newPos.y, newPos.z + yDelta)
                                else
                                    View3D.genericItemsPosition = Qt.vector3d(newPos.x, newPos.y - yDelta, newPos.z)
                            }

                            threeDSettings.refreshPositionValues(true)
                        }
                        else
                        {
                            if (direction == Qt.Horizontal)
                                newPos = Qt.vector3d(xDelta, 0, 0)
                            else
                            {
                                if (mouse.modifiers & Qt.ShiftModifier)
                                    newPos = Qt.vector3d(0, 0, yDelta)
                                else
                                    newPos = Qt.vector3d(0, -yDelta, 0)
                            }

                            contextManager.fixturesPosition = newPos
                            View3D.genericItemsPosition = newPos
                        }
                    }
                    else if (mouse.buttons === Qt.RightButton)  // camera rotation
                    {
                        if (!mouse.modifiers || (mouse.modifiers & Qt.ShiftModifier && direction == Qt.Horizontal))
                            viewCamera.panAboutViewCenter(-xDelta, Qt.vector3d(0, 1, 0))
                        if (!mouse.modifiers || (mouse.modifiers & Qt.ShiftModifier && direction == Qt.Vertical))
                            viewCamera.tiltAboutViewCenter(yDelta, Qt.vector3d(1, 0, 0))

                        View3D.cameraPosition = viewCamera.position
                        View3D.cameraUpVector = viewCamera.upVector
                        View3D.cameraViewCenter = viewCamera.viewCenter
                    }
                    else if (mouse.buttons === Qt.MiddleButton) // camera translation
                    {
                        if (!mouse.modifiers || (mouse.modifiers & Qt.ShiftModifier && direction == Qt.Horizontal))
                            viewCamera.translate(Qt.vector3d(-xDelta / 100, 0, 0))
                        if (!mouse.modifiers || (mouse.modifiers & Qt.ShiftModifier && direction == Qt.Vertical))
                            viewCamera.translate(Qt.vector3d(0, yDelta / 100, 0))

                        View3D.cameraPosition = viewCamera.position
                        View3D.cameraUpVector = viewCamera.upVector
                        View3D.cameraViewCenter = viewCamera.viewCenter
                    }
                    startPoint = Qt.point(mouse.x, mouse.y)
                }

                onWheel:
                {
                    if (wheel.angleDelta.y > 0)
                        viewCamera.setZoom(-1)
                    else
                        viewCamera.setZoom(1)
                }
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
                    camera: viewCamera
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
        color: UISettings.bgMedium
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
