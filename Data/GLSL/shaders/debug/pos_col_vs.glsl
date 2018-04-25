#include "../includes/platformdefines.inc"

BUFFER_LAYOUT(1,  UBO_MATERIAL_ID) uniform Material
{
	mat4	mProj;
};

// Input attributes
ATTRIB_LOC(0) in vec3 ao_position;
ATTRIB_LOC(1) in vec4 ao_color;

// Output attributes
out vec4 vo_vColor;


void main(void)
{
 	vec4 vPosition = vec4( ao_position.xyz, 1.0) * mProj;
 	//vec4 vPosition = vec4( ao_position.xy, -0.5, 1.0);
 	
 	gl_Position		= vPosition;	

	vo_vColor = ao_color;	
}
