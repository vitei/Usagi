#include "../includes/platformdefines.inc"
#include "../includes/assao.inc"


SAMPLER_LOC(1, 0) uniform sampler2D g_DepthSource;   // corresponds to SSAO_TEXTURE_SLOT0

layout(location = 0) out float out0;
layout(location = 1) out float out1;


void main()
{
    ivec2 baseCoord = ivec2(gl_FragCoord.xy) * 2;
    float a = texelFetchOffset(g_DepthSource, baseCoord, 0, ivec2( 0, 0 ) ).x;
    float d = texelFetchOffset(g_DepthSource, baseCoord, 0, ivec2( 1, 1 ) ).x;

    out0 = ScreenSpaceToViewSpaceDepth( a );
    out1 = ScreenSpaceToViewSpaceDepth( d );
}