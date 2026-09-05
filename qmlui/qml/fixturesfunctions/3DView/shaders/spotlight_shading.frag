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

FS_IN_ATTRIB vec3 fsPos;

DECLARE_FRAG_COLOR

uniform mat4 viewProjectionMatrix;
uniform mat4 inverseViewProjectionMatrix;

uniform sampler2D albedoTex;
uniform sampler2D normalTex;
uniform sampler2D depthTex;

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

uniform sampler2D goboTex;
uniform sampler2D shadowTex;

uniform float headLength;

// Global gain on fixture light landing on surfaces, from the 3D view
// Rendering settings. The volumetric beams have their own scaling in
// spotlight_scattering.frag, through the smoke amount.
uniform float fixtureLightIntensity;

// Distance, in world units, at which fixture light lands unscaled: the mean
// height of the rig above the floor, derived from the project. Light that has
// travelled further is dimmed by the square of the ratio and light that has
// travelled less is brightened, as illuminance does in the room. Normalising on
// the rig's own scale keeps the frame's overall exposure where it was, so this
// term changes the balance between near and far throws without acting as a
// second global gain on top of fixtureLightIntensity. 0 disables it, which is
// how this pass has always rendered: a beam lands the same brightness however
// far it has travelled.
uniform float referenceThrow;

void main()
{

    vec3 albedo, normal, position;

    vec4 u =  viewProjectionMatrix * vec4(fsPos, 1.0);
    vec2 uv = (u.xy / u.w) * 0.5 + vec2(0.5);
    albedo = SAMPLE_TEX2D(albedoTex, uv).rgb;
    normal = SAMPLE_TEX2D(normalTex, uv).xyz;
    float z = SAMPLE_TEX2D(depthTex, uv).r;

    vec4 temp = inverseViewProjectionMatrix * vec4(u.x / u.w, u.y / u.w, -1.0 + 2.0 * z, 1.0);
    temp.xyz = temp.xyz / temp.w;
    position = temp.xyz;

    float shadowMask = 1.0;
    if (useShadows == 1) {
        vec4 p = lightProjectionMatrix * lightViewMatrix * vec4(position.xyz, 1.0);
        float curZ = (p.z / p.w) * 0.5 + 0.5;
        float refZ = SAMPLE_TEX2D(shadowTex, ((p.xy) / p.w) * 0.5 + vec2(0.5)).r;
        shadowMask = (curZ < refZ + 0.0003 ? 1.0 : 0.0);
    }

    vec4 q = lightViewMatrix * vec4(position.xyz, 1.0);
    float r = coneTopRadius + (coneBottomRadius - coneTopRadius) * ((abs(q.z) - 0.5 * headLength) / coneDistCutoff);
    vec2 tc = (mat2x2(goboRotation.x, goboRotation.y, goboRotation.z, goboRotation.w) * ((-q.xy) * (1.0 / r))) * 0.5 + 0.5;

    // Distance the light has travelled to reach this fragment, measured along
    // the beam axis from the emitter, the same quantity the cone radius above
    // is a function of. Floored so a surface up against the lens cannot divide
    // the intensity to infinity.
    float falloff = 1.0;
    if (referenceThrow > 0.0)
    {
        float beamDist = max(abs(q.z) - 0.5 * headLength, 0.25);
        falloff = (referenceThrow * referenceThrow) / (beamDist * beamDist);
    }

    vec4 gSample = SAMPLE_TEX2D(goboTex, tc.xy);
    float goboMask = gSample.a * gSample.r;

    vec3 finalColor = fixtureLightIntensity * falloff * shadowMask * goboMask * lightColor * lightIntensity * max(0, dot(normal, -lightDir)) * albedo;

    MGL_FRAG_COLOR = vec4(finalColor, 1.0);
    //MGL_FRAG_COLOR = vec4(1.0, 0.0, 0.0, 1.0);
}
