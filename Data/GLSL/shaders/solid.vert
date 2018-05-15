#include "includes/platformdefines.inc"
#include "includes/global_3d.inc"

// Input attributes
ATTRIB_LOC(0) in vec3 ao_position;
// in vec3 ao_normal;

out vec3 vo_vNormal;
out vec3 vo_vPosition;
out vec3 vo_vEyePosOut;
out vec3 vo_vWorldPos;

// Motion blur
// out vec4 vo_vScreenPos;
// out vec4 vo_vPrevScreenPos;

void main(void)
{
	vec4 vEyePos	= vec4(ao_position, 1.0) * mViewMat;
	vo_vEyePosOut = vEyePos.xyz;
	vec4 vProjPos = vec4(ao_position, 1.0) * mViewProjMat;
	gl_Position	= vProjPos;
	vo_vPosition	= ao_position.xyz;
	
	vo_vNormal = (vec4(0.0, 1.0, 0.0, 0.0) * mViewMat).xyz;
	vo_vWorldPos = ao_position;
	// CalculateClipDistance( vec4(ao_position, 1.0) );
	// vo_vPrevScreenPos = CalculatePrevPosNoModelMat(vec4(ao_position, 1.0));
	// vo_vScreenPos = vProjPos;
}
