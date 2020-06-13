#include "../includes/platformdefines.inc"
#include "../includes/assao.inc"


ATTRIB_LOC(0) in vec2 vo_vTexCoord;

SAMPLER_LOC(1, 2) uniform sampler2D g_BlurInput;   // corresponds to SSAO_TEXTURE_SLOT2

layout(location = 0) out vec2 colorOut;



void main()
{
    vec2 halfPixel = g_ASSAOConsts.HalfViewportPixelSize * 0.5f;

    vec2 centre = textureLod(g_BlurInput, vo_vTexCoord, 0.0 ).xy;

    vec4 vals;
    vals.x = textureLod(g_BlurInput, vo_vTexCoord + vec2( -halfPixel.x * 3, -halfPixel.y ), 0.0 ).x;
    vals.y = textureLod(g_BlurInput, vo_vTexCoord + vec2( +halfPixel.x, -halfPixel.y * 3 ), 0.0 ).x;
    vals.z = textureLod(g_BlurInput, vo_vTexCoord + vec2( -halfPixel.x, +halfPixel.y * 3 ), 0.0 ).x;
    vals.w = textureLod(g_BlurInput, vo_vTexCoord + vec2( +halfPixel.x * 3, +halfPixel.y ), 0.0 ).x;

    colorOut = vec2(dot( vals, 0.2.xxxx ) + centre.x * 0.2, centre.y);
}