#include "../includes/platformdefines.inc"
#include "../includes/global_3d.inc"
#include "../includes/deferred_read.inc"
#include "../includes/lighting_structs.inc"
#include "../includes/shadow/poisson_values.inc"
#include "../includes/shadow/projectionshadow_read.inc"

#define USE_SPECULAR

BUFFER_LAYOUT(1, UBO_CUSTOM0_ID) uniform Custom0
{
	SpotLight	light;
};

ATTRIB_LOC(0) in vec4 vo_vTexCoord;
ATTRIB_LOC(1) in vec3 vo_vViewRay;
ATTRIB_LOC(2) in vec3 vo_vLightPosEye;
ATTRIB_LOC(3) in vec3 vo_vSpotDirEye;

layout(location = 0) out vec4 vColorOut0;


void main(void)
{
	vec3 vPos, vNormal, vColor, vGSpecCol;
	float fSpecPow;
	vec2 vTexCoord = GetRTUV( vec2(0.5, 0.5) * vo_vTexCoord.xy/vo_vTexCoord.ww + vec2(0.5, 0.5) );	
	GetDeferredDataSpec3D(vTexCoord, vo_vViewRay, vPos, vNormal, vColor, vGSpecCol, fSpecPow);
	
	vec3 vLightDir = vec3( vo_vLightPosEye.xyz -  vPos.xyz );
	float d = length(vLightDir);
	
	vec3 vScaledLightDir = vLightDir.xyz * light.vRange.x;
	float fAttenuation = clamp(1.0 - dot(vScaledLightDir, vScaledLightDir), 0.0, 1.0);

	vLightDir /= d;	// Normalize the vector

	float nDotLD = max(0.0, dot(vNormal, vLightDir));
	float spotEffect = dot( vo_vSpotDirEye, -vLightDir );
	vec3 vSpecColor = vec3(0.0, 0.0, 0.0);

	if(nDotLD > 0.0 && spotEffect > light.fCosSpotCutoff)
	{

		spotEffect = min(light.fCosInnerSpotCutoff, spotEffect);
        spotEffect = smoothstep(light.fCosSpotCutoff, light.fCosInnerSpotCutoff, spotEffect);

        fAttenuation = spotEffect * fAttenuation;

#ifdef USE_SPECULAR                 
        // TODO: Add the ambient colour of the light
     
        vec3 refl = reflect(-vLightDir,vNormal);
             
        // TODO: Add the ambient colour of the light
        float NdotHV = max(dot(vNormal,refl),0.0);
        vSpecColor += fAttenuation * vGSpecCol * light.vColorSpec.w * pow(NdotHV, fSpecPow);		
#endif        
	}
	else
	{
		fAttenuation = 0.0f;
	}


	// TODO: Specular
#ifdef SHADOW_READ
	float fShadow = ShadowCalculationSoft(vPos);
	fAttenuation *= fShadow;
	vSpecColor *= fShadow;
#endif
	vec3 vDiffColor = light.vColorSpec.xyz * nDotLD * fAttenuation;

	vColorOut0	= vec4((vDiffColor*vColor)+(vSpecColor), 1.0);
}
