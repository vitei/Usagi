#ifndef SHADOW_PASS

SAMPLER_LOC(1, 15) uniform sampler2DArrayShadow sampler15;	// Keep this sitting at sampler 7 where it's unlikely to get overwritten

 // By including this file we are stating that we want shadows baby
//#ifdef PLATFORM_PC  // Not ready on switch yet
#define USE_SHADOW_MAP 1
//#endif

BUFFER_LAYOUT(1, UBO_SHADOW_ID) uniform Shadow
{
	mat4		mCascadeMtx[4];
    mat4        mCascadeMtxInvView[4];
	vec4		vSplitDistance;
    vec4        vFadeSplitDist;
    vec4        vInvFadeLength;
	vec4		vDepthBias;
    vec4        vSampleRange;
    vec2        vInvShadowDim;
};


float GetShadowValue(vec4 shadowPos, int index)
{
    float bias = vDepthBias[index];
    vec4 tc;
    tc.xyz = shadowPos.xyz / shadowPos.w;
    tc.z += bias;
    tc.w = float(index);

    float d = texture(sampler15, tc.xywz);

    return d;
}

float GetShadowValueSoft(vec4 shadowPos, int index)
{
    float bias = vDepthBias[index];
    vec4 tc;
    tc.w = float(index);

    float d = 0.0;
    vec2 vDiskRadius = vSampleRange[index] * vInvShadowDim.xy;
    for(int i=0; i<12; i++)
    {
        tc.xyz = (shadowPos.xyz / shadowPos.w);
        tc.xy += (poissonDisk12[i] * vDiskRadius);
        tc.z += (bias * length(poissonDisk12[i]));
        d += texture(sampler15, tc.xywz);
    }

    return d / 12.0;
}

float SampleShadowmap(in float fViewDepth, in vec3 vPosition)
{
    int index;
    float fadeStart = 100000000.0;
    float invFadeRange = 1.0;
    if (fViewDepth < vSplitDistance.x)
    {
        index = 0;
        fadeStart = vFadeSplitDist.x;
        invFadeRange = vInvFadeLength.x;
    }
    else if(fViewDepth < vSplitDistance.y)
    {
        index = 1;
        fadeStart = vFadeSplitDist.y;
        invFadeRange = vInvFadeLength.y;
    }
    else if(fViewDepth < vSplitDistance.z)
    {
        index = 2;
    }
    else
    {
        index = 3;
    }
    
    // FIXME: Ideally want to be using view positions
    // vec4 tc = vec4(vPosition.xyz, 1.0) * mCascadeMtxInvView[index];
    vec4 shadowPos = vec4(vPosition.xyz, 1.0) * mCascadeMtx[index];

    float d;
    if(fViewDepth > fadeStart)
    {
        // I experimented with splitting the samples between the two shadow maps but that creates
        // a jarring effect with low sample counts
        float alpha = (fViewDepth - fadeStart) * invFadeRange;
        vec4 shadowPosNext = vec4(vPosition.xyz, 1.0) * mCascadeMtx[index+1];
        d = GetShadowValueSoft(shadowPos, index) * (1.0 - alpha);
        d += GetShadowValueSoft(shadowPosNext, index+1) * (alpha);
    }
    else
    {
        d = GetShadowValueSoft(shadowPos, index);
    }

    // Fall off the shadow if it's beyond our maximum distance
    float decay = smoothstep(0.8 * vSplitDistance.w, vSplitDistance.w, fViewDepth);

    return max(d, decay);
}

#endif
