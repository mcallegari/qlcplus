/*
  Q Light Controller Plus
  geo.vert

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
VS_IN_ATTRIB vec3 vertexNormal;

VS_OUT_ATTRIB vec3 fsNormal;
VS_OUT_ATTRIB vec3 fsPos;

VS_OUT_ATTRIB vec4 fsHistCsPos;
VS_OUT_ATTRIB vec4 fsUnjitteredCsPos;

uniform mat4 viewProjectionMatrix;
uniform mat4 modelMatrix;

uniform mat4 uHistVp;
uniform mat4 uUnjitteredVp;

void main()
{
    fsNormal =  (modelMatrix * vec4(vertexNormal, 0.0)).xyz;
    gl_Position = viewProjectionMatrix * modelMatrix * vec4(vertexPosition, 1.0);
    fsPos = (modelMatrix * vec4(vertexPosition, 1.0)).xyz;

    // history clip-space position.
    fsHistCsPos = (uHistVp * vec4(vertexPosition, 1.0));
    // current clip-space pos, but with no jitter.
    fsUnjitteredCsPos = (uUnjitteredVp * vec4(vertexPosition, 1.0));    
}
