#ifndef SHADOW_PASS

 // By including this file we are stating that we want shadows baby
//#ifdef PLATFORM_PC  // Not ready on switch yet
#define USE_SHADOW_MAP 1
//#endif




float GetShadowValue(vec4 shadowPos, in Cascade cascade, int cascadeIndex, in sampler2DArrayShadow shadowTex)
{
    float bias = cascade.vDepthBias[cascadeIndex];
    vec4 tc;
    tc.xyz = shadowPos.xyz / shadowPos.w;
    tc.z += bias;
    tc.w = float(cascadeIndex);

    float d = texture(shadowTex, tc.xywz);

    return d;
}

float GetShadowValueSoft(vec4 shadowPos, in Cascade cascade, int index, in sampler2DArrayShadow shadowTex)
{
    float bias = cascade.vDepthBias[index];
    vec4 tc;
    tc.w = float(index);

    float d = 0.0;
    vec2 vDiskRadius = cascade.vSampleRange[index] * cascade.vInvShadowDim.xy;
    for(int i=0; i<12; i++)
    {
        tc.xyz = (shadowPos.xyz / shadowPos.w);
        tc.xy += (poissonDisk12[i] * vDiskRadius);
        tc.z += (bias * length(poissonDisk12[i]));
        d += texture(shadowTex, tc.xywz);
    }

    return d / 12.0;
}

float SampleShadowmap(in float fViewDepth, in vec3 vPosition, in Cascade cascade, in sampler2DArrayShadow shadowTex)
{
    int index;
    float fadeStart = 100000000.0;
    float invFadeRange = 1.0;
    if (fViewDepth < cascade.vSplitDistance.x)
    {
        index = 0;
        fadeStart = cascade.vFadeSplitDist.x;
        invFadeRange = cascade.vInvFadeLength.x;
    }
    else if(fViewDepth < cascade.vSplitDistance.y)
    {
        index = 1;
        fadeStart = cascade.vFadeSplitDist.y;
        invFadeRange = cascade.vInvFadeLength.y;
    }
    else if(fViewDepth < cascade.vSplitDistance.z)
    {
        index = 2;
        fadeStart = cascade.vFadeSplitDist.z;
        invFadeRange = cascade.vInvFadeLength.z;
    }
    else
    {
        index = 3;
    }
    
    // FIXME: Ideally want to be using view positions
    // vec4 tc = vec4(vPosition.xyz, 1.0) * cascade.mCascadeMtxInvView[index];
    vec4 shadowPos = vec4(vPosition.xyz, 1.0) * cascade.mCascadeMtx[index];

    float d;
    if(fViewDepth > fadeStart)
    {
        // I experimented with splitting the samples between the two shadow maps but that creates
        // a jarring effect with low sample counts
        float alpha = (fViewDepth - fadeStart) * invFadeRange;
        vec4 shadowPosNext = vec4(vPosition.xyz, 1.0) * cascade.mCascadeMtx[index+1];
        d = GetShadowValueSoft(shadowPos, cascade, index, shadowTex) * (1.0 - alpha);
        d += GetShadowValueSoft(shadowPosNext, cascade, index+1, shadowTex) * (alpha);
    }
    else
    {
        d = GetShadowValueSoft(shadowPos, cascade, index, shadowTex);
    }

    // Fall off the shadow if it's beyond our maximum distance
    float decay = smoothstep(0.8 * cascade.vSplitDistance.w, cascade.vSplitDistance.w, fViewDepth);

    return max(d, decay);
}

#endif
