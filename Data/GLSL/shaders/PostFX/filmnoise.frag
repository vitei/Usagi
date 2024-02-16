#include "../includes/platformdefines.inc"


BUFFER_LAYOUT(1, UBO_MATERIAL_ID) uniform Material
{
	vec2  vResolution;
	float fTime;
};

ATTRIB_LOC(0) in vec2 vo_vTexCoord;
SAMPLER_LOC(1, 0) uniform sampler2D sampler0;
SAMPLER_LOC(1, 1) uniform sampler3D sampler1;

float luma(vec3 vSample)
{
    return sqrt(dot(vSample.rgb, vec3(0.299, 0.587, 0.114)));
}

layout(location = 0) out vec4 colorOut;

void main(void)
{	
   vec3 texColor = texture(sampler0, vo_vTexCoord).rgb;

    float zoom = 1.f/1024.f;//0.002;
    float strength = 1.0f;
    vec3 noiseUV = vec3(vo_vTexCoord * vResolution * zoom, fTime);

#if 0
    vec3 g = texture(sampler1, noiseUV ).rgb - vec3(0.5);

    // Scale intensity
    g *= vec3(0.5);

    colorOut = vec4(texColor + g, 1.0);

#else
    vec3 overlay = texture(sampler1, noiseUV ).rrr;
    float luminance = luma(texColor);
    vec3 result;
    result.r = mix(1 - (1-2 *(texColor.r-0.5)) * (1 - overlay.r), 2 * texColor.r * overlay.r, step( texColor.r, 0.5 ));
    result.g = mix(1 - (1-2 *(texColor.g-0.5)) * (1 - overlay.g), 2 * texColor.g * overlay.g, step( texColor.g, 0.5 ));
    result.b = mix(1 - (1-2 *(texColor.b-0.5)) * (1 - overlay.b), 2 * texColor.b * overlay.b, step( texColor.b, 0.5 ));
 
    result = mix(texColor, result, strength);

    colorOut = vec4(result, 1.0);
#endif
    
    //colorOut = vec4(texture(sampler1, noiseUV ).rgb, 1.0);

}
