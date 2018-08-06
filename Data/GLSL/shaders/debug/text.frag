#include "../includes/platformdefines.inc"

in GeometryData
{
    INT_LOC(0) vec4 	go_vColor;
    INT_LOC(1) vec2 	go_vTexcoord;

} geometryData;


SAMPLER_LOC(1, 0) uniform sampler2D sampler0;
layout(location = 0) out vec4 colorOut;

void main(void)
{	
	vec2 vTmp = geometryData.go_vTexcoord;
	vec4 vRead =  texture(sampler0, GetUV(vTmp)).xyzw;
	colorOut = geometryData.go_vColor * vRead;
	//gl_FragData[0] = geometryData.vo_vColor;//vec4(0.0, 1.0, 0.0, 1.0);
	//	colorOut = vec4(geometryData.go_vTexcoord, 0.0, 1.0);
}
