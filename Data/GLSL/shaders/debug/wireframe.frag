#include "../includes/platformdefines.inc"

ATTRIB_LOC(0) in vec4 vo_vColor;

layout(location = 0) out vec4 colorOut;

void main(void)
{	
	colorOut = vo_vColor;
}
