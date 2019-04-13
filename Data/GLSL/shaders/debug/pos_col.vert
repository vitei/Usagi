#include "../includes/platformdefines.inc"
#include "../includes/global_2d.inc"


// Input attributes
ATTRIB_LOC(0) in vec3 ao_position;
ATTRIB_LOC(1) in vec4 ao_color;

// Output attributes
ATTRIB_LOC(0) out vec4 vo_vColor;


void main(void)
{
 	vec4 vPosition = vec4( ao_position.xyz, 1.0) * proj;
 	//vec4 vPosition = vec4( ao_position.xy, -0.5, 1.0);
 	
 	gl_Position		= vPosition;	

	vo_vColor = ao_color;	
}
