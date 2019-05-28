#include "../includes/platformdefines.inc"
#include "../includes/global_3d.inc"
#include "../includes/deferred_read.inc"
#include "../includes/lighting_structs.inc"
#include "../includes/shadow/poisson_values.inc"

BUFFER_LAYOUT(2, UBO_CUSTOM_3_ID) uniform Custom0
{
	PointLight	light;
	vec2		vInvShadowDim;
};

#ifdef SHADOW_READ
SAMPLER_LOC(2, 9) uniform samplerCubeShadow sampler9;	
#endif

ATTRIB_LOC(0) in vec4 vo_vTexCoord;
ATTRIB_LOC(1) in vec3 vo_vViewRay;
#if USE_LINEAR_DEPTH
ATTRIB_LOC(2) in vec3 vo_vLightPosEye;
#endif

layout (location=0) out vec4 vColorOut0;


#ifdef SHADOW_READ
// For the time being since my google on soft point shadows failed I'm just turning it into a 2D problem and pinching
// the same technique as for directional shadows. 

float rand(vec2 co)
{
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

float ShadowCalculationSoft(vec3 fragPos)
{
	float fBias   = -0.045;

    vec3 vFragToLight = fragPos - light.vPos.xyz;
	float fCurrentDepth = length(vFragToLight) / light.vRange.y;
    vFragToLight.y = 1.0 - vFragToLight.y;

    vFragToLight = normalize(vFragToLight);
   	vec3 vRight = normalize(cross(vFragToLight, vec3(0, 0, 1)));
	vec3 vUp = cross(vRight, vFragToLight);

	vRight *= vInvShadowDim.xxx;
	vUp *= vInvShadowDim.yyy;

    float fDiskRadius = 2.5;
	float fShadow = 0.0;
	for(int i = 0; i < 12; ++i)
	{
		float sampleBias = fBias * length(poissonDisk12[i]);
		vec4 tc = vec4( vFragToLight + (((poissonDisk12[i].x * vRight) + (poissonDisk12[i].y * vUp)) * fDiskRadius), fCurrentDepth + sampleBias);
		fShadow += texture(sampler9, tc);
	}
	fShadow /= float(12);  

	return fShadow;
}

float ShadowCalculation(vec3 fragPos)
{
    vec3 vFragToLight = fragPos - light.vPos.xyz;
	float fCurrentDepth = length(vFragToLight) / light.vRange.y;
	// TODO: Pass this value in
    float fBias = -0.01; 	
    vFragToLight.y = 1.0 - vFragToLight.y;
	vec4 tc = vec4( normalize(vFragToLight), fCurrentDepth + fBias);

    float readValue = texture(sampler9, tc);
    
    return readValue;
}  
#endif

void main(void)
{
	vec3 vPos, vNormal, vColor, vSpecCol;
	float fSpecularPow;
	vec2 vTexCoord = GetRTUV( vec2(0.5, 0.5) * vo_vTexCoord.xy/vo_vTexCoord.ww + vec2(0.5, 0.5) );	
	GetDeferredDataSpec3D(vTexCoord, vo_vViewRay, vPos, vNormal, vColor, vSpecCol, fSpecularPow);
	
	vec3 vLightDir = vec3( vo_vLightPosEye.xyz -  vPos.xyz );

	float d = length(vLightDir);
	
	vec3 vScaledLightDir = vLightDir.xyz * light.vRange.x;
	float fAttenuation = 1.0-smoothstep(light.vRange.w, light.vRange.y, d);

	vLightDir /= d;	// Normalize the vector

	float nDotLD = max(0.0, dot(vNormal, vLightDir));

	vec3 refl = normalize(-reflect(vLightDir,vNormal));
                 
    // TODO: Add the ambient colour of the light
    float NdotHV = max(dot(vNormal,refl),0.0);
    vec3 vSpecular = fAttenuation * vSpecCol * light.vColorSpec.www * pow(NdotHV, fSpecularPow);		

    float fShadow = 1.0f;
#ifdef SHADOW_READ
	vec3 vWorldPos = (vec4(vPos,1.0) * mInvViewMat).xyz;
    fShadow = ShadowCalculationSoft(vWorldPos);//ShadowCalculation(vWorldPos);
#endif
	vec3 vDiffColor = ((light.vColorSpec.xyz * nDotLD * (fShadow)) + light.vAmbient.rgb) * fAttenuation;

	vColorOut0	= vec4((vDiffColor*vColor)+(vSpecular*fShadow), 1.0);
}
