#include "includes/platformdefines.inc"
#include "includes/global_3d.inc"
#include "includes/depth_read.inc"

in vec3 vo_vTexCoord;
in vec4 vo_vScreenTex;
in vec3 vo_vViewRay;

SAMPLER_LOC(1, 0) uniform samplerCube sampler0;
SAMPLER_LOC(1, 5) uniform sampler2D sampler5;	// The depth texture


layout(location = 0) out vec4 colorOut;



void main(void)
{   
	vec2 vUVCoords = GetRTUV( vec2(0.5, 0.5) * vo_vScreenTex.xy/vo_vScreenTex.ww + vec2(0.5, 0.5) );	
	float fZVal = texture(sampler5, vUVCoords).r;
	vec3 vDepthPos = GetPosFromLinDepth3D(fZVal, vo_vViewRay);
	
	//vec3 vDepthPos = VSPositionFromLinDepth(vUVCoords);
	
	float fFogVal = CalculateLinearFog(vDepthPos);

	colorOut = vec4(texture(sampler0, vo_vTexCoord).rgb, fFogVal);
}

