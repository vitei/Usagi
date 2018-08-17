#include "../includes/platformdefines.inc"
#include "../includes/global_3d.inc"

SAMPLER_LOC(1, 0) uniform sampler2D sampler0;	// Particle texture
SAMPLER_LOC(1, 5) uniform sampler2D sampler5;	// Linear depth texture

in GeometryData
{
	INT_LOC(0) vec4 	vo_vColor;
    INT_LOC(1) vec2 	vo_vTexcoord;
    INT_LOC(2) vec2  vo_vScreenTex;
    INT_LOC(3) float vo_fEyeDepth;

} geometryData;

BUFFER_LAYOUT(1, UBO_MATERIAL_ID) uniform Material
{
	vec4 vEffectConsts;
    vec4 vEffectConsts2;
};

#define DEPTH_FADE	vEffectConsts2.w

layout(location = 0) out vec4 colorOut;

void main(void)
{
	float zFade = 1.0;
	vec2 vScreenTex = clamp(vec2(0.5, 0.5) * geometryData.vo_vScreenTex.xy + vec2(0.5, 0.5), vec2(0.0, 0.0), vec2(1.0, 1.0));
	//	float fDepthRead = texture(sampler5, vScreenTex).r;
//	zFade *= clamp(DEPTH_FADE * (fDepthRead - geometryData.vo_fEyeDepth), 0.0, 1.0);
//	zFade *= clamp(DEPTH_FADE * (geometryData.vo_fEyeDepth-vNearFar.z), 0.0, 1.0);


	vec4 vTex 		=  texture(sampler0, geometryData.vo_vTexcoord);
	vec4 vOut = vTex * geometryData.vo_vColor;
	vOut.a *= zFade;

	colorOut =  vOut;

	if(vOut.a<=0.0f)
		discard;
}
