#include "../includes/platformdefines.inc"

in GeometryData
{
    vec4 	vo_vColor;
    vec2 	vo_vTexcoord;

} geometryData;


SAMPLER_LOC(1, 0) uniform sampler2D sampler0;
layout(location = 0) out vec4 colorOut;

void main(void)
{	
	vec2 vTmp = geometryData.vo_vTexcoord;
	vec4 vRead =  texture(sampler0, GetUV(vTmp)).xyzw;
	colorOut = geometryData.vo_vColor * vRead;
	//gl_FragData[0] = geometryData.vo_vColor;//vec4(0.0, 1.0, 0.0, 1.0);
}
