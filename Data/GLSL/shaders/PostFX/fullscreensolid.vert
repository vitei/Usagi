#include "../includes/platformdefines.inc"

// Input attributes
ATTRIB_LOC(0) in vec3 ao_position;


void main(void)
{
 	vec4 vPosition = vec4( ao_position.xy, 0.0, 1.0);
 	
 	gl_Position		= vPosition;	
}
