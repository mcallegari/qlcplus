/*
  Q Light Controller Plus
  directional.frag

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

uniform sampler2D albedoTex;
uniform sampler2D normalTex;

uniform float ambient;

DECLARE_FRAG_COLOR

void main()
{
    vec4 albedo = SAMPLE_TEX2D(albedoTex, fsUv).xyzw;
    
    vec3 finalColor = vec3(0.0);
    vec3 l = normalize(vec3(1.0, 1.0, 1.0));
    vec3 n =  normalize(SAMPLE_TEX2D(normalTex, fsUv).xyz);
    
    float isGuiElement = abs(albedo.w - 2.0) < 0.0001 ? 1.0 : 0.0;
    finalColor = isGuiElement * albedo.rgb + (1.0 - isGuiElement) * ambient * albedo.rgb * max(0.0, dot(l, n));
    
    MGL_FRAG_COLOR = vec4(finalColor, 1.0);
}
