#include "../includes/platformdefines.inc"

in vec4 go_vColor;
in vec4 go_vBgColor;
in vec2 go_vTexCoord;

SAMPLER_LOC(1, 0) uniform sampler2D sampler0;

layout(location = 0) out vec4 colorOut;

void main(void)
{	
	vec4 vRead =  texture(sampler0, go_vTexCoord).xyzw;
	vec4 vOut;

	vOut.rgb = (1.0-vRead.r)*go_vColor.rgb;
	vOut.a = vRead.a * go_vColor.a;

	colorOut = vOut;
}
