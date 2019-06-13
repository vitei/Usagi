#include "../includes/platformdefines.inc"


// Input attributes
ATTRIB_LOC(0) in vec3 ao_position;
ATTRIB_LOC(1) in vec2 ao_texCoord0;
ATTRIB_LOC(2) in vec4 ao_color;

ATTRIB_LOC(0) out vec4    vo_vColor;
ATTRIB_LOC(1) out vec2    vo_vTexCoord;


 
void main(void)
{
 	vec4 vPosition = vec4( ao_position.xy, 0.0, 1.0);
 	
 	gl_Position		= vPosition;	

	vo_vColor 	= ao_color;	
	vo_vTexCoord	= ao_texCoord0;
}
