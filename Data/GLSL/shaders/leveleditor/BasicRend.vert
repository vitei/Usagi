#include "../includes/platformdefines.inc"
#include "../includes/global_3d.inc"

BUFFER_LAYOUT(1,  UBO_CUSTOM0_ID) uniform Custom0
{
	mat4 	mTransform;
	vec4	vColour;
	bool	bLit;
};


// Input attributes
ATTRIB_LOC(0) in vec3 ao_position;
ATTRIB_LOC(1) in vec3 ao_normal;

// Output attributes
ATTRIB_LOC(0) out vec3 vo_vWorldPos;
ATTRIB_LOC(1) out vec3 vo_vNormal;

void main(void)
{
	vec4 vWorld = vec4(ao_position, 1.0) * mTransform;
	vec4 vViewPos = vWorld * mViewMat;
	vec4 vProjPos = vViewPos * mProjMat;
	if(bLit)
	{
		vo_vWorldPos = vWorld.xyz;
		vo_vNormal = (vec4(ao_normal, 0.0) * mTransform).xyz;
	}

	gl_Position = vProjPos;
}
