#include "../includes/platformdefines.inc"
#include "../includes/global_3d.inc"
#include "../includes/lighting_structs.inc"

BUFFER_LAYOUT(1,  UBO_CUSTOM0_ID) uniform Custom0
{
	PointLight	light;
	vec2		vInvShadowDim;
};


// Input attributes
ATTRIB_LOC(0) in vec3 ao_position;

// Output attributes
ATTRIB_LOC(0) out vec4 vo_vTexCoord;
ATTRIB_LOC(1) out vec3 vo_vViewRay;
ATTRIB_LOC(2) out vec3 vo_vLightPosEye;


void main(void)
{
	vec3 vAdjustedPos = (ao_position * light.vRange.yyy) + light.vPos.xyz;
	
	vec4 vViewPos = vec4(vAdjustedPos, 1.0) * mViewMat;	// Reverse ordering as a mat3x4
	vec4 vTransformedPos = vViewPos * mProjMat;

	vo_vTexCoord = vTransformedPos;
	vo_vViewRay = vViewPos.xyz;
	vo_vLightPosEye = (light.vPos * mViewMat).xyz;

	gl_Position = vTransformedPos;
}
