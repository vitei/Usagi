#ifndef SHADOW_PASS

#ifdef SHADOW_READ
SAMPLER_LOC(2, 15) uniform sampler2DShadow sampler15;

BUFFER_LAYOUT(2, UBO_SHADOW_ID) uniform Shadow
{
    mat4        mShadowMtx;
    mat4        mShadowMtxInvView;
    vec4        vBias;
    vec2		vInvShadowDim;
};


float ShadowCalculationSoft(vec3 vEyePos)
{
    // FIXME: We should be able to take this transfrom into world space out
    vec4 vWorldPos = (vec4(vEyePos,1.0) * mInvViewMat);
    vec4 vShadowCoord = vWorldPos * mShadowMtx;

    vec2 vDiskRadius = 2.5 * vInvShadowDim.xy*vShadowCoord.w;
	float fShadow = 0.0;
	vec4 tc = vShadowCoord;
	for(int i = 0; i < 12; ++i)
	{
		float sampleBias = vBias.z * length(poissonDisk12[i]);
		tc.xy = vShadowCoord.xy + (poissonDisk12[i] * vDiskRadius);
		tc.z = vShadowCoord.z + sampleBias;
		fShadow += textureProj(sampler15, tc);
	}
	fShadow /= 12.0;  

    return fShadow;
}

float ShadowCalculation(vec3 vEyePos)
{
    // FIXME: We should be able to take this transfrom into world space out
    vec4 vWorldPos = (vec4(vEyePos,1.0) * mInvViewMat);
    vec4 vShadowCoord = vWorldPos * mShadowMtx;
    vShadowCoord.z += vBias.z;
    return textureProj( sampler15, vShadowCoord.xyzw );
}
#endif

#endif
