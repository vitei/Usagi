#include "../includes/platformdefines.inc"

in GeometryData
{
    vec4 	go_vColor;
    vec4 	go_vBgColor;	// Not currently used on standard font rendering
    vec2 	go_vTexcoord;

} geometryData;

SAMPLER_LOC(1, 0) uniform sampler2D sampler0;

layout(location = 0) out vec4 colorOut;

void main(void)
{	
	vec4 vRead =  texture(sampler0, geometryData.go_vTexcoord).xyzw;
	vec4 vOut;

	vOut.rgb = (1.0-vRead.r)*geometryData.go_vColor.rgb;
	vOut.a = vRead.a * geometryData.go_vColor.a;

	colorOut = vOut;
}
