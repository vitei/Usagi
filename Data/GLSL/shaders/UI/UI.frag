#include "../includes/platformdefines.inc"
#include "../includes/colorspace.inc"

// <<GENERATED_CODE>>

ATTRIB_LOC(0) in vec4 go_vColor;
ATTRIB_LOC(1) in vec2 go_vTexcoord;

layout(location = 0) out vec4 colorOut;

void main(void)
{	
	vec4 vRead =  texture(sampler0, go_vTexcoord).xyzw;
	vRead*= go_vColor;

	if(bsRGBConv)
	{
		colorOut = vec4(fromLinear(vRead.rgb), vRead.a);
	}
	else
	{
		colorOut = vRead;
	}
}
