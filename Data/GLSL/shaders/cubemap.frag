#include "includes/platformdefines.inc"
#include "includes/global_3d.inc"
ATTRIB_LOC(0) in vec3 vo_vTexCoord;
ATTRIB_LOC(1) in vec2 vo_vScreenPos;

SAMPLER_LOC(1, 0) uniform samplerCube sampler0;


layout(location = 0) out vec4 colorOut;

void main(void)
{   
	colorOut = texture(sampler0, vo_vTexCoord);
}

