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
uniform sampler2D specularTex;
uniform sampler2D depthTex;

uniform mat4 inverseViewProjectionMatrix;

uniform float ambient;

uniform vec3 eyePosition;

DECLARE_FRAG_COLOR

void main()
{
    
    vec3 position;
    {
        float z = SAMPLE_TEX2D(depthTex, fsUv).r;

        vec2 u =  2.0 * fsUv - vec2(1.0);
        vec4 temp = inverseViewProjectionMatrix * vec4(u.x, u.y, -1.0 + 2.0 * z, 1.0);
        temp.xyz = temp.xyz / temp.w;
        position = temp.xyz;
    }
    
    vec4 albedo = SAMPLE_TEX2D(albedoTex, fsUv).xyzw;
    vec4 s = SAMPLE_TEX2D(specularTex, fsUv).xyzw;
    vec4 specular = s.xyzw;
    float shininess = s.w;

    vec3 finalColor = vec3(0.0);
    vec3 l = normalize(vec3(1.05, 1.3, 0.9));
    vec3 n =  normalize(SAMPLE_TEX2D(normalTex, fsUv).xyz);
    float flag =  normalize(SAMPLE_TEX2D(normalTex, fsUv).w);
    
    float isGuiElement = abs(albedo.w - 2.0) < 0.0001 ? 1.0 : 0.0;
    finalColor += isGuiElement * albedo.rgb;

    vec3 v = normalize(eyePosition - position); 
    vec3 r = normalize(2.0 * dot(l, n.xyz) * n.xyz - l); 

    if(flag < 2.1)
        finalColor += (1.0 - isGuiElement) * ambient * ( albedo.rgb * max(0.0, dot(l, n)) + specular.rgb * pow(max(0.0, dot(r, v) ), shininess));
  
    MGL_FRAG_COLOR = vec4(finalColor, 1.0);
}
