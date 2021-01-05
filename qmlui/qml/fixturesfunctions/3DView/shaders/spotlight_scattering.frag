/*
  Q Light Controller Plus
  spotlight_scattering.frag

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

uniform vec3 eyePosition;

uniform vec3 lightDir;
uniform vec3 lightPos;
uniform float lightIntensity;

uniform int raymarchSteps;

uniform float uLightTanCutoffAngle;
uniform vec3 lightColor;
uniform int useShadows;

uniform vec4 goboRotation;

uniform float coneTopRadius;
uniform float coneBottomRadius;
uniform float coneDistCutoff;

uniform float smokeAmount;

uniform sampler2D depthTex;
uniform mat4 viewProjectionMatrix;
uniform mat4 inverseViewProjectionMatrix;

uniform sampler2D goboTex;
uniform sampler2D shadowTex;
uniform mat4 lightViewProjectionScaleAndOffsetMatrix;

uniform mat4 lightViewMatrix;

uniform float headLength;

uniform sampler2D frontDepthTex;

float hash(float n)
{
    return fract(sin(n) * 43758.5453123);
}

void main()
{

    vec3 l = lightDir;
    vec3 rd = normalize(fsPos - eyePosition); // ray marching direction.

    vec3 begP;
    vec3 endP = fsPos;

    vec4 u =  viewProjectionMatrix * vec4(fsPos, 1.0);
    vec2 uv = (u.xy / u.w) * 0.5 + vec2(0.5);

    float z = SAMPLE_TEX2D(depthTex, uv).r;

    vec4 temp = inverseViewProjectionMatrix * vec4(u.x / u.w, u.y / u.w, -1.0 + 2.0 * z, 1.0);

    temp.xyz = temp.xyz / temp.w;
    vec3 position = temp.xyz;
    vec3 intersectBackgroundP = position;

    begP = SAMPLE_TEX2D(frontDepthTex, uv).xyz;
    if (length(begP)  < 0.0001)
    {
        begP = eyePosition;  // camera is inside spotlight.
    }
    else
    {
        if(distance(intersectBackgroundP, eyePosition) < distance(begP, eyePosition))
        {
            MGL_FRAG_COLOR = vec4(0.0, 0.0, 0.0, 0.0);
            return;
        }
    }

    endP = fsPos;
    if(distance(begP,intersectBackgroundP) < distance(begP, fsPos))
        endP =  intersectBackgroundP;

    float stepLength = distance(begP, endP) / float(raymarchSteps);

    vec3 p = begP;

    // add initial noise add beginning of ray. noise is removed by anti-aliasing later
    vec2 j = gl_FragCoord.xy * 10.2;
    p += rd * stepLength * hash(j.x + j.y * 47.0) * 0.3;

    float accum = 0.0;
    int i;
    float beta = 2.0e-5;

    // precalculate fixed math
    float g = 0.67;
    float pmFactor = ((1.0) / (4.0 * 3.14)) * ((3.0 * (1.0 - g * g)) / (2.0 * (2.0 + g * g)));

    for (i = 0; i < raymarchSteps; ++i)
    {
        float shadowMask = 1.0f;
        if(useShadows == 1)
        {
            vec4 q = (lightViewProjectionScaleAndOffsetMatrix * vec4(p, 1.0));
            float curZ = q.z / q.w;
            float refZ = SAMPLE_TEX2D(shadowTex, (q.xy / q.w)).r;
            shadowMask = (curZ < refZ ? 1.0 : 0.0);
        }
        
        float dist = distance(p, lightPos);
        float cos_theta = dot(l, -rd);
        float pm = pmFactor * ((1.0 + cos_theta * cos_theta) / (pow(1.0 + g * g - 2.0 * g * cos_theta, 3.0 / 2.0)));
        vec3 pf = p * 0.5;

        vec4 myq = lightViewMatrix * vec4(p.xyz, 1.0);
        float r = coneTopRadius + (coneBottomRadius - coneTopRadius) * ((abs(myq.z) - 0.5 * headLength) / coneDistCutoff);
        vec2 tc = mat2x2(goboRotation.x, goboRotation.y, goboRotation.z, goboRotation.w) * (((-myq.xy) * (1.0 / r))) * 0.5 + 0.5;

        vec4 gSample = SAMPLE_TEX2D(goboTex, tc.xy);
        
        float goboMask = gSample.a * gSample.r;

        float contrib =  (1.0 / (1.0  + 0.09 * dist + 0.032 * dist * dist)) * stepLength;

        contrib *= shadowMask;
        contrib *= goboMask;

        accum += contrib;

        p += rd * stepLength;
    }
    MGL_FRAG_COLOR = vec4(accum * lightIntensity * smokeAmount * lightColor, 0.0);
    //MGL_FRAG_COLOR = vec4(1.0, 0.0, 0.0, 0.0);
}
