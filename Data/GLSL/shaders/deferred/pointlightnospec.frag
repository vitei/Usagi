#include "../includes/platformdefines.inc"
#include "../includes/global_3d.inc"
#include "../includes/deferred_read.inc"
#include "../includes/lighting_structs.inc"

BUFFER_LAYOUT(1, UBO_CUSTOM0_ID) uniform Custom0
{
	PointLight	light;
	vec2		vInvShadowDim;
};

in vec4 vo_vTexCoord;
in vec3 vo_vViewRay;
#if USE_LINEAR_DEPTH
in vec3 vo_vLightPosEye;
#endif

out vec4 vColorOut0;

// TODO: Remove hardcodeing
const float gfSpecularPower = 32.0;

void main(void)
{
	vec3 vPos, vNormal, vColor;
	vec2 vTexCoord = GetRTUV( vec2(0.5, 0.5) * vo_vTexCoord.xy/vo_vTexCoord.ww + vec2(0.5, 0.5) );	
	GetDeferredData3D(vTexCoord, vo_vViewRay, vPos, vNormal, vColor);
	
	vec3 vLightDir = vec3( vo_vLightPosEye.xyz -  vPos.xyz );

	float d = length(vLightDir);
	
	vec3 vScaledLightDir = vLightDir.xyz * light.vRange.x;
	float fAttenuation = 1.0-smoothstep(light.vRange.w, light.vRange.y, d);

	vLightDir /= d;	// Normalize the vector

	float nDotLD = max(0.0, dot(vNormal, vLightDir));

	vec3 vDiffColor = ((light.vColorSpec.xyz * nDotLD) + light.vAmbient.rgb) * fAttenuation;

	vColorOut0	= vec4((vDiffColor*vColor), 1.0);
}
