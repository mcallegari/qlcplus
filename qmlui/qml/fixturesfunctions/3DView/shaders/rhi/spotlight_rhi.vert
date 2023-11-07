/*
  Q Light Controller Plus
  spotlight_rhi.vert

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

layout(location = 0) in vec3 vertexPosition;
layout(location = 0) out vec3 fsPos;

layout(std140, binding = auto) uniform myUniforms {
    mat4 customModelMatrix;
    float coneTopRadius;
    float coneBottomRadius;
};

void main()
{
    // the mesh is cylinder. We modulate the bottom and top radiuses in the vertex shader.
    vec3 p = (vertexPosition.y > 0.0) ? 
                vertexPosition * vec3(coneTopRadius, 1.0, coneTopRadius): 
                vertexPosition * vec3(coneBottomRadius, 1.0, coneBottomRadius);

    fsPos = (customModelMatrix * vec4(p, 1.0)).xyz;
    gl_Position = viewProjectionMatrix * customModelMatrix * vec4(p, 1.0);
}
