#ifndef _FORWARD_LIGHTING_INC
#define _FORWARD_LIGHTING_INC
#ifndef SHADOW_PASS



void CalculatePointLighting(int iLight, vec3 vPos, vec3 vNormal, vec3 vViewDir, inout vec4 vDiffuse, inout vec4 vSpecular, float fSpecPower)
{
	vec3 vLightDir = vec3( lights[iLight].vPosition.xyz -  vPos.xyz );

	if( dot(vLightDir, vLightDir) < lights[iLight].vRange.z )
	{
		float d = length(vLightDir);
		float fAttenuation;


		// Obey the inner / outer settings
		fAttenuation = 1.0-smoothstep(lights[iLight].vRange.w, lights[iLight].vRange.y, d);

		vLightDir /= d;	// Normalize the vector

		float nDotLD = max(0.0, dot(vNormal, vLightDir));

		vec3 refl = normalize(-reflect(vLightDir,vNormal));
	                 
	    // TODO: Add the ambient colour of the light
	    float NdotHV = max(dot(vNormal,refl),0.0);
	    vSpecular.xyz += fAttenuation * lights[iLight].vSpecular.xyz * pow(NdotHV, fSpecPower);		

		vDiffuse.xyz += ((lights[iLight].vDiffuse.xyz * nDotLD) + lights[iLight].vAmbient.rgb) * fAttenuation;
	}
}

void CalculateDirectionalLighting(int iLight, vec3 vNormal, vec3 vViewDir, inout vec4 vDiffuse, inout vec4 vSpecular, float fSpecPower)
{
	vec3 vLightDir		= lights[iLight].vDirection.xyz;
	float nDotL			= max( dot( vNormal, vLightDir ), 0.0); 
   
	vec3  vReflection	= reflect( -vLightDir, vNormal ); 
	float fRDotV		= max( 0.0, dot( vReflection, vViewDir ) );

	float fSpecular = ( pow( fRDotV, fSpecPower ) );

	vDiffuse  += lights[iLight].vDiffuse*nDotL;
	vSpecular += lights[iLight].vSpecular*fSpecular;
}


void CalculateSpotLighting(int iLight, vec3 vPos, vec3 vNormal, vec3 vViewDir, inout vec4 vDiffuse, inout vec4 vSpecular, float fSpecPower)
{	
	vec3 vLightDir = vec3( lights[iLight].vPosition.xyz -  vPos.xyz );
	float d = length(vLightDir);
	
	float fAttenuation = 1.0f;
	
	if(lights[iLight].vRange.y > 0.0f)
	{
		//
		vec3 vScaledLightDir = vLightDir.xyz * lights[iLight].vRange.x;
		fAttenuation = clamp(1.0 - dot(vScaledLightDir, vScaledLightDir), 0.0, 1.0);
	}

	vLightDir /= d;	// Normalize the vector

	float nDotLD = max(0.0, dot(vNormal, vLightDir));
	float spotEffect = dot( lights[iLight].vDirection.xyz, -vLightDir );
	vec3 vSpecColor = vec3(0.0, 0.0, 0.0);

	if(nDotLD > 0.0 && spotEffect > lights[iLight].fCosSpotCutoff)
	{

		spotEffect = min(lights[iLight].fCosInnerSpotCutoff, spotEffect);
       	spotEffect = smoothstep(lights[iLight].fCosSpotCutoff, lights[iLight].fCosInnerSpotCutoff, spotEffect);
	
        fAttenuation = spotEffect * fAttenuation;
            
        vec3 refl = reflect(-vLightDir,vNormal);
             
        // TODO: Add the ambient colour of the light
        float NdotHV = max(dot(vNormal,refl),0.0);
        vSpecColor += fAttenuation * lights[iLight].vSpecular.w * pow(NdotHV, fSpecPower);		  
	}
	else
	{
		fAttenuation = 0.0f;
	}

	vec4 vDiffColor = lights[iLight].vDiffuse * nDotLD * fAttenuation;

	vDiffuse += vDiffColor;
	vSpecular.xyz += vSpecColor;
}


void DirectionLighting(vec3 vNormal, vec3 vViewDir, vec3 vWorldPos, float fViewDepth, inout vec4 vDiffuse, inout vec4 vSpecular, float fSpecPower)
{
	int count = iDirLightCount;

	for(int i=0; i<count; i++)
	{
		if(i != iCascadeLightStart)
		{
			CalculateDirectionalLighting(i, vNormal, vViewDir, vDiffuse, vSpecular, fSpecPower);
		}
		else
		{
			float fShadow = SampleShadowmap(fViewDepth, vWorldPos);
			vec4 vShadowMul = mix(vShadowColor, vec4(1.0), fShadow);
			// For now assuming always one light
			{
				vec3 vLightDir		= lights[i].vDirection.xyz;
				float nDotL			= max( dot( vNormal, vLightDir ), 0.0); 
		   
				vec3  vReflection	= reflect( -vLightDir, vNormal ); 
				float fRDotV		= max( 0.0, dot( vReflection, vViewDir ) );

				float fSpecular = ( pow( fRDotV, fSpecPower ) );

				vDiffuse  += lights[i].vDiffuse*nDotL*fShadow;
				vSpecular += lights[i].vSpecular*fSpecular*fShadow;
			}
		}

	}
}


void PointLighting(vec3 vPos, vec3 vNormal, vec3 vViewDir, inout vec4 vDiffuse, inout vec4 vSpecular, float fSpecPower)
{
	for(int i=0; i<iPointLightCount; i++)
	{
		CalculatePointLighting(i+iDirLightCount, vPos, vNormal, vViewDir, vDiffuse, vSpecular, fSpecPower);
	}
}

void SpotLighting(vec3 vPos, vec3 vNormal, vec3 vViewDir, inout vec4 vDiffuse, inout vec4 vSpecular, float fSpecPower)
{
	int offset = iDirLightCount + iPointLightCount;
	for(int i=0; i<iSpotLightCount; i++)
	{
		CalculateSpotLighting(i+offset, vPos, vNormal, vViewDir, vDiffuse, vSpecular, fSpecPower);
	}
}


#endif
#endif
