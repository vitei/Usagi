#include "../includes/platformdefines.inc"
#include "../includes/global_3d.inc"

SAMPLER_LOC(1, 0) uniform sampler2D sampler0;	// Particle texture

in GeometryData
{
    vec4 	vo_vColor;
    vec2 	vo_vTexcoord;
    vec2	vo_vScreenTex;
	float	vo_fEyeDepth;

} geometryData;


layout(location = 0) out vec4 colorOut;

void main(void)
{
	vec4 vTex 		=  texture(sampler0, geometryData.vo_vTexcoord);
	vec4 vOut = vTex * geometryData.vo_vColor;

	colorOut =  vOut;
}
