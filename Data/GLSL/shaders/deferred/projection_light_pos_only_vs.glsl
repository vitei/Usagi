#include "../includes/platformdefines.inc"
#include "../includes/global_3d.inc"
#include "../includes/lighting_structs.inc"

BUFFER_LAYOUT(1,  UBO_CUSTOM0_ID) uniform Custom0
{
	ProjectionLight	light;
};


// Input attributes
in vec3 ao_position;

void main(void)
{
	vec4 vWorldPos = vec4(ao_position, 1.0) * light.mViewProjInv;
	vec4 vViewPos = vWorldPos * mViewMat;	
	vec4 vTransformedPos = vViewPos * mProjMat;


	gl_Position = vTransformedPos;
}
