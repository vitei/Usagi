#include "includes/platformdefines.inc"
#include "includes/global_3d.inc"
#include "includes/depth_read.inc"

in vec3 vo_vTexCoord;
in vec4 vo_vScreenTex;
in vec3 vo_vViewRay;

SAMPLER_LOC(1, 0) uniform samplerCube sampler0;

layout(location = 0) out vec4 colorOut;



void main(void)
{   
	colorOut = vec4(texture(sampler0, vo_vTexCoord).rgb, 1.0f);
}

