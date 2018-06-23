#include "../includes/platformdefines.inc"

// FIXME: When defines are working in vulkan include global_3d.inc instead
BUFFER_LAYOUT(1, UBO_GLOBAL_ID) uniform Globals
{
	mat4 	mMatrices[6];
	vec4	lightPos;
	float 	lightFarDist;
};

ATTRIB_LOC(0) in vec4 go_vFragPos;

void main()
{
    // Distance of this fragment from the light postion
    float lightDistance = length(go_vFragPos.xyz - lightPos.xyz);
    
    // Linear depth value for the light
    lightDistance = lightDistance / lightFarDist;
    
    // Manually set the z-buffer value to be this linear depth value
    gl_FragDepth = lightDistance;
}  