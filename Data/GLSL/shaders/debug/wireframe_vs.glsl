#include "../includes/platformdefines.inc"
#include "../includes/global_3d.inc"

BUFFER_LAYOUT(1,  UBO_MATERIAL_ID) uniform Material
{
	mat4	mModel;
	vec4	vExtents;
	vec4	vColor;
};

// Input attributes
in vec3 ao_position;

// Output attributes
out vec4 vo_vColor;


void main(void)
{
	vec3 vAdjustedSize = ao_position.xyz * vExtents.xyz;

 	vec4 vPosition = vec4( vAdjustedSize, 1.0) * mModel;
 	//vec4 vPosition = vec4( ao_position.xy, -0.5, 1.0);
 	
	vec4 vViewPos = vPosition * mViewMat;	// Reverse ordering as a mat3x4
	vec4 vProjPos = vec4( vViewPos.xyz, 1.0 ) * mProjMat;

 	gl_Position		= vProjPos;	

	vo_vColor = vColor;	
}
