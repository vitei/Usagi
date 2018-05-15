#include "includes/platformdefines.inc"
#include "includes/global_3d.inc"
#include "includes/depth_read.inc"

ATTRIB_LOC(0) in vec3 vo_vTexCoord;
ATTRIB_LOC(1) in vec4 vo_vScreenTex;
ATTRIB_LOC(2) in vec3 vo_vViewRay;

SAMPLER_LOC(1, 0) uniform samplerCube sampler0;

layout(location = 0) out vec4 colorOut;



void main(void)
{   
	colorOut = vec4(texture(sampler0, vo_vTexCoord).rgb, 1.0f);
}

