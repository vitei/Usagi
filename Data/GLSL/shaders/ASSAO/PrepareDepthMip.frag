#include "../includes/platformdefines.inc"
#include "../includes/assao.inc"


SAMPLER_LOC(1, 0) uniform sampler2D g_ViewspaceDepthSource;   // corresponds to SSAO_TEXTURE_SLOT0
SAMPLER_LOC(1, 1) uniform sampler2D g_ViewspaceDepthSource1;   // corresponds to SSAO_TEXTURE_SLOT1
SAMPLER_LOC(1, 2) uniform sampler2D g_ViewspaceDepthSource2;   // corresponds to SSAO_TEXTURE_SLOT2
SAMPLER_LOC(1, 3) uniform sampler2D g_ViewspaceDepthSource3;   // corresponds to SSAO_TEXTURE_SLOT3

layout(location = 0) out float out0;
layout(location = 1) out float out1;
layout(location = 2) out float out2;
layout(location = 3) out float out3;


// calculate effect radius and fit our screen sampling pattern inside it
void CalculateRadiusParameters( const float pixCenterLength, const vec2 pixelDirRBViewspaceSizeAtCenterZ, out float pixLookupRadiusMod, out float effectRadius, out float falloffCalcMulSq )
{
    effectRadius = g_ASSAOConsts.EffectRadius;

    // leaving this out for performance reasons: use something similar if radius needs to scale based on distance
    effectRadius *= clamp( pow( pixCenterLength, g_ASSAOConsts.RadiusDistScalingFunctionPow), 0.0, 1.0);

    // when too close, on-screen sampling disk will grow beyond screen size; limit this to avoid closeup temporal artifacts
    const float tooCloseLimitMod = clamp( pixCenterLength * g_ASSAOConsts.EffectSamplingRadiusNearLimitRec, 0.0, 1.0 ) * 0.8 + 0.2;
    
    effectRadius *= tooCloseLimitMod;

    // 0.85 is to reduce the radius to allow for more samples on a slope to still stay within influence
    pixLookupRadiusMod = (0.85 * effectRadius) / pixelDirRBViewspaceSizeAtCenterZ.x;

    // used to calculate falloff (both for AO samples and per-sample weights)
    falloffCalcMulSq= -1.0f / (effectRadius*effectRadius);
}


void PrepareDepthMip( const vec4 inPos/*, const vec2 inUV*/, int mipLevel, out float outD0, out float outD1, out float outD2, out float outD3 )
{
    ivec2 baseCoords = ivec2(inPos.xy) * 2;

    vec4 depthsArr[4];
    float depthsOutArr[4];
    
    // how to Gather a specific mip level?
    depthsArr[0].x = texelFetch(g_ViewspaceDepthSource, baseCoords + ivec2( 0, 0 ), 0).x ;// * g_ASSAOConsts.MaxViewspaceDepth;
    depthsArr[0].y = texelFetch(g_ViewspaceDepthSource, baseCoords + ivec2( 1, 0 ), 0).x ;// * g_ASSAOConsts.MaxViewspaceDepth;
    depthsArr[0].z = texelFetch(g_ViewspaceDepthSource, baseCoords + ivec2( 0, 1 ), 0).x ;// * g_ASSAOConsts.MaxViewspaceDepth;
    depthsArr[0].w = texelFetch(g_ViewspaceDepthSource, baseCoords + ivec2( 1, 1 ), 0).x ;// * g_ASSAOConsts.MaxViewspaceDepth;
    depthsArr[1].x = texelFetch(g_ViewspaceDepthSource1, baseCoords + ivec2( 0, 0 ), 0).x;// * g_ASSAOConsts.MaxViewspaceDepth;
    depthsArr[1].y = texelFetch(g_ViewspaceDepthSource1, baseCoords + ivec2( 1, 0 ), 0).x;// * g_ASSAOConsts.MaxViewspaceDepth;
    depthsArr[1].z = texelFetch(g_ViewspaceDepthSource1, baseCoords + ivec2( 0, 1 ), 0).x;// * g_ASSAOConsts.MaxViewspaceDepth;
    depthsArr[1].w = texelFetch(g_ViewspaceDepthSource1, baseCoords + ivec2( 1, 1 ), 0).x;// * g_ASSAOConsts.MaxViewspaceDepth;
    depthsArr[2].x = texelFetch(g_ViewspaceDepthSource2, baseCoords + ivec2( 0, 0 ), 0).x;// * g_ASSAOConsts.MaxViewspaceDepth;
    depthsArr[2].y = texelFetch(g_ViewspaceDepthSource2, baseCoords + ivec2( 1, 0 ), 0).x;// * g_ASSAOConsts.MaxViewspaceDepth;
    depthsArr[2].z = texelFetch(g_ViewspaceDepthSource2, baseCoords + ivec2( 0, 1 ), 0).x;// * g_ASSAOConsts.MaxViewspaceDepth;
    depthsArr[2].w = texelFetch(g_ViewspaceDepthSource2, baseCoords + ivec2( 1, 1 ), 0).x;// * g_ASSAOConsts.MaxViewspaceDepth;
    depthsArr[3].x = texelFetch(g_ViewspaceDepthSource3, baseCoords + ivec2( 0, 0 ), 0).x;// * g_ASSAOConsts.MaxViewspaceDepth;
    depthsArr[3].y = texelFetch(g_ViewspaceDepthSource3, baseCoords + ivec2( 1, 0 ), 0).x;// * g_ASSAOConsts.MaxViewspaceDepth;
    depthsArr[3].z = texelFetch(g_ViewspaceDepthSource3, baseCoords + ivec2( 0, 1 ), 0).x;// * g_ASSAOConsts.MaxViewspaceDepth;
    depthsArr[3].w = texelFetch(g_ViewspaceDepthSource3, baseCoords + ivec2( 1, 1 ), 0).x;// * g_ASSAOConsts.MaxViewspaceDepth;

    const uvec2 SVPosui         = uvec2( inPos.xy );
    const uint pseudoRandomA    = (SVPosui.x ) + 2 * (SVPosui.y );

    float dummyUnused1;
    float dummyUnused2;
    float falloffCalcMulSq, falloffCalcAdd;
 
    for( int i = 0; i < 4; i++ )
    {
        vec4 depths = depthsArr[i];

        float closest = min( min( depths.x, depths.y ), min( depths.z, depths.w ) );

        CalculateRadiusParameters( abs( closest ), vec2(1.0), dummyUnused1, dummyUnused2, falloffCalcMulSq );

        vec4 dists = depths - closest.xxxx;

        vec4 weights = clamp( dists * dists * falloffCalcMulSq + 1.0, 0.0, 1.0 );

        float smartAvg = dot( weights, depths ) / dot( weights, vec4( 1.0, 1.0, 1.0, 1.0 ) );

        const uint pseudoRandomIndex = ( pseudoRandomA + i ) % 4;

        //depthsOutArr[i] = closest;
        //depthsOutArr[i] = depths[ pseudoRandomIndex ];
        depthsOutArr[i] = smartAvg;
    }

    outD0 = depthsOutArr[0];
    outD1 = depthsOutArr[1];
    outD2 = depthsOutArr[2];
    outD3 = depthsOutArr[3];
}



void main()
{
    PrepareDepthMip( gl_FragCoord/*, inUV*/, MIP_LEVEL, out0, out1, out2, out3 );
}