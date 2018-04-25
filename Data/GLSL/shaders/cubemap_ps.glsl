#include "includes/platformdefines.inc"
#include "includes/global_3d.inc"
in vec3 vo_vTexCoord;
in vec2 vo_vScreenVel;

SAMPLER_LOC(1, 0) uniform samplerCube sampler0;

in vec4 vo_vScreenPos;

layout(location = 0) out vec4 colorOut;

void main(void)
{   
	colorOut = texture(sampler0, vo_vTexCoord);
}

