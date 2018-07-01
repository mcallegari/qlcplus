/*
  Q Light Controller Plus
  spotlight_shading.vert

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

VS_IN_ATTRIB vec3 vertexPosition;

VS_OUT_ATTRIB vec3 fsPos;

uniform mat4 viewProjectionMatrix;
uniform mat4 customModelMatrix;

void main()
{
    fsPos = (customModelMatrix * vec4(vertexPosition, 1.0)).xyz;
    
    gl_Position = viewProjectionMatrix * customModelMatrix * vec4(vertexPosition, 1.0);
}