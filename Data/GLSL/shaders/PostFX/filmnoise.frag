// The MIT License (MIT) Copyright (c) 2015 Matt DesLauriers
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#include "../includes/platformdefines.inc"
#include "../includes/noise/noisecommon.inc"
#include "../includes/noise/pnoise3d.inc"
#include "../includes/noise/snoise3d.inc"

BUFFER_LAYOUT(1, UBO_MATERIAL_ID) uniform Material
{
	vec2  vResolution;
	float fTime;
};

ATTRIB_LOC(0) in vec2 vo_vTexCoord;
SAMPLER_LOC(1, 0) uniform sampler2D sampler0;



float grain(vec2 texCoord, vec2 resolution, float frame, float multiplier) {
    vec2 mult = texCoord * resolution;
    float offset = snoise(vec3(mult / multiplier, frame));
    float n1 = pnoise(vec3(mult, offset), vec3(1.0/texCoord * resolution, 1.0));
    return n1 / 2.0 + 0.5;
}

float grain(vec2 texCoord, vec2 resolution, float frame) {
    return grain(texCoord, resolution, frame, 2.5);
}

float grain(vec2 texCoord, vec2 resolution) {
    return grain(texCoord, resolution, 0.0);
}


vec3 blend(vec3 base, vec3 blend)
{
    return mix(
        sqrt(base) * (2.0 * blend - 1.0) + 2.0 * base * (1.0 - blend), 
        2.0 * base * blend + base * base * (1.0 - 2.0 * blend), 
        step(base, vec3(0.5))
    );
}


float luma(vec3 vSample)
{
	return sqrt(dot(vSample.rgb, vec3(0.299, 0.587, 0.114)));
}

layout(location = 0) out vec4 colorOut;

void main(void)
{	
   vec3 texColor = texture(sampler0, vo_vTexCoord).rgb;

    float zoom = 0.35;
    vec3 g = vec3(grain(vo_vTexCoord, vResolution * zoom, fTime));

    float luminance = luma(texColor);
    vec3 desaturated = vec3(luminance);

    vec3 color = blend(desaturated, g);

    float response = smoothstep(0.05, 0.5, luminance);
    color = mix(color, desaturated, pow(response,2.0));

    colorOut = vec4(color, 1.0);

}
