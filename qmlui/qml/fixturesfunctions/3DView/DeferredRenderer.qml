/*
  Q Light Controller Plus
  DeferredRenderer.qml

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

import Qt3D.Core 2.0
import Qt3D.Render 2.0

import QtQuick 2.0

RenderSettings
{
    pickingSettings.pickMethod: PickingSettings.TrianglePicking
    //renderPolicy: RenderSettings.OnDemand

    property alias camera: sceneCameraSelector.camera
    property alias myCameraSelector: sceneCameraSelector
    property alias myShadowFrameGraphNode: shadowFrameGraphNode

    property real windowWidth: 0
    property real windowHeight: 0

    activeFrameGraph:
        Viewport
        {
            id: root
            normalizedRect: Qt.rect(0.0, 0.0, 1.0, 1.0)

            RenderSurfaceSelector
            {
                FrameGraphNode { id: shadowFrameGraphNode }
                CameraSelector { id: sceneCameraSelector }
            } // RenderSurfaceSelector
        } // Viewport
}
