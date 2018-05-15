#define SAMPLE_COUNT 16
#include "../includes/platformdefines.inc"

BUFFER_LAYOUT(1, UBO_MATERIAL_ID) uniform Material
{
	vec4 vOffsets[SAMPLE_COUNT];
};

ATTRIB_LOC(0) in vec2 vo_vTexCoord;

SAMPLER_LOC(1, 0) uniform sampler2D sampler0;

layout(location = 0) out vec4 colorOut;

void main(void)
{
    vec4 vColorSample = vec4(0.0, 0.0, 0.0, 0.0);

	for( int i=0; i < SAMPLE_COUNT; i++ )
	{
		vColorSample += texture( sampler0, vo_vTexCoord + vOffsets[i].xy );
	}
    
	colorOut = vColorSample / vec4(16.0, 16.0, 16.0, 16.0);
}
