#include "includes/platformdefines.inc"

SAMPLER_LOC(1, 0) uniform sampler2D sampler0;

in vec4 vo_vColor;
in vec2 vo_vTexcoord;

layout(location = 0) out vec4 colorOut;

void main(void)
{	
	colorOut = texture(sampler0, vo_vTexcoord);
}
