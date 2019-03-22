#include "../includes/platformdefines.inc"
#include "../includes/global_3d.inc"
#include "../includes/deferred_read.inc"
#include "../includes/lighting_structs.inc"
#include "../includes/shadow/poisson_values.inc"
#include "../includes/shadow/projectionshadow_read.inc"


BUFFER_LAYOUT(2, UBO_CUSTOM0_ID) uniform Custom0
{
	ProjectionLight	light;
};

// The projection texture (0-4 are take up by the deferred shading)
SAMPLER_LOC(2, 5) uniform sampler2D sampler5;

ATTRIB_LOC(0) in vec4 vo_vTexCoord;
ATTRIB_LOC(1) in vec3 vo_vViewRay;
ATTRIB_LOC(2) in vec3 vo_vLightPosEye;

layout (location=0) out vec4 vColorOut0;

void main(void)
{
	vec3 vPos, vNormal, vColor, vSpecCol;
	float fSpecular;
	vec2 vTexCoord = GetRTUV( vec2(0.5, 0.5) * vo_vTexCoord.xy/vo_vTexCoord.ww + vec2(0.5, 0.5) );	
	GetDeferredDataSpec3D(vTexCoord, vo_vViewRay, vPos, vNormal, vColor, vSpecCol, fSpecular);
	
	vec3 vLightDir = vec3( vo_vLightPosEye.xyz -  vPos.xyz );
	float d = length(vLightDir);
	
	vec3 vScaledLightDir = vLightDir.xyz * light.vRange.x;
	float fAttenuation = 1.0-smoothstep(light.vRange.w, light.vRange.y, d);

	vLightDir /= d;	// Normalize the vector

	float nDotLD = max(0.0, dot(vNormal, vLightDir));

	vec3 texRead = vec3(0,0,0);


	vec4 vProjCoord 	= vec4(vPos, 1.0)*mInvViewMat;
	vProjCoord = GetUV(vProjCoord * light.mTextureMat);
	
	if(vProjCoord.w > 0.0)
		texRead = texture(sampler5, GetUV(vProjCoord.xy / vProjCoord.ww)).rgb;
		//texRead = texture2DProj(sampler3, -vProjCoord).rgb;

	vec3 vDiffColor = ((texRead * nDotLD * light.vColorSpec.xyz) + (texRead * light.vAmbient.xyz)) * fAttenuation;

	// FIXME: Add specular
#ifdef SHADOW_READ
	float fShadow = ShadowCalculationSoft(vPos);
	vDiffColor *= fShadow;
#endif

	vColorOut0	= vec4((vDiffColor*vColor), 1.0);
}
