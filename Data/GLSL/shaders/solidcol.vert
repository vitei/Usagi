#include "includes/platformdefines.inc"
#include "includes/global_3d.inc"

// Input attributes
ATTRIB_LOC(0) in vec3 ao_position;
ATTRIB_LOC(1) in vec4 ao_color;
// in vec3 ao_normal;

ATTRIB_LOC(0) out vec4 vo_vColor;
ATTRIB_LOC(1) out vec3 vo_vPosition;
ATTRIB_LOC(2) out vec3 vo_vEyePosOut;
ATTRIB_LOC(3) out vec3 vo_vWorldPos;

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

	vo_vColor = ao_color;
	
	vo_vWorldPos = ao_position;
	// CalculateClipDistance( vec4(ao_position, 1.0) );
	// vo_vPrevScreenPos = CalculatePrevPosNoModelMat(vec4(ao_position, 1.0));
	// vo_vScreenPos = vProjPos;
}
