#include "../includes/platformdefines.inc"
#include "../includes/assao.inc"


// This was passed in as a compile define in the sample
#define SSAO_MAX_TAPS                               32
#define SSAO_MAX_REF_TAPS                           512
#define SSAO_ADAPTIVE_TAP_BASE_COUNT                5
#define SSAO_ADAPTIVE_TAP_FLEXIBLE_COUNT            (SSAO_MAX_TAPS-SSAO_ADAPTIVE_TAP_BASE_COUNT)
#define SSAO_DEPTH_MIP_LEVELS                       4

layout(location = 0) out vec2 colorOut;

SAMPLER_LOC(1, 0) uniform sampler2D g_ViewspaceDepthSource;   // corresponds to SSAO_TEXTURE_SLOT0
SAMPLER_LOC(1, 1) uniform sampler2D g_NormalmapSource;   // corresponds to SSAO_TEXTURE_SLOT1
SAMPLER_LOC(1, 2) uniform sampler1D g_LoadCounter;   // corresponds to SSAO_TEXTURE_SLOT2
SAMPLER_LOC(1, 3) uniform sampler2D g_ImportanceMap;   // corresponds to SSAO_TEXTURE_SLOT3
SAMPLER_LOC(1, 4) uniform sampler2DArray g_FinalSSAO;   // corresponds to SSAO_TEXTURE_SLOT4


vec3 DecodeNormal( vec3 encodedNormal )
{
    vec3 normal = encodedNormal * g_ASSAOConsts.NormalsUnpackMul.xxx + g_ASSAOConsts.NormalsUnpackAdd.xxx;

#if SSAO_ENABLE_NORMAL_WORLD_TO_VIEW_CONVERSION
    normal = mul( (vec3x3)g_ASSAOConsts.NormalsWorldToViewspaceMatrix, normal ).xyz; 
#endif

    // normal = normalize( normal );    // normalize adds around 2.5% cost on High settings but makes little (PSNR 66.7) visual difference when normals are as in the sample (stored in R8G8B8A8_UNORM,
    //                                  // decoded in the shader), however it will likely be required if using different encoding/decoding or the inputs are not normalized, etc.

    return normal;
}

vec4 CalculateEdges( const float centerZ, const float leftZ, const float rightZ, const float topZ, const float bottomZ )
{
    // slope-sensitive depth-based edge detection
    vec4 edgesLRTB = vec4( leftZ, rightZ, topZ, bottomZ ) - centerZ;
    vec4 edgesLRTBSlopeAdjusted = edgesLRTB + edgesLRTB.yxwz;
    edgesLRTB = min( abs( edgesLRTB ), abs( edgesLRTBSlopeAdjusted ) );
    return clamp( ( 1.3 - edgesLRTB / (centerZ * 0.040) ), 0.0, 1.0 );

    // cheaper version but has artifacts
    // edgesLRTB = abs( vec4( leftZ, rightZ, topZ, bottomZ ) - centerZ; );
    // return saturate( ( 1.3 - edgesLRTB / (pixZ * 0.06 + 0.1) ) );
}


vec3 NDCToViewspace( vec2 pos, float viewspaceDepth )
{
    vec3 ret;

    ret.xy = (g_ASSAOConsts.NDCToViewMul * pos.xy + g_ASSAOConsts.NDCToViewAdd) * viewspaceDepth;

    ret.z = viewspaceDepth;

    return ret;
}

float CalculatePixelObscurance( vec3 pixelNormal, vec3 hitDelta, float falloffCalcMulSq )
{
  float lengthSq = dot( hitDelta, hitDelta );
  float NdotD = dot(pixelNormal, hitDelta) / sqrt(lengthSq);

  float falloffMult = max( 0.0, lengthSq * falloffCalcMulSq + 1.0 );

  return max( 0, NdotD - g_ASSAOConsts.EffectHorizonAngleThreshold ) * falloffMult;
}



void SSAOTapInner( const int qualityLevel, inout float obscuranceSum, inout float weightSum, const vec2 samplingUV, const float mipLevel, const vec3 pixCenterPos, const vec3 negViewspaceDir,vec3 pixelNormal, const float falloffCalcMulSq, const float weightMod, const uint dbgTapIndex )
{
    // get depth at sample
    float viewspaceSampleZ = textureLod(g_ViewspaceDepthSource, samplingUV.xy, mipLevel ).x; // * g_ASSAOConsts.MaxViewspaceDepth;

    // convert to viewspace
    vec3 hitPos = NDCToViewspace( samplingUV.xy, viewspaceSampleZ ).xyz;
    vec3 hitDelta = hitPos - pixCenterPos;

    float obscurance = CalculatePixelObscurance( pixelNormal, hitDelta, falloffCalcMulSq );
    float weight = 1.0;
        
    if( qualityLevel >= SSAO_HALOING_REDUCTION_ENABLE_AT_QUALITY_PRESET )
    {
        //float reduct = max( 0, dot( hitDelta, negViewspaceDir ) );
        float reduct = max( 0, -hitDelta.z ); // cheaper, less correct version
        reduct = clamp( reduct * g_ASSAOConsts.NegRecEffectRadius + 2.0, 0.0, 1.0 ); // saturate( 2.0 - reduct / g_ASSAOConsts.EffectRadius );
        weight = SSAO_HALOING_REDUCTION_AMOUNT * reduct + (1.0 - SSAO_HALOING_REDUCTION_AMOUNT);
    }
    weight *= weightMod;
    obscuranceSum += obscurance * weight;
    weightSum += weight;
}


void SSAOTap( const int qualityLevel, inout float obscuranceSum, inout float weightSum, const uint tapIndex, const mat2x2 rotScale, const vec3 pixCenterPos, const vec3 negViewspaceDir, vec3 pixelNormal, const vec2 normalizedScreenPos, const float mipOffset, const float falloffCalcMulSq, float weightMod, vec2 normXY, float normXYLength )
{
    vec2  sampleOffset;
    float   samplePow2Len;

    // patterns
    {
        vec4 newSample = g_samplePatternMain[tapIndex];
        sampleOffset    = newSample.xy * rotScale;
        samplePow2Len   = newSample.w;                      // precalculated, same as: samplePow2Len = log2( length( newSample.xy ) );
        weightMod *= newSample.z;
    }

    // snap to pixel center (more correct obscurance math, avoids artifacts)
    sampleOffset                    = round(sampleOffset);

    // calculate MIP based on the sample distance from the centre, similar to as described 
    // in http://graphics.cs.williams.edu/papers/SAOHPG12/.
    float mipLevel = ( qualityLevel < SSAO_DEPTH_MIPS_ENABLE_AT_QUALITY_PRESET )?(0):(samplePow2Len + mipOffset);

    vec2 samplingUV = sampleOffset * g_ASSAOConsts.Viewport2xPixelSize + normalizedScreenPos;

    SSAOTapInner( qualityLevel, obscuranceSum, weightSum, samplingUV, mipLevel, pixCenterPos, negViewspaceDir, pixelNormal, falloffCalcMulSq, weightMod, tapIndex * 2 );

    // for the second tap, just use the mirrored offset
    vec2 sampleOffsetMirroredUV    = -sampleOffset;

    // tilt the second set of samples so that the disk is effectively rotated by the normal
    // effective at removing one set of artifacts, but too expensive for lower quality settings
    if( qualityLevel >= SSAO_TILT_SAMPLES_ENABLE_AT_QUALITY_PRESET )
    {
        float dotNorm = dot( sampleOffsetMirroredUV, normXY );
        sampleOffsetMirroredUV -= dotNorm * normXYLength * normXY;
        sampleOffsetMirroredUV = round(sampleOffsetMirroredUV);
    }
    
    // snap to pixel center (more correct obscurance math, avoids artifacts)
    vec2 samplingMirroredUV = sampleOffsetMirroredUV * g_ASSAOConsts.Viewport2xPixelSize + normalizedScreenPos;

    SSAOTapInner( qualityLevel, obscuranceSum, weightSum, samplingMirroredUV, mipLevel, pixCenterPos, negViewspaceDir, pixelNormal, falloffCalcMulSq, weightMod, tapIndex * 2 + 1 );
}


// calculate effect radius and fit our screen sampling pattern inside it
void CalculateRadiusParameters( const float pixCenterLength, const vec2 pixelDirRBViewspaceSizeAtCenterZ, out float pixLookupRadiusMod, out float effectRadius, out float falloffCalcMulSq )
{
    effectRadius = g_ASSAOConsts.EffectRadius;

    // leaving this out for performance reasons: use something similar if radius needs to scale based on distance
    //effectRadius *= pow( pixCenterLength, g_ASSAOConsts.RadiusDistanceScalingFunctionPow);

    // when too close, on-screen sampling disk will grow beyond screen size; limit this to avoid closeup temporal artifacts
    const float tooCloseLimitMod = clamp( pixCenterLength * g_ASSAOConsts.EffectSamplingRadiusNearLimitRec, 0.0, 1.0 ) * 0.8 + 0.2;
    
    effectRadius *= tooCloseLimitMod;

    // 0.85 is to reduce the radius to allow for more samples on a slope to still stay within influence
    pixLookupRadiusMod = (0.85 * effectRadius) / pixelDirRBViewspaceSizeAtCenterZ.x;

    // used to calculate falloff (both for AO samples and per-sample weights)
    falloffCalcMulSq= -1.0f / (effectRadius*effectRadius);
}


float PackEdges( vec4 edgesLRTB )
{
//    int4 edgesLRTBi = int4( saturate( edgesLRTB ) * 3.0 + 0.5 );
//    return ( (edgesLRTBi.x << 6) + (edgesLRTBi.y << 4) + (edgesLRTBi.z << 2) + (edgesLRTBi.w << 0) ) / 255.0;

    // optimized, should be same as above
    edgesLRTB = round( clamp( edgesLRTB, 0.0, 1.0 ) * 3.05 );
    return dot( edgesLRTB, vec4( 64.0 / 255.0, 16.0 / 255.0, 4.0 / 255.0, 1.0 / 255.0 ) ) ;
}


vec3 LoadNormal( ivec2 pos )
{
    vec3 encodedNormal = texelFetch(g_NormalmapSource, pos, 0 ).xyz;
    return DecodeNormal( encodedNormal );
}


// this function is designed to only work with half/half depth at the moment - there's a couple of hardcoded paths that expect pixel/texel size, so it will not work for full res
void GenerateSSAOShadowsInternal( out float outShadowTerm, out vec4 outEdges, out float outWeight, const vec2 SVPos/*, const vec2 normalizedScreenPos*/, int qualityLevel, bool adaptiveBase )
{
    vec2 SVPosRounded = trunc( SVPos );
    uvec2 SVPosui = uvec2( SVPosRounded ); //same as uvec2( SVPos )

    const uint numberOfTaps = (adaptiveBase)?(SSAO_ADAPTIVE_TAP_BASE_COUNT) : ( g_numTaps[qualityLevel] );
    float pixZ, pixLZ, pixTZ, pixRZ, pixBZ;

    vec4 valuesUL     = textureGather(g_ViewspaceDepthSource, SVPosRounded * g_ASSAOConsts.HalfViewportPixelSize, 0 );
    vec4 valuesBR     = textureGatherOffset( g_ViewspaceDepthSource, SVPosRounded * g_ASSAOConsts.HalfViewportPixelSize, ivec2( 1, 1 ), 0 );

    // get this pixel's viewspace depth
    pixZ = valuesUL.y; //float pixZ = g_ViewspaceDepthSource.SampleLevel( g_PointMirrorSampler, normalizedScreenPos, 0.0 ).x; // * g_ASSAOConsts.MaxViewspaceDepth;

    // get left right top bottom neighbouring pixels for edge detection (gets compiled out on qualityLevel == 0)
    pixLZ   = valuesUL.x;
    pixTZ   = valuesUL.z;
    pixRZ   = valuesBR.z;
    pixBZ   = valuesBR.x;

    vec2 normalizedScreenPos = SVPosRounded * g_ASSAOConsts.Viewport2xPixelSize + g_ASSAOConsts.Viewport2xPixelSize_x_025;
    vec3 pixCenterPos = NDCToViewspace( normalizedScreenPos, pixZ ); // g

    // Load this pixel's viewspace normal
    ivec2 fullResCoord = ivec2(SVPosui * 2 + g_ASSAOConsts.PerPassFullResCoordOffset.xy);
    vec3 pixelNormal = LoadNormal( fullResCoord );

    const vec2 pixelDirRBViewspaceSizeAtCenterZ = pixCenterPos.z * g_ASSAOConsts.NDCToViewMul * g_ASSAOConsts.Viewport2xPixelSize;  // optimized approximation of:  vec2 pixelDirRBViewspaceSizeAtCenterZ = NDCToViewspace( normalizedScreenPos.xy + g_ASSAOConsts.ViewportPixelSize.xy, pixCenterPos.z ).xy - pixCenterPos.xy;

    float pixLookupRadiusMod;
    float falloffCalcMulSq;

    // calculate effect radius and fit our screen sampling pattern inside it
    float effectViewspaceRadius;
    CalculateRadiusParameters( length( pixCenterPos ), pixelDirRBViewspaceSizeAtCenterZ, pixLookupRadiusMod, effectViewspaceRadius, falloffCalcMulSq );

    // calculate samples rotation/scaling
    mat2x2 rotScale;
    {
        // reduce effect radius near the screen edges slightly; ideally, one would render a larger depth buffer (5% on each side) instead
        if( !adaptiveBase && (qualityLevel >= SSAO_REDUCE_RADIUS_NEAR_SCREEN_BORDER_ENABLE_AT_QUALITY_PRESET) )
        {
            float nearScreenBorder = min( min( normalizedScreenPos.x, 1.0 - normalizedScreenPos.x ), min( normalizedScreenPos.y, 1.0 - normalizedScreenPos.y ) );
            nearScreenBorder = clamp( 10.0 * nearScreenBorder + 0.6, 0.0, 1.0 );
            pixLookupRadiusMod *= nearScreenBorder;
        }

        // load & update pseudo-random rotation matrix
        uint pseudoRandomIndex = uint( SVPosRounded.y * 2 + SVPosRounded.x ) % 5;
        vec4 rs = g_ASSAOConsts.PatternRotScaleMatrices[ pseudoRandomIndex ];
        rotScale = mat2x2( rs.x * pixLookupRadiusMod, rs.y * pixLookupRadiusMod, rs.z * pixLookupRadiusMod, rs.w * pixLookupRadiusMod );
    }

    // the main obscurance & sample weight storage
    float obscuranceSum = 0.0;
    float weightSum = 0.0;

    // edge mask for between this and left/right/top/bottom neighbour pixels - not used in quality level 0 so initialize to "no edge" (1 is no edge, 0 is edge)
    vec4 edgesLRTB = vec4( 1.0, 1.0, 1.0, 1.0 );

    // Move center pixel slightly towards camera to avoid imprecision artifacts due to using of 16bit depth buffer; a lot smaller offsets needed when using 32bit floats
    pixCenterPos *= g_ASSAOConsts.DepthPrecisionOffsetMod;

    if( !adaptiveBase && (qualityLevel >= SSAO_DEPTH_BASED_EDGES_ENABLE_AT_QUALITY_PRESET) )
    {
        edgesLRTB = CalculateEdges( pixZ, pixLZ, pixRZ, pixTZ, pixBZ );
    }

    // adds a more high definition sharp effect, which gets blurred out (reuses left/right/top/bottom samples that we used for edge detection)
    if( !adaptiveBase && (qualityLevel >= SSAO_DETAIL_AO_ENABLE_AT_QUALITY_PRESET) )
    {
        // disable in case of quality level 4 (reference)
        if( qualityLevel != 4 )
        {
            //approximate neighbouring pixels positions (actually just deltas or "positions - pixCenterPos" )
            vec3 viewspaceDirZNormalized = vec3( pixCenterPos.xy / pixCenterPos.zz, 1.0 );
            vec3 pixLDelta  = vec3( -pixelDirRBViewspaceSizeAtCenterZ.x, 0.0, 0.0 ) + viewspaceDirZNormalized * (pixLZ - pixCenterPos.z); // very close approximation of: vec3 pixLPos  = NDCToViewspace( normalizedScreenPos + vec2( -g_ASSAOConsts.HalfViewportPixelSize.x, 0.0 ), pixLZ ).xyz - pixCenterPos.xyz;
            vec3 pixRDelta  = vec3( +pixelDirRBViewspaceSizeAtCenterZ.x, 0.0, 0.0 ) + viewspaceDirZNormalized * (pixRZ - pixCenterPos.z); // very close approximation of: vec3 pixRPos  = NDCToViewspace( normalizedScreenPos + vec2( +g_ASSAOConsts.HalfViewportPixelSize.x, 0.0 ), pixRZ ).xyz - pixCenterPos.xyz;
            vec3 pixTDelta  = vec3( 0.0, -pixelDirRBViewspaceSizeAtCenterZ.y, 0.0 ) + viewspaceDirZNormalized * (pixTZ - pixCenterPos.z); // very close approximation of: vec3 pixTPos  = NDCToViewspace( normalizedScreenPos + vec2( 0.0, -g_ASSAOConsts.HalfViewportPixelSize.y ), pixTZ ).xyz - pixCenterPos.xyz;
            vec3 pixBDelta  = vec3( 0.0, +pixelDirRBViewspaceSizeAtCenterZ.y, 0.0 ) + viewspaceDirZNormalized * (pixBZ - pixCenterPos.z); // very close approximation of: vec3 pixBPos  = NDCToViewspace( normalizedScreenPos + vec2( 0.0, +g_ASSAOConsts.HalfViewportPixelSize.y ), pixBZ ).xyz - pixCenterPos.xyz;

            const float rangeReductionConst         = 4.0f;                         // this is to avoid various artifacts
            const float modifiedFalloffCalcMulSq    = rangeReductionConst * falloffCalcMulSq;

            vec4 additionalObscurance;
            additionalObscurance.x = CalculatePixelObscurance( pixelNormal, pixLDelta, modifiedFalloffCalcMulSq );
            additionalObscurance.y = CalculatePixelObscurance( pixelNormal, pixRDelta, modifiedFalloffCalcMulSq );
            additionalObscurance.z = CalculatePixelObscurance( pixelNormal, pixTDelta, modifiedFalloffCalcMulSq );
            additionalObscurance.w = CalculatePixelObscurance( pixelNormal, pixBDelta, modifiedFalloffCalcMulSq );

            obscuranceSum += g_ASSAOConsts.DetailAOStrength * dot( additionalObscurance, edgesLRTB );
        }
    }

    // Sharp normals also create edges - but this adds to the cost as well
    if( !adaptiveBase && (qualityLevel >= SSAO_NORMAL_BASED_EDGES_ENABLE_AT_QUALITY_PRESET ) )
    {
	    vec3 neighbourNormalL = DecodeNormal( texelFetchOffset(g_NormalmapSource, fullResCoord, 0, ivec2( -2,  0 ) ).xyz );
	    vec3 neighbourNormalR = DecodeNormal( texelFetchOffset(g_NormalmapSource, fullResCoord, 0, ivec2(  2,  0 ) ).xyz );
	    vec3 neighbourNormalT = DecodeNormal( texelFetchOffset(g_NormalmapSource, fullResCoord, 0, ivec2(  0, -2 ) ).xyz );
	    vec3 neighbourNormalB = DecodeNormal( texelFetchOffset(g_NormalmapSource, fullResCoord, 0, ivec2(  0,  2 ) ).xyz );

        const float dotThreshold = SSAO_NORMAL_BASED_EDGES_DOT_THRESHOLD;

        vec4 normalEdgesLRTB;
        normalEdgesLRTB.x = clamp( (dot( pixelNormal, neighbourNormalL ) + dotThreshold ), 0.0, 1.0 );
        normalEdgesLRTB.y = clamp( (dot( pixelNormal, neighbourNormalR ) + dotThreshold ), 0.0, 1.0 );
        normalEdgesLRTB.z = clamp( (dot( pixelNormal, neighbourNormalT ) + dotThreshold ), 0.0, 1.0 );
        normalEdgesLRTB.w = clamp( (dot( pixelNormal, neighbourNormalB ) + dotThreshold ), 0.0, 1.0 );

//#define SSAO_SMOOTHEN_NORMALS // fixes some aliasing artifacts but kills a lot of high detail and adds to the cost - not worth it probably but feel free to play with it
#ifdef SSAO_SMOOTHEN_NORMALS
        //neighbourNormalL  = LoadNormal( fullResCoord, ivec2( -1,  0 ) );
        //neighbourNormalR  = LoadNormal( fullResCoord, ivec2(  1,  0 ) );
        //neighbourNormalT  = LoadNormal( fullResCoord, ivec2(  0, -1 ) );
        //neighbourNormalB  = LoadNormal( fullResCoord, ivec2(  0,  1 ) );
        pixelNormal += neighbourNormalL * edgesLRTB.x + neighbourNormalR * edgesLRTB.y + neighbourNormalT * edgesLRTB.z + neighbourNormalB * edgesLRTB.w;
        pixelNormal = normalize( pixelNormal );
#endif

        edgesLRTB *= normalEdgesLRTB;
    }



    const float globalMipOffset     = SSAO_DEPTH_MIPS_GLOBAL_OFFSET;
    float mipOffset = ( qualityLevel < SSAO_DEPTH_MIPS_ENABLE_AT_QUALITY_PRESET ) ? ( 0 ) : ( log2( pixLookupRadiusMod ) + globalMipOffset );

    // Used to tilt the second set of samples so that the disk is effectively rotated by the normal
    // effective at removing one set of artifacts, but too expensive for lower quality settings
    vec2 normXY = vec2( pixelNormal.x, pixelNormal.y );
    float normXYLength = length( normXY );
    normXY /= vec2( normXYLength, -normXYLength );
    normXYLength *= SSAO_TILT_SAMPLES_AMOUNT;

    const vec3 negViewspaceDir = -normalize( pixCenterPos );

    // standard, non-adaptive approach
    if( (qualityLevel != 3) || adaptiveBase )
    {
        // [unroll] // <- doesn't seem to help on any platform, although the compilers seem to unroll anyway if const number of tap used!
        for( uint i = 0; i < numberOfTaps; i++ )
        {
            SSAOTap( qualityLevel, obscuranceSum, weightSum, i, rotScale, pixCenterPos, negViewspaceDir, pixelNormal, normalizedScreenPos, mipOffset, falloffCalcMulSq, 1.0, normXY, normXYLength );
        }
    }
    else // if( qualityLevel == 3 ) adaptive approach
    {
        // add new ones if needed
        vec2 fullResUV = normalizedScreenPos + g_ASSAOConsts.PerPassFullResUVOffset.xy;
        float importance = textureLod(g_ImportanceMap, fullResUV, 0.0 ).x;

        // this is to normalize SSAO_DETAIL_AO_AMOUNT across all pixel regardless of importance
        obscuranceSum *= (SSAO_ADAPTIVE_TAP_BASE_COUNT / float(SSAO_MAX_TAPS)) + (importance * SSAO_ADAPTIVE_TAP_FLEXIBLE_COUNT / float(SSAO_MAX_TAPS));

        // load existing base values
        vec2 baseValues = texelFetch(g_FinalSSAO, ivec3( SVPosui, g_ASSAOConsts.PassIndex), 0 ).xy;
        weightSum += baseValues.y * float(SSAO_ADAPTIVE_TAP_BASE_COUNT * 4.0);
        obscuranceSum += (baseValues.x) * weightSum;
        
        // increase importance around edges
        float edgeCount = dot( 1.0-edgesLRTB, vec4( 1.0, 1.0, 1.0, 1.0 ) );
        //importance += edgeCount / (float)SSAO_ADAPTIVE_TAP_FLEXIBLE_COUNT;

        float avgTotalImportance = float(texelFetch(g_LoadCounter, 0, 0) * g_ASSAOConsts.LoadCounterAvgDiv);

        float importanceLimiter = clamp( g_ASSAOConsts.AdaptiveSampleCountLimit / avgTotalImportance, 0.0, 1.0 );
        importance *= importanceLimiter;

        float additionalSampleCountFlt = SSAO_ADAPTIVE_TAP_FLEXIBLE_COUNT * importance;

        const float blendRange = 3.0; // use 1 to just blend the last one; use larger number to blend over more for a more smooth transition
        const float blendRangeInv = 1.0 / blendRange;

        additionalSampleCountFlt += 0.5;
        uint additionalSamples   = uint( additionalSampleCountFlt );
        uint additionalSamplesTo = min( SSAO_MAX_TAPS, additionalSamples + SSAO_ADAPTIVE_TAP_BASE_COUNT );

        // additional manual unroll doesn't help unfortunately
        for( uint i = SSAO_ADAPTIVE_TAP_BASE_COUNT; i < additionalSamplesTo; i++ )
        {
            additionalSampleCountFlt -= 1.0f;
            float weightMod = clamp(additionalSampleCountFlt * blendRangeInv, 0.0, 1.0); // slowly blend in the last few samples
            SSAOTap( qualityLevel, obscuranceSum, weightSum, i, rotScale, pixCenterPos, negViewspaceDir, pixelNormal, normalizedScreenPos, mipOffset, falloffCalcMulSq, weightMod, normXY, normXYLength );
        }
    }

    // early out for adaptive base - just output weight (used for the next pass)
    if( adaptiveBase )
    {
        float obscurance = obscuranceSum / weightSum;

        outShadowTerm   = obscurance;
        outEdges        = vec4(0);
        outWeight       = weightSum;
        return;
    }

    // calculate weighted average
    float obscurance = obscuranceSum / weightSum;

    // calculate fadeout (1 close, gradient, 0 far)
    float fadeOut = clamp( pixCenterPos.z * g_ASSAOConsts.EffectFadeOutMul + g_ASSAOConsts.EffectFadeOutAdd, 0.0, 1.0 );
  
    // Reduce the SSAO shadowing if we're on the edge to remove artifacts on edges (we don't care for the lower quality one)
    if( !adaptiveBase && (qualityLevel >= SSAO_DEPTH_BASED_EDGES_ENABLE_AT_QUALITY_PRESET) )
    {
        // float edgeCount = dot( 1.0-edgesLRTB, vec4( 1.0, 1.0, 1.0, 1.0 ) );

        // when there's more than 2 opposite edges, start fading out the occlusion to reduce aliasing artifacts
        float edgeFadeoutFactor = clamp( (1.0 - edgesLRTB.x - edgesLRTB.y) * 0.35, 0.0, 1.0) + clamp( (1.0 - edgesLRTB.z - edgesLRTB.w) * 0.35, 0.0, 1.0 );

        // (experimental) if you want to reduce the effect next to any edge
        // edgeFadeoutFactor += 0.1 * saturate( dot( 1 - edgesLRTB, vec4( 1, 1, 1, 1 ) ) );

        fadeOut *= clamp( 1.0 - edgeFadeoutFactor, 0.0, 1.0 );
    }
    
    // same as a bove, but a lot more conservative version
    // fadeOut *= saturate( dot( edgesLRTB, vec4( 0.9, 0.9, 0.9, 0.9 ) ) - 2.6 );

    // strength
    obscurance = g_ASSAOConsts.EffectShadowStrength * obscurance;
    
    // clamp
    obscurance = min( obscurance, g_ASSAOConsts.EffectShadowClamp );
    
    // fadeout
    obscurance *= fadeOut;

    // conceptually switch to occlusion with the meaning being visibility (grows with visibility, occlusion == 1 implies full visibility), 
    // to be in line with what is more commonly used.
    float occlusion = 1.0 - obscurance;

    // modify the gradient
    // note: this cannot be moved to a later pass because of loss of precision after storing in the render target
    occlusion = pow( clamp( occlusion, 0.0, 1.0 ), g_ASSAOConsts.EffectShadowPow );

    // outputs!
    outShadowTerm   = occlusion;    // Our final 'occlusion' term (0 means fully occluded, 1 means fully lit)
    outEdges        = edgesLRTB;    // These are used to prevent blurring across edges, 1 means no edge, 0 means edge, 0.5 means half way there, etc.
    outWeight       = weightSum;
}



// PSGenerateQ0
void main()
{
    float   outShadowTerm;
    float   outWeight;
    vec4  outEdges;
    GenerateSSAOShadowsInternal( outShadowTerm, outEdges, outWeight, gl_FragCoord.xy/*, inUV*/, 0, false );
    colorOut.x = outShadowTerm;
    colorOut.y = PackEdges( vec4( 1, 1, 1, 1 ) ); // no edges in low quality
}