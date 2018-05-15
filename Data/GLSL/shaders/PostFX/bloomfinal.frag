#include "../includes/platformdefines.inc"

BUFFER_LAYOUT(1, UBO_MATERIAL_ID) uniform Material
{
	 float  fBloomScale;       // Bloom process multiplier
};

in vec2 vo_vTexCoord;

SAMPLER_LOC(1, 0) uniform sampler2D sampler0;
SAMPLER_LOC(1, 1) uniform sampler2D sampler1;

layout(location = 0) out vec4 colorOut;

void main(void)
{
    vec4 vSample = texture(sampler0, vo_vTexCoord);    
    vec4 vBloom = texture(sampler1, vo_vTexCoord);

	vSample += fBloomScale * vBloom;
 	
 	// Compute the luma for use by FXAA
 	// The sqrt is only needed if we are in linear colour space
 	vSample.a = sqrt(dot(vSample.rgb, vec3(0.299, 0.587, 0.114)));

    colorOut = vSample;
}

