#include "../includes/platformdefines.inc"
#include "../includes/global_3d.inc"
#include "../includes/lighting_structs.inc"

BUFFER_LAYOUT(2,  UBO_CUSTOM0_ID) uniform Custom0
{
	PointLight	light;
	vec2		vInvShadowDim;
};


// Input attributes
ATTRIB_LOC(0) in vec3 ao_position;

void main(void)
{
	vec3 vAdjustedPos = (ao_position * light.vRange.yyy) + light.vPos.xyz;
	
	vec4 vViewPos = vec4(vAdjustedPos, 1.0) * mViewMat;	// Reverse ordering as a mat3x4
	vec4 vProjPos = vViewPos * mProjMat;

	gl_Position = vProjPos;
}
