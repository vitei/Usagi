#include "../includes/platformdefines.inc"
#include "../includes/shadow/globalshadow_write.inc"

// Input attributes
ATTRIB_LOC(0) in vec3 ao_position;
ATTRIB_LOC(1) in mat3x4 ao_transform;

out vec4 vo_position;

void main(void)
{
	vec4 vWorldPos = vec4( vec4(ao_position, 1.0) * ao_transform, 1.0);
	vec4 vViewPos = vWorldPos * mViewMat;
	vec4 vProjPos = vec4( vViewPos.xyz, 1.0 ) * mProjMat;

	vo_position			= vProjPos;
	gl_Position			= vProjPos;
}
