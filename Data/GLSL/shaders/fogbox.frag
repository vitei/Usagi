#include "includes/platformdefines.inc"
#include "includes/global_3d.inc"
#include "includes/depth_read.inc"


// <<GENERATED_CODE>>

ATTRIB_LOC(0) in vec3 vo_vTexCoord;
ATTRIB_LOC(1) in vec4 vo_vScreenTex;
ATTRIB_LOC(2) in vec3 vo_vViewRay;



layout(location = 0) out vec4 colorOut;



void main(void)
{   
	float fFogVal = 1.0;
	
	#ifdef BLEND_GEO

	vec2 vUVCoords = GetRTUV( vec2(0.5, 0.5) * vo_vScreenTex.xy/vo_vScreenTex.ww + vec2(0.5, 0.5) );	
	float fZVal = texture(sampler5, vUVCoords).r;
	vec3 vDepthPos = GetPosFromLinDepth3D(fZVal, vo_vViewRay);
	
	//vec3 vDepthPos = VSPositionFromLinDepth(vUVCoords);
	
	fFogVal = CalculateLinearFog(vDepthPos);
	
	#endif

	colorOut = vec4(texture(sampler0, vo_vTexCoord).rgb, fFogVal);
}

