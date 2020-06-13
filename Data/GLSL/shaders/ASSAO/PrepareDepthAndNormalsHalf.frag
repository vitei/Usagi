#include "../includes/platformdefines.inc"
#include "../includes/assao.inc"


ATTRIB_LOC(0) in vec2 vo_vTexCoord;

SAMPLER_LOC(1, 0) uniform sampler2D g_DepthSource;   // corresponds to SSAO_TEXTURE_SLOT0
SAMPLER_LOC(1, 2) uniform sampler2D g_BlurInput;   // corresponds to SSAO_TEXTURE_SLOT2

layout(location = 0) out float out0;
layout(location = 1) out float out1;
layout(location = 2) out float out2;
layout(location = 3) out float out3;

writeonly layout(binding=4) uniform image2D g_NormalsOutputUAV;   // corresponds to SSAO_NORMALMAP_OUT_UAV_SLOT


void main()
{
    ivec2 baseCoords = ( ivec2(gl_FragCoord.xy) * 2);
    vec2 upperLeftUV = (gl_FragCoord.xy - 0.25) * g_ASSAOConsts.Viewport2xPixelSize;

    ivec2 baseCoord = ivec2(gl_FragCoord.xy) * 2;
    float z0 = ScreenSpaceToViewSpaceDepth( texelFetchOffset(g_DepthSource, baseCoord, 0, ivec2( 0, 0 ) ).x );
    float z1 = ScreenSpaceToViewSpaceDepth( texelFetchOffset(g_DepthSource, baseCoord, 0, ivec2( 1, 0 ) ).x );
    float z2 = ScreenSpaceToViewSpaceDepth( texelFetchOffset(g_DepthSource, baseCoord, 0, ivec2( 0, 1 ) ).x );
    float z3 = ScreenSpaceToViewSpaceDepth( texelFetchOffset(g_DepthSource, baseCoord, 0, ivec2( 1, 1 ) ).x );

    out0 = z0;
    out1 = z3;

    float pixZs[4][4];

    // middle 4
    pixZs[1][1] = z0;
    pixZs[2][1] = z1;
    pixZs[1][2] = z2;
    pixZs[2][2] = z3;
    // left 2
    pixZs[0][1] = ScreenSpaceToViewSpaceDepth(  textureLodOffset(g_DepthSource, upperLeftUV, 0.0, ivec2( -1, 0 ) ).x ); 
    pixZs[0][2] = ScreenSpaceToViewSpaceDepth(  textureLodOffset(g_DepthSource, upperLeftUV, 0.0, ivec2( -1, 1 ) ).x ); 
    // right 2
    pixZs[3][1] = ScreenSpaceToViewSpaceDepth(  textureLodOffset(g_DepthSource, upperLeftUV, 0.0, ivec2(  2, 0 ) ).x ); 
    pixZs[3][2] = ScreenSpaceToViewSpaceDepth(  textureLodOffset(g_DepthSource, upperLeftUV, 0.0, ivec2(  2, 1 ) ).x ); 
    // top 2
    pixZs[1][0] = ScreenSpaceToViewSpaceDepth(  textureLodOffset(g_DepthSource, upperLeftUV, 0.0, ivec2(  0, -1 ) ).x );
    pixZs[2][0] = ScreenSpaceToViewSpaceDepth(  textureLodOffset(g_DepthSource, upperLeftUV, 0.0, ivec2(  1, -1 ) ).x );
    // bottom 2
    pixZs[1][3] = ScreenSpaceToViewSpaceDepth(  textureLodOffset(g_DepthSource, upperLeftUV, 0.0, ivec2(  0,  2 ) ).x );
    pixZs[2][3] = ScreenSpaceToViewSpaceDepth(  textureLodOffset(g_DepthSource, upperLeftUV, 0.0, ivec2(  1,  2 ) ).x );

    vec4 edges0 = CalculateEdges( pixZs[1][1], pixZs[0][1], pixZs[2][1], pixZs[1][0], pixZs[1][2] );
    vec4 edges1 = CalculateEdges( pixZs[2][1], pixZs[1][1], pixZs[3][1], pixZs[2][0], pixZs[2][2] );
    vec4 edges2 = CalculateEdges( pixZs[1][2], pixZs[0][2], pixZs[2][2], pixZs[1][1], pixZs[1][3] );
    vec4 edges3 = CalculateEdges( pixZs[2][2], pixZs[1][2], pixZs[3][2], pixZs[2][1], pixZs[2][3] );

    vec3 pixPos[4][4];

    // there is probably a way to optimize the math below; however no approximation will work, has to be precise.

    // middle 4
    pixPos[1][1] = NDCToViewspace( upperLeftUV + g_ASSAOConsts.ViewportPixelSize * vec2( 0.0,  0.0 ), pixZs[1][1] );
    pixPos[2][1] = NDCToViewspace( upperLeftUV + g_ASSAOConsts.ViewportPixelSize * vec2( 1.0,  0.0 ), pixZs[2][1] );
    pixPos[1][2] = NDCToViewspace( upperLeftUV + g_ASSAOConsts.ViewportPixelSize * vec2( 0.0,  1.0 ), pixZs[1][2] );
    pixPos[2][2] = NDCToViewspace( upperLeftUV + g_ASSAOConsts.ViewportPixelSize * vec2( 1.0,  1.0 ), pixZs[2][2] );
    // left 2
    pixPos[0][1] = NDCToViewspace( upperLeftUV + g_ASSAOConsts.ViewportPixelSize * vec2( -1.0,  0.0), pixZs[0][1] );
    //pixPos[0][2] = NDCToViewspace( upperLeftUV + g_ASSAOConsts.ViewportPixelSize * vec2( -1.0,  1.0), pixZs[0][2] );
    // right 2                                                                                     
    //pixPos[3][1] = NDCToViewspace( upperLeftUV + g_ASSAOConsts.ViewportPixelSize * vec2(  2.0,  0.0), pixZs[3][1] );
    pixPos[3][2] = NDCToViewspace( upperLeftUV + g_ASSAOConsts.ViewportPixelSize * vec2(  2.0,  1.0), pixZs[3][2] );
    // top 2                                                                                       
    pixPos[1][0] = NDCToViewspace( upperLeftUV + g_ASSAOConsts.ViewportPixelSize * vec2( 0.0, -1.0 ), pixZs[1][0] );
    //pixPos[2][0] = NDCToViewspace( upperLeftUV + g_ASSAOConsts.ViewportPixelSize * vec2( 1.0, -1.0 ), pixZs[2][0] );
    // bottom 2                                                                                    
    //pixPos[1][3] = NDCToViewspace( upperLeftUV + g_ASSAOConsts.ViewportPixelSize * vec2( 0.0,  2.0 ), pixZs[1][3] );
    pixPos[2][3] = NDCToViewspace( upperLeftUV + g_ASSAOConsts.ViewportPixelSize * vec2( 1.0,  2.0 ), pixZs[2][3] );

    vec3 norm0 = CalculateNormal( edges0, pixPos[1][1], pixPos[0][1], pixPos[2][1], pixPos[1][0], pixPos[1][2] );
    vec3 norm3 = CalculateNormal( edges3, pixPos[2][2], pixPos[1][2], pixPos[3][2], pixPos[2][1], pixPos[2][3] );

    imageStore(g_NormalsOutputUAV, baseCoords + ivec2( 0, 0 ), vec4( norm0 * 0.5 + 0.5, 0.0 ));
    imageStore(g_NormalsOutputUAV, baseCoords + ivec2( 1, 1 ), vec4( norm3 * 0.5 + 0.5, 0.0 ));
}