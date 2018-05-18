/**
 * Copyright (C) 2013 Jorge Jimenez (jorge@iryoku.com)
 * Copyright (C) 2013 Jose I. Echevarria (joseignacioechevarria@gmail.com)
 * Copyright (C) 2013 Belen Masia (bmasia@unizar.es)
 * Copyright (C) 2013 Fernando Navarro (fernandn@microsoft.com)
 * Copyright (C) 2013 Diego Gutierrez (diegog@unizar.es)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to
 * do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software. As clarification, there
 * is no requirement that the copyright notice and permission be included in
 * binary distributions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "../includes/platformdefines.inc"
#include "../includes/smaa.inc"

ATTRIB_LOC(0) in vec4 vo_vPosition;
ATTRIB_LOC(1) in vec2 vo_vTexCoord;

SAMPLER_LOC(1, 0) uniform sampler2D sampler0;   // CurrentColor
SAMPLER_LOC(1, 1) uniform sampler2D sampler1;   // PreviousColor
#if SMAA_PREDICATION
SAMPLER_LOC(1, 2) uniform sampler2D sampler2;  // VelocityTex
#endif

layout(location = 0) out vec4 colorOut;

//-----------------------------------------------------------------------------
// Temporal Resolve Pixel Shader (Optional Pass)
void main(void)
{
    #if SMAA_REPROJECTION
    // Velocity is assumed to be calculated for motion blur, so we need to
    // inverse it for reprojection:
    vec2 velocity = -SMAA_DECODE_VELOCITY(texture(sampler2, vo_vTexCoord).rg);

    // Fetch current pixel:
    vec4 current = texture(sampler0, vo_vTexCoord);

    // Reproject current coordinates and fetch previous pixel:
    vec4 previous = texture(sampler1, vo_vTexCoord + velocity);

    // Attenuate the previous pixel if the velocity is different:
    float delta = abs(current.a * current.a - previous.a * previous.a) / 5.0;
    float weight = 0.5 * clamp(1.0 - sqrt(delta) * SMAA_REPROJECTION_WEIGHT_SCALE, 0.0, 1.0);

    // Blend the pixels according to the calculated weight:
    colorOut = mix(current, previous, weight);
    #else
    // Just blend the pixels:
    vec4 current = texture(sampler0, vo_vTexCoord);
    vec4 previous = texture(sampler1, vo_vTexCoord);
    colorOut = mix(current, previous, 0.5);
    #endif
}