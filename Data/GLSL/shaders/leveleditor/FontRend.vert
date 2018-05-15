#include "../includes/platformdefines.inc"
#include "../includes/global_3d.inc"

// Input attributes
ATTRIB_LOC(0) in vec3 ao_position;
ATTRIB_LOC(1) in vec2 ao_texCoord0;
ATTRIB_LOC(2) in vec4 ao_color;


ATTRIB_LOC(0) out vec4  vo_vColor;
ATTRIB_LOC(1) out vec2  vo_vTexCoord;

void main(void)
{
	// It's already in Ndc space, no need to transform
	vec4 vPos = vec4(ao_position, 1.0);

	vo_vColor = ao_color;
	vo_vTexCoord = GetUV(ao_texCoord0);
	gl_Position = vPos;
}
