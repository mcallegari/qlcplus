/*
  Q Light Controller Plus
  upsample.frag

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
uniform vec4 pixelSize;
uniform vec4 intensity;

DECLARE_FRAG_COLOR

void main()
{
	vec2 halfpixel = pixelSize.xy;
	vec2 uv = fsUv;

	vec4 sum = vec4(0.0, 0.0, 0.0, 0.0);

	sum += (2.0 / 16.0) * SAMPLE_TEX2D(tex, uv + vec2(-halfpixel.x,  0.0) );
	sum += (2.0 / 16.0) * SAMPLE_TEX2D(tex, uv + vec2( 0.0,          halfpixel.y) );
	sum += (2.0 / 16.0) * SAMPLE_TEX2D(tex, uv + vec2( halfpixel.x,  0.0) );
	sum += (2.0 / 16.0) * SAMPLE_TEX2D(tex, uv + vec2( 0.0,         -halfpixel.y) );

	sum += (1.0 / 16.0) * SAMPLE_TEX2D(tex, uv + vec2(-halfpixel.x, -halfpixel.y) );
	sum += (1.0 / 16.0) * SAMPLE_TEX2D(tex, uv + vec2(-halfpixel.x,  halfpixel.y) );
	sum += (1.0 / 16.0) * SAMPLE_TEX2D(tex, uv + vec2( halfpixel.x, -halfpixel.y) );
	sum += (1.0 / 16.0) * SAMPLE_TEX2D(tex, uv + vec2( halfpixel.x,  halfpixel.y) );

	sum += (4.0 / 16.0) * SAMPLE_TEX2D(tex, uv);

	MGL_FRAG_COLOR = intensity.x * sum;
}
