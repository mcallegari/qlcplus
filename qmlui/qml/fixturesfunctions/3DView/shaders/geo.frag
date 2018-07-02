/*
  Q Light Controller Plus
  geo.frag

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

FS_IN_ATTRIB vec3 fsNormal;
FS_IN_ATTRIB vec3 fsPos;

FS_IN_ATTRIB vec4 fsHistCsPos;
FS_IN_ATTRIB vec4 fsUnjitteredCsPos;

uniform vec4 meshColor;

DECLARE_GBUFFER_OUTPUT

vec2 outputVelocity(vec4 histCsPos, vec4 curCsPos)
{
    vec2 vel = vec2(0.5, 0.5) * ( (histCsPos.xy / histCsPos.w) - (curCsPos.xy / curCsPos.w) ) ;
    return vel;
}

void main()
{
    MGL_FRAG_DATA0 = vec4(meshColor.xyzw);
    MGL_FRAG_DATA1 = vec4(fsNormal.xyz, 1.0);
    //MGL_FRAG_DATA2.xy = outputVelocity(fsHistCsPos, fsUnjitteredCsPos);
}
