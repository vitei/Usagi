#include "../includes/platformdefines.inc"

SAMPLER_LOC(1, 0) uniform sampler2D sampler0;


layout(location = 0) out vec4 colorOut;

ATTRIB_LOC(0) in vec4  vo_vColor;
ATTRIB_LOC(1) in vec2  vo_vTexCoord;

void main(void)
{ 
  	vec4 vRead =  texture(sampler0, vo_vTexCoord).xyzw;
	vec4 vOut;

	vOut.rgb = vo_vColor.rgb;
	vOut.a = vRead.a * vo_vColor.a;

	colorOut = vOut;
}
