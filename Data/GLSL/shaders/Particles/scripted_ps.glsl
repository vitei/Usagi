#include "../includes/platformdefines.inc"
#include "../includes/global_3d.inc"

SAMPLER_LOC(1, 0) uniform sampler2D sampler0;	// Particle texture
SAMPLER_LOC(1, 1) uniform sampler2D sampler1;	// Particle texture
SAMPLER_LOC(4, 14) uniform sampler2D sampler14;	// Linear depth texture


BUFFER_LAYOUT(1, UBO_MATERIAL_1_ID) uniform Material1
{
    float    fAlphaRef;
    float	 fDepthFade;
};

in GeometryData
{
    vec4 	vo_vColor;
    vec2 	vo_vTexcoord[2];
    vec2	vo_vScreenTex;
    float	vo_fEyeDepth;

} geometryData;


layout(location = 0) out vec4 colorOut;

void main(void)
{
	float zFade = 1.0;

	if(fDepthFade > 0.0)
	{
		vec2 vScreenTex = clamp(vec2(0.5, 0.5) * geometryData.vo_vScreenTex.xy + vec2(0.5, 0.5), vec2(0.0, 0.0), vec2(1.0, 1.0));
		float fDepthRead = texture(sampler14, vScreenTex).r;
		zFade *= clamp((fDepthFade*vNearFar.y) * (fDepthRead - geometryData.vo_fEyeDepth), 0.0, 1.0);
		//zFade *= clamp(fDepthFade * (geometryData.vo_fEyeDepth-vNearFar.z), 0.0, 1.0);
	}


	vec4 vTex 		=  texture(sampler0, geometryData.vo_vTexcoord[0]);
	vec4 vOut = vTex * geometryData.vo_vColor;
	vOut.a *= zFade;

	if(vOut.a < fAlphaRef)
		discard;

	colorOut =  vOut;
}
