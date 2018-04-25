#include "../includes/platformdefines.inc"
#define SAMPLE_COUNT 4

BUFFER_LAYOUT(1, UBO_MATERIAL_ID) uniform Material
{
	vec4 vOffsets[4];
};

in vec2 vo_vTexCoord;

SAMPLER_LOC(1, 0) uniform sampler2D sampler0;

layout(location = 0) out vec4 colorOut;
// TODO: This is more efficently done with a linear sampler than with multiple tex reads

void main(void)
{	
    vec4 sample = vec4(0.0, 0.0, 0.0, 0.0);

	for( int i=0; i < 4; i++ )
	{
		sample += texture( sampler0, vo_vTexCoord + vOffsets[i].xy );
	}
    
	colorOut = sample / vec4(4.0, 4.0, 4.0, 4.0);
}
