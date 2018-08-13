#ifndef _GLOBAL_MATRICS_INC
#define _GLOBAL_MATRICS_INC

// use binding ... to make use of that

#define USE_SKYBOX_FOG
#define CASCADE_COUNT 4

struct Cascade
{
	mat4		mCascadeMtx[CASCADE_COUNT];
    mat4        mCascadeMtxInvView[CASCADE_COUNT];
	vec4		vSplitDistance;
    vec4        vFadeSplitDist;
    vec4        vInvFadeLength;
	vec4		vDepthBias;
    vec4        vSampleRange;
    vec2        vInvShadowDim;
};

#ifndef SHADOW_PASS
BUFFER_LAYOUT(0, UBO_GLOBAL_ID) uniform Globals
{
	mat4 	mProjMat;
	mat4 	mViewMat;
	mat4 	mInvViewMat;
	mat4 	mViewProjMat;
	vec4	vFogVars;
	vec4	vFogColor;
	vec4	vEyePos;
	vec4	vLookDir;
	vec4	vSceneOffset;
	// Fragment specific variables
	mat4	mInvProjMat;
	vec4	vNearFar;
	vec4	vHemSkyColor;
	vec4	vHemGroundColor;
	vec4	vHemisphereDir;
	vec4	vShadowColor;
};
#elif defined(OMNI_DEPTH)
BUFFER_LAYOUT(0, UBO_GLOBAL_ID) uniform Globals
{
	mat4 	mMatrices[6];
	vec4	lightPos;
	float 	lightFarDist;
};
#else
BUFFER_LAYOUT(0, UBO_GLOBAL_ID) uniform Globals
{
	mat4 	mProjMat;
	mat4 	mViewMat;
};
#endif


#ifndef OVERRIDE_TRANS
BUFFER_LAYOUT(1, UBO_INSTANCE_ID) uniform Instance
{
	mat3x4	mModelMat;
};
#endif


#ifndef SHADOW_PASS
float CalculateLinearFog(in vec3 vViewDir)
{
	// TODO: Confirm nrm doesn't produce nan on zero length
	vec3 	vNormalised = normalize(vViewDir);
	float 	fDist = dot(vViewDir, vNormalised);
	float	fFog = fDist - vFogVars.x;
	fFog = fFog  * vFogVars.y;

	fFog = clamp(fFog, 0.0, 1.0);


	return fFog;
}


vec4 CalculateColourWithFog(vec3 vColor, float fog)
{     
 #ifdef USE_SKYBOX_FOG     
    return vec4(vColor.rgb, fog);
 #else
 	vec3 vFog = vec3(0.5, 0.5, 1.0);	// TODO: Removed hardcoding
	return vec4( (vColor*(1-fog)) + (vFog*fog) , 1.0);
 #endif
}
#endif


#endif

