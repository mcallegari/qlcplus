/*
  Q Light Controller Plus
  blur.frag

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


uniform sampler2D tex;
uniform vec4 blurDirection;

DECLARE_FRAG_COLOR

/*
9-tap gaussian blur, taken from 
https://github.com/Jam3/glsl-fast-gaussian-blur
It is MIT-licensed. 
*/
vec4 blur9(sampler2D image, vec2 uv, vec2 resolution, vec2 direction) {
  vec4 color = vec4(0.0);
  vec2 off1 = vec2(1.3846153846) * direction;
  vec2 off2 = vec2(3.2307692308) * direction;
  color += SAMPLE_TEX2D(image, uv) * 0.2270270270;
  color += SAMPLE_TEX2D(image, uv + (off1 / resolution)) * 0.3162162162;
  color += SAMPLE_TEX2D(image, uv - (off1 / resolution)) * 0.3162162162;
  color += SAMPLE_TEX2D(image, uv + (off2 / resolution)) * 0.0702702703;
  color += SAMPLE_TEX2D(image, uv - (off2 / resolution)) * 0.0702702703;
  return color;
}


void main()
{
    MGL_FRAG_COLOR = blur9(tex, fsUv, vec2(1024.0, 1024.0), blurDirection.xy);   
}
