/*
  Q Light Controller Plus
  blit.frag

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


FS_IN_ATTRIB vec2 fsUv;

uniform sampler2D colorTex;

DECLARE_FRAG_COLOR

void main()
{
    vec3 finalColor = 1.0 * SAMPLE_TEX2D(colorTex, fsUv).rgb;
    MGL_FRAG_COLOR = vec4(finalColor, 1.0);
}
