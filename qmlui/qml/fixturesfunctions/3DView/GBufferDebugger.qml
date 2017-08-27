/*
  Q Light Controller Plus
  GBufferDebugger.qml

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
import Qt3D.Extras 2.0

Entity
{
    readonly property Layer layer: debugLayer

    components : [
        Layer { id: debugLayer },

        PlaneMesh
        {
            width: 2.0
            height: 2.0
            meshResolution: Qt.size(2, 2)
        },

        Transform { // We rotate the plane so that it faces us
            rotation: fromAxisAndAngle(Qt.vector3d(1, 0, 0), 90)
        },

        Material
        {
            effect: Effect {
                techniques: [
                    Technique {
                        graphicsApiFilter { api: GraphicsApiFilter.OpenGL; profile: GraphicsApiFilter.NoProfile; majorVersion: 2; minorVersion: 0 }
                        renderPasses: RenderPass {
                            filterKeys: FilterKey { name: "pass"; value: "lights" }
                            shaderProgram: ShaderProgram {
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
                                    uniform sampler2D position;
                                    uniform sampler2D normal;
                                    uniform sampler2D depth;
                                    uniform vec2 winSize;

                                    void main()
                                    {
                                        vec2 texCoord = (gl_FragCoord.xy + vec2(-winSize.x, 0)) / winSize;

                                        // Draw 4 quadrants
                                        if (texCoord.x > 0.5) { // Right
                                            if (texCoord.y > 0.5) { // Top
                                                gl_FragColor = vec4(texture2D(normal, vec2(texCoord.x - 0.5, texCoord.y - 0.5) * 2.0).xyz, 1.0);
                                            } else { // Bottom
                                                gl_FragColor = vec4(texture2D(color, vec2(texCoord.x - 0.5, texCoord.y) * 2.0).xyz, 1.0);
                                            }
                                        } else { // Left
                                            if (texCoord.y > 0.5) { // Top
                                                gl_FragColor = texture2D(position, vec2(texCoord.x, texCoord.y - 0.5) * 2.0);
                                            } else { // Bottom
                                                gl_FragColor = vec4(texture2D(depth, texCoord * 2.0).rrr, 1.0);
                                            }
                                        }
                                        gl_FragColor.a = 0.5;
                                    }"
                            }
                        }
                    },
                    Technique {
                        graphicsApiFilter { api: GraphicsApiFilter.OpenGL; profile: GraphicsApiFilter.CoreProfile; majorVersion: 3; minorVersion: 2 }
                        renderPasses: RenderPass {
                            filterKeys: FilterKey { name: "pass"; value: "lights" }
                            shaderProgram: ShaderProgram {
                                vertexShaderCode:
                                    "#version 150

                                    in vec4 vertexPosition;
                                    uniform mat4 modelMatrix;

                                    void main()
                                    {
                                        gl_Position = modelMatrix * vertexPosition;
                                    }"
                                fragmentShaderCode:
                                    "#version 150

                                    uniform sampler2D color;
                                    uniform sampler2D position;
                                    uniform sampler2D normal;
                                    uniform sampler2D depth;
                                    uniform vec2 winSize;

                                    out vec4 fragColor;

                                    void main()
                                    {
                                        vec2 texCoord = (gl_FragCoord.xy + vec2(-winSize.x, 0)) / winSize;

                                        // Draw 4 quadrants
                                        if (texCoord.x > 0.5) { // Right
                                            if (texCoord.y > 0.5) { // Top
                                                fragColor = vec4(texture(normal, vec2(texCoord.x - 0.5, texCoord.y - 0.5) * 2.0).xyz, 1.0);
                                            } else { // Bottom
                                                fragColor = vec4(texture(color, vec2(texCoord.x - 0.5, texCoord.y) * 2.0).xyz, 1.0);
                                            }
                                        } else { // Left
                                            if (texCoord.y > 0.5) { // Top
                                                fragColor = texture(position, vec2(texCoord.x, texCoord.y - 0.5) * 2.0);
                                            } else { // Bottom
                                                fragColor = vec4(texture(depth, texCoord * 2.0).rrr, 1.0);
                                            }
                                        }
                                        fragColor.a = 0.5;
                                    }"
                            }
                        }
                    }
                ]
            }
        }
    ]
}
