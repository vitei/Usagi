#include "../includes/platformdefines.inc"
#include "../includes/global_3d.inc"
#include "../includes/lighting_structs.inc"

BUFFER_LAYOUT(2,  UBO_CUSTOM_3_ID) uniform Custom0
{
	ProjectionLight	light;
};

// Input attributes
ATTRIB_LOC(0) in vec3 ao_position;

// Output attributes
ATTRIB_LOC(0) out vec4 vo_vTexCoord;
ATTRIB_LOC(1) out vec3 vo_vViewRay;
ATTRIB_LOC(2) out vec3 vo_vLightPosEye;


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
