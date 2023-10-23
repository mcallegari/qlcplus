/*
  Q Light Controller Plus
  spotlight_shading.frag

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

layout(location = 0) in vec3 fsPos;
layout(location = 0) out vec4 fragColor;

layout(std140, binding = auto) uniform myUniforms {
    uniform vec3 lightPos;
    uniform vec3 lightDir;
    uniform vec3 lightColor;
    uniform float lightIntensity;
    uniform int useShadows;
    uniform vec4 goboRotation;
    uniform float coneTopRadius;
    uniform float coneBottomRadius;
    uniform float coneDistCutoff;
    uniform mat4 lightViewMatrix;
    uniform mat4 lightViewProjectionMatrix;
    uniform mat4 lightProjectionMatrix;
    uniform float headLength;
};

layout(binding = auto) uniform sampler2D albedoTex;
layout(binding = auto) uniform sampler2D normalTex;
layout(binding = auto) uniform sampler2D depthTex;
layout(binding = auto) uniform sampler2D goboTex;
layout(binding = auto) uniform sampler2D shadowTex;

void main()
{

    vec3 albedo, normal, position;
    
    vec4 u =  viewProjectionMatrix * vec4(fsPos, 1.0);
    vec2 uv = (u.xy / u.w) * 0.5 + vec2(0.5);
    albedo = texture(albedoTex, uv).rgb;
    normal = texture(normalTex, uv).xyz;
    float z = texture(depthTex, uv).r;

    vec4 temp = inverseViewProjectionMatrix * vec4(u.x / u.w, u.y / u.w, -1.0 + 2.0 * z, 1.0);
    temp.xyz = temp.xyz / temp.w;
    position = temp.xyz;
  
    float shadowMask = 1.0;
    if (useShadows == 1) {
        vec4 p = lightProjectionMatrix * lightViewMatrix * vec4(position.xyz, 1.0);
        float curZ = (p.z / p.w) * 0.5 + 0.5;
        float refZ = texture(shadowTex, ((p.xy) / p.w) * 0.5 + vec2(0.5)).r;
        shadowMask = (curZ < refZ + 0.0003 ? 1.0 : 0.0);
    }

    vec4 q = lightViewMatrix * vec4(position.xyz, 1.0);
    float r = coneTopRadius + (coneBottomRadius - coneTopRadius) * ((abs(q.z) - 0.5 * headLength) / coneDistCutoff);
    vec2 tc = (mat2x2(goboRotation.x, goboRotation.y, goboRotation.z, goboRotation.w) * ((-q.xy) * (1.0 / r))) * 0.5 + 0.5;

    vec4 gSample = texture(goboTex, tc.xy);
    float goboMask = gSample.a * gSample.r;

    vec3 finalColor = shadowMask * goboMask * lightColor * lightIntensity * max(0, dot(normal, -lightDir)) * albedo;

    fragColor = vec4(finalColor, 1.0);
    //fragColor = vec4(1.0, 0.0, 0.0, 1.0);
}
