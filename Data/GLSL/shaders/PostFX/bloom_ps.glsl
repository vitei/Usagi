#include "../includes/platformdefines.inc"

#define SAMPLE_COUNT 15

BUFFER_LAYOUT(1, UBO_MATERIAL_ID) uniform Material
{
	vec4  vOffsets[SAMPLE_COUNT];
};

in vec2 vo_vTexCoord;

SAMPLER_LOC(1, 0) uniform sampler2D sampler0;

layout(location = 0) out vec4 colorOut;

void main(void)
{
    vec4 vSample = vec4(0.0, 0.0, 0.0, 0.0);
    vec4 vColor = vec4(0.0, 0.0, 0.0, 0.0);
        
    vec2 vSamplePosition;
    
    // Perform a one-directional gaussian blur
    for(int iSample = 0; iSample < SAMPLE_COUNT; iSample++)
    {
        vSamplePosition = vo_vTexCoord + vOffsets[iSample].xy;
        vColor = texture(sampler0, vSamplePosition);
        vSample += vOffsets[iSample].zzzz*vColor;
    }
    
    colorOut = vSample;
}