#include "../includes/platformdefines.inc"
#include "../includes/global_3d.inc"

BUFFER_LAYOUT(1,  UBO_CUSTOM_3_ID) uniform Custom0
{
	mat4 	mLineViewProj;
};


// Input attributes
ATTRIB_LOC(0) in vec3 ao_position;
ATTRIB_LOC(1) in vec4 ao_color;

ATTRIB_LOC(0) out vec4 vo_vColor;

void main(void)
{
	vec4 vPos = vec4(ao_position, 1.0) * mLineViewProj;

	vo_vColor = ao_color;
	gl_Position = vPos;
}
