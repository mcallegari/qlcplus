/*
  Q Light Controller Plus
  BlitEffect.qml

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

Effect
{
    techniques:
    [
        // OpenGL 3.1
        Technique
        {
            graphicsApiFilter { api: GraphicsApiFilter.OpenGL; profile: GraphicsApiFilter.CoreProfile; majorVersion: 3; minorVersion: 1 }
            renderPasses:
            [
                // Lights pass
                RenderPass
                {
                    filterKeys: FilterKey { name: "pass"; value: "blit" }
                    shaderProgram:
                        ShaderProgram
                        {
                            vertexShaderCode: View3D.makeShader(loadSource("qrc:/fullscreen.vert"))
                            fragmentShaderCode: View3D.makeShader(loadSource("qrc:/blit.frag"))
                        }
                }
            ]
        }
    ]
}
