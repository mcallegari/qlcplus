/*
  Q Light Controller Plus
  grab_bright.frag

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

layout(location = 0) in vec2 fsUv;
layout(location = 0) out vec4 fragColor;

layout(binding = 2) uniform sampler2D albedoTex;
layout(binding = 3) uniform sampler2D normalTex;

void main()
{
    vec4 albedo = texture(albedoTex, fsUv).xyzw;
    float v = texture(normalTex, fsUv).w;
    if (v > 2.0)
    {
        fragColor = vec4(albedo.rgb, 1.0);
    }
    else
    {
        fragColor = vec4(0.0, 0.0, 0.0, 0.0);
    }
}
