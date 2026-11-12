/*
  Q Light Controller Plus
  smoke_plume.glsl

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

// Smoke put in the air by Smoke and Hazer fixtures, three vec4 per emitter:
// (position, density), (direction, reach) and (colour of its own LEDs, 0).
// A null direction is a hazer. Shared by every shader that sees smoke.
uniform vec4 smokeEmitters[24];
uniform int smokeEmitterCount;
uniform float smokeTime;

// density of emitter e at p: a jet is a tube that widens and thins with
// distance, rises as it travels and breaks up at its tip; a hazer is a soft
// cloud around the machine
float emitterDensity(int e, vec3 p)
{
    vec4 src = smokeEmitters[e * 3];
    vec4 jet = smokeEmitters[e * 3 + 1];
    vec3 d = p - src.xyz;

    if (dot(jet.xyz, jet.xyz) < 0.01)
        return src.w * 0.25 * exp(-dot(d, d) / 18.0);

    float t = dot(d, jet.xyz);
    if (t < 0.0 || t > jet.w * 1.3)
        return 0.0;
    // hot smoke rises: bend the axis up with the square of the distance
    vec3 axis = src.xyz + jet.xyz * t + vec3(0.0, 0.05 * t * t, 0.0);
    float radius = 0.05 + 0.09 * t;
    vec3 off = p - axis;
    float turb = 0.7 + 0.3 * sin(p.x * 5.1 + smokeTime * 2.7) * sin(p.y * 4.3 - smokeTime * 3.3) *
                             sin(p.z * 4.7 + smokeTime * 2.1);
    float tip = 1.0 - smoothstep(jet.w * 0.6, jet.w * 1.3, t);
    return src.w * turb * tip * exp(-dot(off, off) / (radius * radius)) / (1.0 + 0.5 * t);
}

float plumeDensity(vec3 p)
{
    float density = 0.0;
    for (int e = 0; e < smokeEmitterCount; ++e)
        density += emitterDensity(e, p);
    return density;
}

// the part of the ray from a to b that crosses emitter e's smoke, as distances
// along the ray; empty when it misses. A jet is bounded by a cylinder around
// its axis, as wide as its widest point plus how far it bends up; a sphere
// that held the whole jet covered half the screen and cost 9.5 ms a frame
vec2 emitterSpan(int e, vec3 a, vec3 dir, float len)
{
    vec4 src = smokeEmitters[e * 3];
    vec4 jet = smokeEmitters[e * 3 + 1];
    vec3 w = a - src.xyz;

    if (dot(jet.xyz, jet.xyz) < 0.01)
    {
        float b = dot(w, dir);
        float h = b * b - (dot(w, w) - 81.0);
        if (h < 0.0)
            return vec2(1.0, 0.0);
        h = sqrt(h);
        return vec2(max(0.0, -b - h), min(len, -b + h));
    }

    float reach = jet.w * 1.3;
    float radius = 0.05 + 0.09 * reach + 0.05 * reach * reach * (1.0 - abs(jet.y));
    vec3 dPerp = dir - dot(dir, jet.xyz) * jet.xyz;
    vec3 wPerp = w - dot(w, jet.xyz) * jet.xyz;
    float qa = dot(dPerp, dPerp);
    float qb = dot(wPerp, dPerp);
    float qc = dot(wPerp, wPerp) - radius * radius;
    vec2 span = vec2(0.0, len);
    if (qa < 1e-6)
    {
        if (qc > 0.0)
            return vec2(1.0, 0.0);
    }
    else
    {
        float h = qb * qb - qa * qc;
        if (h < 0.0)
            return vec2(1.0, 0.0);
        h = sqrt(h);
        span = vec2(max(span.x, (-qb - h) / qa), min(span.y, (-qb + h) / qa));
    }
    // and only between the nozzle and the tip
    float along = dot(dir, jet.xyz);
    float start = dot(w, jet.xyz);
    if (abs(along) < 1e-6)
    {
        if (start < 0.0 || start > reach)
            return vec2(1.0, 0.0);
    }
    else
    {
        float s0 = -start / along;
        float s1 = (reach - start) / along;
        span = vec2(max(span.x, min(s0, s1)), min(span.y, max(s0, s1)));
    }
    return span;
}

// light the plumes send to the eye along a ray from a to b: each machine's
// smoke glows with its own LEDs, plus the ambient light of the room
vec3 plumeGlow(vec3 a, vec3 b, float ambient)
{
    const int steps = 24;
    float len = length(b - a);
    vec3 dir = (b - a) / len;
    vec3 glow = vec3(0.0);
    // start each pixel's march at a different fraction of a step, so the steps
    // show as fine noise instead of bands
    float jitter = fract(sin(dot(gl_FragCoord.xy, vec2(12.9898, 78.233))) * 43758.5453);

    for (int e = 0; e < smokeEmitterCount; ++e)
    {
        vec2 span = emitterSpan(e, a, dir, len);
        if (span.x >= span.y)
            continue;
        float stepLength = (span.y - span.x) / float(steps);
        vec3 light = smokeEmitters[e * 3 + 2].rgb + vec3(0.15 * ambient);
        float transmittance = 1.0;
        for (int i = 0; i < steps; ++i)
        {
            vec3 p = a + dir * (span.x + stepLength * (float(i) + jitter));
            float absorb = emitterDensity(e, p) * stepLength * 2.0;
            glow += transmittance * light * absorb;
            transmittance *= exp(-absorb);
        }
    }
    return glow;
}
