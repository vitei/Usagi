#include "../includes/platformdefines.inc"
#include "../includes/global_3d.inc"

// Input attributes
ATTRIB_LOC(0) in vec3 ao_position;
ATTRIB_LOC(1) in vec4 ao_vPosRadius;
ATTRIB_LOC(2) in vec4 ao_vColour;

// Output attributes
out vec4 vo_vColor;

void main(void)
{
	vec4 vAdjustedPos = vec4( (ao_position * ao_vPosRadius.www) + ao_vPosRadius.xyz, 1.0);
	
	vec4 vViewPos = vAdjustedPos * mViewMat;	// Reverse ordering as a mat3x4
	vec4 vProjPos = vec4( vViewPos.xyz, 1.0 ) * mProjMat;

	gl_Position = vProjPos;
	vo_vColor = ao_vColour;
}
