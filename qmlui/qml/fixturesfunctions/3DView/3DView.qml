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

        Entity
        {
            Component.onCompleted: contextManager.enableContext("3D", true, scene3d)

            //FirstPersonCameraController { camera: sceneEntity.camera }

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

            ScreenQuadEntity { id: screenQuadEntity }

            GBuffer { id: gBufferTarget }

            ForwardTarget
            {
                id: forwardTarget
                depthAttachment: gBufferTarget.depth
            }

            //GBufferDebugger { id: debugEntity }

            components : [
                DeferredRenderer
                {
                    id: frameGraph
                    camera : sceneEntity.camera
                    gBuffer: gBufferTarget
                    forward: forwardTarget
                    sceneDeferredLayer: sceneEntity.deferredLayer
                    sceneSelectionLayer: sceneEntity.selectionLayer
                    screenQuadLayer: screenQuadEntity.layer
                    windowWidth: scene3d.width
                    windowHeight: scene3d.height
                    //debugLayer: debugEntity.layer
                },
                InputSettings {}
            ]
        } // Entity
    } // scene3d

    SettingsView3D
    {
        id: threeDSettings
        visible: false
        x: parent.width - width
        z: 5
    }
}
