/*
  Q Light Controller Plus
  LightPassEffect.qml

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
                    filterKeys: FilterKey { name : "pass"; value : "lights" }
                    shaderProgram: ShaderProgram {
                        vertexShaderCode: loadSource("qrc:/lights_gl3.vert")
                        fragmentShaderCode: loadSource("qrc:/lights_gl3.frag")
                    }
                },
                // Forward pass
                RenderPass
                {
                    filterKeys : FilterKey { name : "pass"; value : "forward" }
                    shaderProgram : ShaderProgram {
                        vertexShaderCode:
                            "#version 140

                            in vec4 vertexPosition;
                            uniform mat4 modelMatrix;

                            void main()
                            {
                                gl_Position = modelMatrix * vertexPosition;
                            }"
                        fragmentShaderCode:
                            "#version 140

                            uniform sampler2D color;
                            uniform vec2 winSize;

                            out vec4 fragColor;

                            void main()
                            {
                                vec2 texCoord = gl_FragCoord.xy / winSize;
                                fragColor = texture(color, texCoord);
                            }"
                    }
                }
            ]
        },
        // OpenGL 2.0 with FBO extension
        Technique
        {
            graphicsApiFilter { api: GraphicsApiFilter.OpenGL; profile: GraphicsApiFilter.NoProfile; majorVersion: 2; minorVersion: 0 }
            renderPasses:
            [
                // Lights pass
                RenderPass
                {
                    filterKeys: FilterKey { name: "pass"; value: "lights" }
                    shaderProgram: ShaderProgram {
                        vertexShaderCode: loadSource("qrc:/lights_es2.vert")
                        fragmentShaderCode: loadSource("qrc:/lights_es2.frag")
                    }
                },
                // Forward pass
                RenderPass
                {
                    filterKeys : FilterKey { name : "pass"; value : "forward" }
                    shaderProgram : ShaderProgram {
                        vertexShaderCode:
                            "#version 110

                            attribute vec4 vertexPosition;
                            uniform mat4 modelMatrix;

                            void main()
                            {
                                gl_Position = modelMatrix * vertexPosition;
                            }"
                        fragmentShaderCode:
                            "#version 110

                            uniform sampler2D color;
                            uniform vec2 winSize;

                            void main()
                            {
                                vec2 texCoord = gl_FragCoord.xy / winSize;
                                gl_FragColor = texture2D(color, texCoord);
                            }"
                    }
                }
            ]
        }
    ]
}
