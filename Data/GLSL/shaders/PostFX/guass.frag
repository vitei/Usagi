#define SAMPLE_COUNT 13
#include "../includes/platformdefines.inc"

// TODO: May be faster to pass the sample offsets as various texture co-ordinates
BUFFER_LAYOUT(1, UBO_MATERIAL_ID) uniform Material
{
	vec4  vOffsets[SAMPLE_COUNT];
};

SAMPLER_LOC(1, 0) uniform sampler2D sampler0;

in vec2 vo_vTexCoord;

layout(location = 0) out vec4 colorOut;

void main(void)
{
    vec4 colorSample = vec4(0.0, 0.0, 0.0, 0.0);

    // TODO: Easy optimization, we are reliant on previous texture reads, couldn't be slower
	for( int i=0; i < SAMPLE_COUNT; i++ )
	{
		colorSample += texture( sampler0, vo_vTexCoord + vOffsets[i].xy ) * vOffsets[i].zzzz;
	}

	colorOut = colorSample;
}
