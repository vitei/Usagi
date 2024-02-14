#include "../includes/platformdefines.inc"
#include "../includes/global_2d.inc"
#include "../includes/colorspace.inc"

// <<GENERATED_CODE>>

// Output attributes
ATTRIB_LOC(0) out vec4 vo_vColor;
ATTRIB_LOC(1) out vec4 vo_vTexCoordRng;
ATTRIB_LOC(2) out vec2 vo_vSize;


void main(void)
{
 	vec4 vPosition = vec4( ao_position.xyz, 1.0);
 	
 	gl_Position		= vPosition;	

	vo_vColor 		= toLinear(ao_color);	
	vo_vTexCoordRng	= ao_texcoordRng;
	vo_vSize 		= ao_size;
}
