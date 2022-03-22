#include "../includes/platformdefines.inc"
#include "../includes/assao.inc"



SAMPLER_LOC(1, 0) uniform sampler2D g_DepthSource;   // corresponds to SSAO_TEXTURE_SLOT0

layout(location = 0) out float out0;
layout(location = 1) out float out1;
layout(location = 2) out float out2;
layout(location = 3) out float out3;


void main()
{
#if 0   // gather can be a bit faster but doesn't work with input depth buffers that don't match the working viewport
    vec2 gatherUV = inPos.xy * g_ASSAOConsts.Viewport2xPixelSize;
    vec4 depths = g_DepthSource.GatherRed( g_PointClampSampler, gatherUV );
    float a = depths.w;  // g_DepthSource.Load( int3( ivec2(inPos.xy) * 2, 0 ), ivec2( 0, 0 ) ).x;
    float b = depths.z;  // g_DepthSource.Load( int3( ivec2(inPos.xy) * 2, 0 ), ivec2( 1, 0 ) ).x;
    float c = depths.x;  // g_DepthSource.Load( int3( ivec2(inPos.xy) * 2, 0 ), ivec2( 0, 1 ) ).x;
    float d = depths.y;  // g_DepthSource.Load( int3( ivec2(inPos.xy) * 2, 0 ), ivec2( 1, 1 ) ).x;
#else
    ivec2 baseCoord = ivec2(gl_FragCoord.xy) * 2;
    float a = texelFetchOffset(g_DepthSource, baseCoord, 0, ivec2( 0, 0 ) ).x;
    float b = texelFetchOffset(g_DepthSource, baseCoord, 0, ivec2( 1, 0 ) ).x;
    float c = texelFetchOffset(g_DepthSource, baseCoord, 0, ivec2( 0, 1 ) ).x;
    float d = texelFetchOffset(g_DepthSource, baseCoord, 0, ivec2( 1, 1 ) ).x;
#endif

    out0 = ScreenSpaceToViewSpaceDepth( a );
    out1 = ScreenSpaceToViewSpaceDepth( b );
    out2 = ScreenSpaceToViewSpaceDepth( c );
    out3 = ScreenSpaceToViewSpaceDepth( d );
}