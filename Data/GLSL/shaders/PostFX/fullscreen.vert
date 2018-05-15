#include "../includes/platformdefines.inc"

// Input attributes
ATTRIB_LOC(0) in vec3 ao_position;

// Output attributes
out vec2 vo_vTexCoord;


void main(void)
{
 	vec4 vPosition = vec4( ao_position.xy, -0.5, 1.0);
 	
 	gl_Position		= vPosition;	

	vo_vTexCoord = GetRTUV( vec2(0.5, 0.5) * vPosition.xy + vec2(0.5, 0.5) );	
}
