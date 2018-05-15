#include "../includes/platformdefines.inc"

in GeometryData
{
    ATTRIB_LOC(0) vec2  go_vPos;
    ATTRIB_LOC(1) vec4 	go_vColor;
    ATTRIB_LOC(2) vec4  go_vBgColor;
    ATTRIB_LOC(3) vec2 	go_vTexcoord;

} geometryData;

SAMPLER_LOC(1, 0) uniform sampler2D sampler0;

layout(location = 0) out vec4 colorOut;

float median(float r, float g, float b)
{
    return max(min(r, g), min(max(r, g), b));
}

const float smoothing = 1.0/64.0;

void main(void)
{	
	const float pxRange = 2.0;	// TODO: Pass this in
    vec2 msdfUnit = pxRange/vec2(textureSize(sampler0, 0));
	vec4 vRead =  texture(sampler0, geometryData.go_vTexcoord).rgba;
    
 	float sigDist = median(vRead.r, vRead.g, vRead.b) - 0.5;
 	sigDist *= dot(msdfUnit, 0.5/fwidth(geometryData.go_vPos));
 	float opacity = clamp(sigDist/fwidth(sigDist) + 0.5, 0.0, 1.0);
 	float stdopacity = smoothstep(0.5 - smoothing, 0.5 + smoothing, vRead.a);
 	//opacity = opacity * stdopacity;
 	colorOut = mix(geometryData.go_vBgColor, geometryData.go_vColor, opacity);
}
