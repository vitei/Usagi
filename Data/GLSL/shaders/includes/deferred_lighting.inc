#ifndef _DEFERRED_LIGHTING_INC
#define _DEFERRED_LIGHTING_INC


ATTRIB_LOC(0) in vec2 vo_vTexCoord;
ATTRIB_LOC(1) in vec3 vo_vFSVector;


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


	int count = iCascadeLightStart;

	if(iCascadeLightStart < iDirLightCount)
	{
		// Apply the first shadowed light
		CalculateDirectionalLighting(iCascadeLightStart, vNormal, vFSVector, fSpecPow, vDiffuse, vSpecular);	
		if(vPos.z < cascades[0].vSplitDistance.w)
		{
			vec3 vWorldPos = (vec4(vPos,1.0) * mInvViewMat).xyz;
			float fShadow = SampleShadowmap(vPos.z, vWorldPos.xyz, 0);
			vec4 vShadowMul = mix(vShadowColor, vec4(1.0), fShadow);

			vDiffuse *= vShadowMul;
			vSpecular *= vShadowMul;
		}

	}

	for(int i=0; i<count; i++)
	{
		CalculateDirectionalLighting(i, vNormal, vFSVector, fSpecPow, vDiffuse, vSpecular);
	}

	vec3 vEmissive = GetDeferredEmissive(vo_vTexCoord);
	colorOut = vec4((vColor * ((vDiffuse.xyz) + vAmbientLight.xyz )) + (vSpecular.xyz * vSpecCol) + vEmissive, 1.0);
}

#endif

