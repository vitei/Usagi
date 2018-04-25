#include "includes/platformdefines.inc"
#include "includes/global_3d.inc"
#include "includes/depth_read.inc"

// Input attributes
ATTRIB_LOC(0) in vec3 ao_position;

// Output attributes
out vec3 vo_vTexCoord;
out vec4 vo_vScreenTex;
out vec3 vo_vViewRay;

void main(void)
{
	vec3 vAdjustedPos = (ao_position * vFogVars.zzz);	// Make our size equal to the minimum fog depth
	vec3 vPosition = vAdjustedPos + vEyePos.xyz;
	vec4 vViewPos = vec4(vPosition, 1.0) * mViewMat;
	vec4 vProjPos = vec4( vViewPos.xyz, 1.0 ) * mProjMat;

	vo_vScreenTex = vProjPos;
	vo_vViewRay = vViewPos.xyz;

	gl_Position = vProjPos;

 	vo_vTexCoord = ao_position.xyz;
}


