/*
  Q Light Controller Plus
  GeometryPassEffect.qml

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

Effect
{
    function makeShader(s)
    {
        return View3D.makeShader(s)
    }

    techniques: [
        Technique
        {
            graphicsApiFilter { api: GraphicsApiFilter.OpenGL; profile: GraphicsApiFilter.CoreProfile; majorVersion: 3; minorVersion: 1 }
            renderPasses: [
                RenderPass
                {
                    filterKeys: FilterKey { name: "pass"; value: "geometry" }
                    shaderProgram:
                        ShaderProgram
                        {
                            vertexShaderCode:  View3D.makeShader(loadSource("qrc:/geo.vert"))
                            fragmentShaderCode:  View3D.makeShader(loadSource("qrc:/geo.frag"))
                        }
                }, // render pass
                RenderPass
                {
                    filterKeys: FilterKey { name: "pass"; value: "shadows" }
                    shaderProgram:
                        ShaderProgram
                        {
                            vertexShaderCode: View3D.makeShader(loadSource("qrc:/output_depth.vert"))
                            fragmentShaderCode: View3D.makeShader(loadSource("qrc:/output_depth.frag"))
                        }
                } // render pass
            ]
        }
    ]
}
