#include "../includes/platformdefines.inc"
#include "../includes/global_3d.inc"

// Input attributes
ATTRIB_LOC(0) in vec3 ao_position;
ATTRIB_LOC(1) in vec2 ao_texCoord0;
ATTRIB_LOC(2) in vec3 ao_normal;
ATTRIB_LOC(3) in vec3 ao_tangent;
ATTRIB_LOC(4) in vec3 ao_binormal;

// Output attributes
out vec2 vo_vTexCoord;
out vec3 vo_vEyeNormal;
out vec3 vo_vEyeTangent;
out vec3 vo_vEyeBinormal;
out vec3 vo_vEyePosOut;



void main(void)
{
	vec4 vViewPos = vec4(ao_position, 1.0) * mViewMat;
	vo_vEyePosOut = vViewPos.xyz;
	vec4 vProjPos = vec4( vViewPos.xyz, 1.0 ) * mProjMat;

	gl_Position	= vProjPos;
	vo_vTexCoord	= ao_texCoord0;
	
	vo_vEyeNormal = (vec4(ao_normal, 0.0) * mViewMat).xyz;
	vo_vEyeTangent = (vec4(ao_tangent, 0.0) * mViewMat).xyz;
	vo_vEyeBinormal = (vec4(ao_binormal, 0.0) * mViewMat).xyz;
}
