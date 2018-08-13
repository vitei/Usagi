#include "../includes/platformdefines.inc"
#include "../includes/global_3d.inc"
#include "../includes/lighting_structs.inc"

BUFFER_LAYOUT(2,  UBO_CUSTOM0_ID) uniform Custom0
{
	SpotLight	light;
};

// Input attributes
ATTRIB_LOC(0) in vec3 ao_position;

// Output attributes
ATTRIB_LOC(0) out vec4 vo_vTexCoord;
ATTRIB_LOC(1) out vec3 vo_vViewRay;
ATTRIB_LOC(2) out vec3 vo_vLightPosEye;
ATTRIB_LOC(3) out vec3 vo_vSpotDirEye;

void main(void)
{
	// Scale to the light settings
	vec3 vAdjustedPos = (ao_position.xyz);
	vAdjustedPos.z *=  light.vRange.y;
	vAdjustedPos.xy *= light.fOuterSpotRadius;

	// Rotate to match the direction of the spot light
	vAdjustedPos = (vec4(vAdjustedPos, 1.0) * light.mRotMat).xyz;

	// Add the position of the spotlight
	vAdjustedPos += light.vPos.xyz;
	
	vec4 vViewPos = vec4(vAdjustedPos, 1.0) * mViewMat;	// Reverse ordering as a mat3x4
	vec4 vTransformedPos = vViewPos * mProjMat;

	vo_vTexCoord = vTransformedPos;
	vo_vViewRay = vViewPos.xyz;
	vo_vLightPosEye = (light.vPos * mViewMat).xyz;
	vo_vSpotDirEye = normalize((light.vDirection * mViewMat).xyz);

	gl_Position = vTransformedPos;
}
