#ifndef _FORWARD_LIGHTING_INC
#define _FORWARD_LIGHTING_INC



BUFFER_LAYOUT(1, UBO_MATERIAL_1_ID) uniform Material1
{
	// Materials
	vec4	vEmission;
	vec4	vAmbient;
	vec4	vDiffuse;
	vec4	vSpecular0;
	vec4	vSpecular1;
	float	fAlphaRef;
} uMaterial;


void ApplyMaterial( inout vec4 vDiffuse, inout vec4 vSpecular )
{
	vDiffuse *= uMaterial.vDiffuse;
	vSpecular *= uMaterial.vSpecular0;
}

// TODO: Remove hardcodeing
const float gfSpecularPower = 16.0;


void CalculatePointLighting(int iLight, vec3 vPos, vec3 vNormal, vec3 vViewDir, inout vec4 vDiffuse, inout vec4 vSpecular)
{
	vec3 vLightDir = vec3( lights[iLight].vPosition.xyz -  vPos.xyz );

	if( dot(vLightDir, vLightDir) < lights[iLight].vRange.z )
	{
		float d = length(vLightDir);
		float fAttenuation;

		if(lights[iLight].iAttenuationLUT >= 0)
		{
			// The falloff is done with a lookup table
			fAttenuation = texture(lightLUT[lights[iLight].iAttenuationLUT],  d / lights[iLight].vRange.y ).r;
			//vDiffuse += vec4(0.0, 0.0, 1.0*fAttenuation, 0.0);
			//fAttenuation = 0.0;
		}
		else
		{
			// Obey the inner / outer settings
			fAttenuation = 1.0-smoothstep(lights[iLight].vRange.w, lights[iLight].vRange.y, d);
		}

		vLightDir /= d;	// Normalize the vector

		float nDotLD = max(0.0, dot(vNormal, vLightDir));

		vec3 refl = normalize(-reflect(vLightDir,vNormal));
	                 
	    // TODO: Add the ambient colour of the light
	    float NdotHV = max(dot(vNormal,refl),0.0);
	    vSpecular.xyz += fAttenuation * lights[iLight].vSpecular0.xyz * pow(NdotHV, gfSpecularPower);		

		vDiffuse.xyz += ((lights[iLight].vDiffuse.xyz * nDotLD) + lights[iLight].vAmbient.rgb) * fAttenuation;
	}
}

void CalculateDirectionalLighting(int iLight, vec3 vNormal, vec3 vViewDir, inout vec4 vDiffuse, inout vec4 vSpecular)
{
	vec3 vLightDir		= lights[iLight].vDirection.xyz;
	float nDotL			= max( dot( vNormal, vLightDir ), 0.0); 
   
	vec3  vReflection	= reflect( vNormal, - vLightDir ); 
	float fRDotV		= max( 0.0, dot( vReflection, vViewDir ) );

	float fSpecular = ( pow( fRDotV, gfSpecularPower ) );

	vDiffuse  += lights[iLight].vDiffuse*nDotL;
	vSpecular += lights[iLight].vSpecular0*fSpecular;
	//vSpecular += lights[iLight].vSpecular1*fSpecular;
}


void CalculateSpotLighting(int iLight, vec3 vPos, vec3 vNormal, vec3 vViewDir, inout vec4 vDiffuse, inout vec4 vSpecular)
{	
	vec3 vLightDir = vec3( lights[iLight].vPosition.xyz -  vPos.xyz );
	float d = length(vLightDir);
	
	float fAttenuation = 1.0f;
	
	if(lights[iLight].iAttenuationLUT >= 0)
	{
		// Look-up table
		fAttenuation = texture(lightLUT[lights[iLight].iAttenuationLUT],  d / lights[iLight].vRange.y ).r;
	}
	else if(lights[iLight].vRange.y > 0.0f)
	{
		//
		vec3 vScaledLightDir = vLightDir.xyz * lights[iLight].vRange.x;
		fAttenuation = clamp(1.0 - dot(vScaledLightDir, vScaledLightDir), 0.0, 1.0);
	}

	vLightDir /= d;	// Normalize the vector

	float nDotLD = max(0.0, dot(vNormal, vLightDir));
	float spotEffect = dot( lights[iLight].vDirection.xyz, -vLightDir );
	vec3 vSpecColor = vec3(0.0, 0.0, 0.0);

	if(nDotLD > 0.0 && spotEffect > lights[iLight].fCosSpotCutoff || lights[iLight].iSpotLUT >= 0)
	{

		if(lights[iLight].iSpotLUT >= 0)
		{
			fAttenuation = texture(lightLUT[lights[iLight].iSpotLUT],  spotEffect ).r;
		}
		else
		{
			spotEffect = min(lights[iLight].fCosInnerSpotCutoff, spotEffect);
        	spotEffect = smoothstep(lights[iLight].fCosSpotCutoff, lights[iLight].fCosInnerSpotCutoff, spotEffect);
		}
	
        fAttenuation = spotEffect * fAttenuation;
            
        vec3 refl = reflect(-vLightDir,vNormal);
             
        // TODO: Add the ambient colour of the light
        float NdotHV = max(dot(vNormal,refl),0.0);
        vSpecColor += fAttenuation * lights[iLight].vSpecular0.w * pow(NdotHV, gfSpecularPower);		  
	}
	else
	{
		fAttenuation = 0.0f;
	}

	vec4 vDiffColor = lights[iLight].vDiffuse * nDotLD * fAttenuation;

	vDiffuse += vDiffColor;
	vSpecular.xyz += vSpecColor;
}


void DirectionLighting(vec3 vNormal, vec3 vViewDir, vec3 vWorldPos, float fViewDepth, inout vec4 vDiffuse, inout vec4 vSpecular)
{
	int count = iDirLightCount;
#ifdef USE_SHADOW_MAP
	vec4 vDiffuseTmp = vec4(0.0, 0.0, 0.0, 0.0);
	vec4 vSpecTmp = vec4(0.0, 0.0, 0.0, 0.0);
	if(count > 0)
		CalculateDirectionalLighting(0, vNormal, vViewDir, vDiffuseTmp, vSpecTmp);
	
	float fShadow = SampleShadowmap(fViewDepth, vWorldPos);
	vDiffuse += vDiffuseTmp*fShadow;
	vSpecular += vSpecTmp*fShadow;
#else
	if(count>0)
		CalculateDirectionalLighting(0, vNormal, vViewDir, vDiffuse, vSpecular);
#endif

	for(int i=1; i<count; i++)
	{
		CalculateDirectionalLighting(i, vNormal, vViewDir, vDiffuse, vSpecular);
	}
}


void PointLighting(vec3 vPos, vec3 vNormal, vec3 vViewDir, inout vec4 vDiffuse, inout vec4 vSpecular)
{
	for(int i=0; i<iPointLightCount; i++)
	{
		CalculatePointLighting(i+iDirLightCount, vPos, vNormal, vViewDir, vDiffuse, vSpecular);
	}
}

void SpotLighting(vec3 vPos, vec3 vNormal, vec3 vViewDir, inout vec4 vDiffuse, inout vec4 vSpecular)
{
	int offset = iDirLightCount + iPointLightCount;
	for(int i=0; i<iSpotLightCount; i++)
	{
		CalculateSpotLighting(i+offset, vPos, vNormal, vViewDir, vDiffuse, vSpecular);
	}
}


#endif
