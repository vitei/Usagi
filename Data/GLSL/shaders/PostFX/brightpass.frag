#include "../includes/platformdefines.inc"

#define  BRIGHT_PASS_THRESHOLD	4.0
#define  BRIGHT_PASS_OFFSET		10.0

in vec2 vo_vTexCoord;
SAMPLER_LOC(1, 0) uniform sampler2D sampler0;

BUFFER_LAYOUT(1, UBO_MATERIAL_ID) uniform Material
{
	float fMiddleGray;
};

layout(location = 0) out vec4 colorOut;

void main(void)
{
	vec4 vSample = texture( sampler0, vo_vTexCoord );
	float fAdaptedLum = 0.0f;//tex2D( s1, float2(0.5f, 0.5f) ).r;
	
	// Determine what the pixel's value will be after tone-mapping occurs
	vSample.rgb *= fMiddleGray/(fAdaptedLum + 0.001f);
	
	// Subtract out dark pixels
	vSample.rgb -= BRIGHT_PASS_THRESHOLD;
	
	// Clamp to 0
	vSample = max(vSample, 0.0f);
	
	// Map the resulting value into the 0 to 1 range. Higher values for
	// BRIGHT_PASS_OFFSET will isolate lights from illuminated scene 
	// objects.
	vSample.r /= (BRIGHT_PASS_OFFSET+vSample.r);
	vSample.g /= (BRIGHT_PASS_OFFSET+vSample.g);
	vSample.b /= (BRIGHT_PASS_OFFSET+vSample.b);
    
	colorOut = vSample;
}
