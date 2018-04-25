#include "../includes/platformdefines.inc"

in vec2 vo_vTexCoord;
SAMPLER_LOC(1, 0) uniform sampler2D sampler0;

layout(location = 0) out vec4 colorOut;

void main(void)
{	
	colorOut = texture( sampler0, vo_vTexCoord ).rrrr;
}
