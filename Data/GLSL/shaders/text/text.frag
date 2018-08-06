#include "../includes/platformdefines.inc"

in GeometryData
{
	INT_LOC(0) vec2  	go_vPos;
    INT_LOC(1) vec4 	go_vColor;
    INT_LOC(2) vec4 	go_vBgColor;	// Not currently used on standard font rendering
    INT_LOC(3) vec2 	go_vTexcoord;

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
