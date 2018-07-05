/*
  Q Light Controller Plus
  output_depth.vert

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

uniform mat4 lightProjectionMatrix;
uniform mat4 lightViewMatrix;
uniform mat4 modelMatrix;

void main()
{
    gl_Position = lightProjectionMatrix * lightViewMatrix * modelMatrix * vec4(vertexPosition, 1.0);    
}

