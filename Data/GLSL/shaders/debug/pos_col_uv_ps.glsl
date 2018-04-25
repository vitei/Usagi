#include "../includes/platformdefines.inc"

in vec4 vo_vColor;
in vec2 vo_vTexCoord;

SAMPLER_LOC(1, 0) uniform sampler2D sampler0;

layout(location = 0) out vec4 colorOut;

void main(void)
{	
	vec4 vRead =  texture(sampler0, vo_vTexCoord).xyzw;
	colorOut = vRead*vo_vColor;
	//colorOut = vec4(1.0, 0.0, 0.0, 1.0);
}
