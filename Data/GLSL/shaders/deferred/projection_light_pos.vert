#include "../includes/platformdefines.inc"
#include "../includes/global_3d.inc"
#include "../includes/lighting_structs.inc"

BUFFER_LAYOUT(1,  UBO_CUSTOM0_ID) uniform Custom0
{
	ProjectionLight	light;
};

// Input attributes
in vec3 ao_position;

// Output attributes
out vec4 vo_vTexCoord;
out vec3 vo_vViewRay;
out vec3 vo_vLightPosEye;


void main(void)
{	
	vec4 vWorldPos = vec4(ao_position, 1.0) * light.mViewProjInv;
	vec4 vViewPos = vWorldPos * mViewMat;	
	vec4 vTransformedPos = vViewPos * mProjMat;

	vo_vTexCoord	= vTransformedPos;
	vo_vViewRay		= vViewPos.xyz;
	vo_vLightPosEye = (light.vPos * mViewMat).xyz;

	gl_Position = vTransformedPos;
}
