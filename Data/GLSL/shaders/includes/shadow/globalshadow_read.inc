#ifndef SHADOW_PASS

 // By including this file we are stating that we want shadows baby
//#ifdef PLATFORM_PC  // Not ready on switch yet
#define USE_SHADOW_MAP 1
//#endif




float GetShadowValue(vec4 shadowPos, in int shadowIdx, int cascadeIndex)
{
    float bias = cascades[shadowIdx].vDepthBias[cascadeIndex];
    vec4 tc;
    tc.xyz = shadowPos.xyz / shadowPos.w;
    tc.z += bias;
    tc.w = float( (CASCADE_COUNT*shadowIdx) + cascadeIndex);

    float d = texture(sampler15, tc.xywz);

    return d;
}

float GetShadowValueSoft(vec4 shadowPos, in int shadowIdx, int index)
{
    float bias = cascades[shadowIdx].vDepthBias[index];
    vec4 tc;
    tc.w = float( (CASCADE_COUNT*shadowIdx) + index );

    float d = 0.0;
    vec2 vDiskRadius = cascades[shadowIdx].vSampleRange[index] * cascades[shadowIdx].vInvShadowDim.xy;
    for(int i=0; i<12; i++)
    {
        tc.xyz = (shadowPos.xyz / shadowPos.w);
        tc.xy += (poissonDisk12[i] * vDiskRadius);
        tc.z += (bias * length(poissonDisk12[i]));
        d += texture(sampler15, tc.xywz);
    }

    return d / 12.0;
}

float SampleShadowmap(in float fViewDepth, in vec3 vPosition, in int shadowIdx)
{
    int index;
    float fadeStart = 100000000.0;
    float invFadeRange = 1.0;
    if (fViewDepth < cascades[shadowIdx].vSplitDistance.x)
    {
        index = 0;
        fadeStart = cascades[shadowIdx].vFadeSplitDist.x;
        invFadeRange = cascades[shadowIdx].vInvFadeLength.x;
    }
    else if(fViewDepth < cascades[shadowIdx].vSplitDistance.y)
    {
        index = 1;
        fadeStart = cascades[shadowIdx].vFadeSplitDist.y;
        invFadeRange = cascades[shadowIdx].vInvFadeLength.y;
    }
    else if(fViewDepth < cascades[shadowIdx].vSplitDistance.z)
    {
        index = 2;
        fadeStart = cascades[shadowIdx].vFadeSplitDist.z;
        invFadeRange = cascades[shadowIdx].vInvFadeLength.z;
    }
    else
    {
        index = 3;
    }
    
    // FIXME: Ideally want to be using view positions
    // vec4 tc = vec4(vPosition.xyz, 1.0) * cascades[shadowIdx].mCascadeMtxInvView[index];
    vec4 shadowPos = vec4(vPosition.xyz, 1.0) * cascades[shadowIdx].mCascadeMtx[index];

    float d;
    if(fViewDepth > fadeStart)
    {
        // I experimented with splitting the samples between the two shadow maps but that creates
        // a jarring effect with low sample counts
        float alpha = (fViewDepth - fadeStart) * invFadeRange;
        vec4 shadowPosNext = vec4(vPosition.xyz, 1.0) * cascades[shadowIdx].mCascadeMtx[index+1];
        d = GetShadowValueSoft(shadowPos, shadowIdx, index) * (1.0 - alpha);
        d += GetShadowValueSoft(shadowPosNext, shadowIdx, index+1) * (alpha);
    }
    else
    {
        d = GetShadowValueSoft(shadowPos, shadowIdx, index);
    }

    // Fall off the shadow if it's beyond our maximum distance
    float decay = smoothstep(0.8 * cascades[shadowIdx].vSplitDistance.w, cascades[shadowIdx].vSplitDistance.w, fViewDepth);

    return max(d, decay);
}

#endif
