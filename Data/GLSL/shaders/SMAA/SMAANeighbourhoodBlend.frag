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

in vec4 vo_vPosition;
in vec2 vo_vTexCoord;
in vec4 vo_vOffset;

// Linear sample
SAMPLER_LOC(1, 0) uniform sampler2D sampler0;   // ColorTex
SAMPLER_LOC(1, 1) uniform sampler2D sampler1;   // BlendTex
#if SMAA_REPROJECTION
SAMPLER_LOC(1, 2) uniform sampler2D sampler2;  // velocity tex
#endif

layout(location = 0) out vec4 colorOut;

//-----------------------------------------------------------------------------
// Neighborhood Blending Pixel Shader (Third Pass)

void main(void)
{
    // Fetch the blending weights for current pixel:
    vec4 a;
    a.x = texture(sampler1, vo_vOffset.xy).a; // Right
    a.y = texture(sampler1, vo_vOffset.zw).g; // Top
    a.wz = texture(sampler1, vo_vTexCoord).xz; // Bottom / Left

    // Is there any blending weight with a value greater than 0.0?
    if (dot(a, vec4(1.0, 1.0, 1.0, 1.0)) < 1e-5) {
        vec4 color = textureLod(sampler0, vo_vTexCoord, 0.0);

        #if SMAA_REPROJECTION
        vec2 velocity = SMAA_DECODE_VELOCITY(textureLod(sampler2, vo_vTexCoord, 0.0));

        // Pack velocity into the alpha channel:
        color.a = sqrt(5.0 * length(velocity));
        #endif

        colorOut = color;
    } else {
        bool h = max(a.x, a.z) > max(a.y, a.w); // max(horizontal) > max(vertical)

        // Calculate the blending offsets:
        vec4 blendingOffset = vec4(0.0, a.y, 0.0, a.w);
        vec2 blendingWeight = a.yw;
        SMAAMovc(bvec4(h, h, h, h), blendingOffset, vec4(a.x, 0.0, a.z, 0.0));
        SMAAMovc(bvec2(h, h), blendingWeight, a.xz);
        blendingWeight /= dot(blendingWeight, vec2(1.0, 1.0));

        // Calculate the texture coordinates:
        vec4 blendingCoord = mad(blendingOffset, vec4(vScreenMetrics.xy, -vScreenMetrics.xy), vo_vTexCoord.xyxy);

        // We exploit bilinear filtering to mix current pixel with the chosen
        // neighbor:
        vec4 color = blendingWeight.x * textureLod(sampler0, blendingCoord.xy, 0.0);
        color += blendingWeight.y * textureLod(sampler0, blendingCoord.zw, 0.0);

        #if SMAA_REPROJECTION
        // Antialias velocity for proper reprojection in a later stage:
        vec2 velocity = blendingWeight.x * SMAA_DECODE_VELOCITY(textureLod(sampler2, blendingCoord.xy, 0.0));
        velocity += blendingWeight.y * SMAA_DECODE_VELOCITY(textureLod(sampler2, blendingCoord.zw, 0.0));

        // Pack velocity into the alpha channel:
        color.a = sqrt(5.0 * length(velocity));
        #endif

        colorOut = color;
    }
}