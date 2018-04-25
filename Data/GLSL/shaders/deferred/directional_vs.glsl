#include "../includes/platformdefines.inc"
#include "../includes/global_3d.inc"


// Input attributes
in vec3 ao_position;

// Output attributes
out vec2 vo_vTexCoord;
out vec3 vo_vFSVector;


void main(void)
{
 	vec4 vPosition = vec4( ao_position.xy, -0.5, 1.0);
 	
 	gl_Position		= vPosition;	

	vo_vTexCoord = GetRTUV( vec2(0.5, 0.5) * vPosition.xy + vec2(0.5, 0.5) );	
	
	// Calculate the position as projected onto the projection matrix far plane
	vo_vFSVector = ((vec4(ao_position.x,ao_position.y,1,1) * mInvProjMat).xyz) * vNearFar.y;
}

