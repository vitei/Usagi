#ifndef _DEFERRED_LIGHTING_INC
#define _DEFERRED_LIGHTING_INC


in vec2 vo_vTexCoord;
in vec3 vo_vFSVector;



layout(location = 0) out vec4 colorOut;

void CalculateDirectionalLighting(int iLight, vec3 vNormal, vec3 vViewDir, float fSpecPow, inout vec4 vDiffuse, inout vec4 vSpecular)
{
	vec3 vLightDir		= lights[iLight].vDirection.xyz;
	float nDotL			= max( dot( vNormal, vLightDir ), 0.0); 
   
	vec3  vReflection	= reflect( vNormal, - vLightDir ); 
	float fRDotV		= max( 0.0, dot( vReflection, vViewDir ) );

	float fSpecular = ( pow( fRDotV, fSpecPow ) );

	vDiffuse  += lights[iLight].vDiffuse*nDotL;
	vSpecular += lights[iLight].vSpecular*fSpecular;
}

void main(void)
{
	vec3 vPos, vNormal, vColor, vSpecCol;
	float fSpecPow;
	GetDeferredDataSpecFS(vo_vTexCoord, vo_vFSVector, vPos, vNormal, vColor, vSpecCol, fSpecPow);

	vec3 vFSVector = normalize(vo_vFSVector);
	vec4 vSpecular = vec4(0.0f, 0.0f, 0.0f, 0.0f);
	vec4 vDiffuse = vec4(0.0f, 0.0f, 0.0f, 0.0f);

	CalculateDirectionalLighting(LIGHT_INDEX, vNormal, vFSVector, fSpecPow, vDiffuse, vSpecular);
	
	if(vPos.z < vSplitDistance.w)
	{
		vec3 vWorldPos = (vec4(vPos,1.0) * mInvViewMat).xyz;
		float fShadow = SampleShadowmap(vPos.z, vWorldPos.xyz);
		vec4 vShadowMul = mix(vShadowColor, vec4(1.0), fShadow);
		
		vDiffuse *= vShadowMul;
		vSpecular *= vShadowMul;
	}

	colorOut = vec4((vColor * (vDiffuse.xyz)) + (vSpecular.xyz * vSpecCol), 1.0);
}

#endif

