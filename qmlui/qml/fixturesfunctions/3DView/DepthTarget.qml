/*
  Q Light Controller Plus
  DepthTarget.qml

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

RenderTarget
{
    property Texture2D positionTex:
        Texture2D
        {
            width: 1024
            height: 1024
            // We use RGBA32F here instead of a more fitting format because
            // OpenGL vendors might not support other formats
            format: Texture.RGBA32F
            generateMipMaps: false
            magnificationFilter: Texture.Nearest
            minificationFilter: Texture.Nearest
            wrapMode
            {
                x: WrapMode.ClampToEdge
                y: WrapMode.ClampToEdge
            }
        }

    attachments: [

        RenderTargetOutput
        {
            attachmentPoint: RenderTargetOutput.Color0
            texture: positionTex
        }
    ] // outputs
}
