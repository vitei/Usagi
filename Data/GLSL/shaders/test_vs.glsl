#include "includes/platformdefines.inc"


// Input attributes
ATTRIB_LOC(0) in vec3 ao_position;
ATTRIB_LOC(1) in vec2 ao_texCoord0;

// Output attributes
out vec2 vo_vTexcoord;

BUFFER_LAYOUT(1,  UBO_CUSTOM0_ID) uniform Custom0
{
	mat4	mProjection;
	vec4 	vTestVars;
};


void main(void)
{
 	vec4 vPosition = vec4( ao_position.xyz, 1.0);
	//vPosition.xy += vTestVars.xy;
 	//vec4 vPosition = vec4( ao_position.xy, -0.5, 1.0);
	vPosition = vPosition * mProjection;
 	
 	gl_Position		= vPosition;	

	vo_vTexcoord = ao_texCoord0;
}
