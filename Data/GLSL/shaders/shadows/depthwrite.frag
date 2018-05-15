#include "../includes/platformdefines.inc"

layout(location = 0) out vec4 colorOut;

in vec4 vo_position;

void main(void)
{	
	// Output the depth and the depth squared for variance look-up
	colorOut = vec4(vo_position.z, vo_position.z*vo_position.z, 1.0, 1.0);
}
