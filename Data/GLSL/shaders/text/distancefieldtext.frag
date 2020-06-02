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

const float smoothing = 1.0/16.0;

void main(void)
{	
	const float pxRange = 2.0;	// TODO: Pass this in
    vec2 msdfUnit = pxRange/vec2(textureSize(sampler0, 0));

	//vec4 vRead =  texture(sampler0, geometryData.go_vTexcoord).rgba;


	vec2 duv = dFdx(geometryData.go_vTexcoord) + dFdy(geometryData.go_vTexcoord);
	// Blurring with neighbours to smooth out
	//duv *= 0.5f;
    vec4 box = vec4(geometryData.go_vTexcoord - duv, geometryData.go_vTexcoord + duv);

    float avgOpacity = 0.0f;

    duv /= 2.f;
    vec2 uv = box.xy;
    for(int i=0; i<4; i++)
    {
    	for(int j=0; j<4; j++)	
    	{
    		vec4 vRead = texture(sampler0,uv).xyzw;

		 	//float sigDist = median(vRead.r, vRead.g, vRead.b) - 0.5;
		 	//sigDist *= dot(msdfUnit, 0.5/fwidth(geometryData.go_vPos));
		 	//float opacity = clamp(sigDist/fwidth(sigDist) + 0.5, 0.0, 1.0);
		 	float stdopacity = smoothstep(0.5 - smoothing, 0.5 + smoothing, vRead.a);

		 	avgOpacity += stdopacity;

    		uv.y += duv.y;
    	}
    	uv.x += duv.x;
    	uv.y = box.y;
    }

    avgOpacity /= 16.0f;
 	colorOut = mix(geometryData.go_vBgColor, geometryData.go_vColor, avgOpacity);

 	//colorOut = mix(geometryData.go_vBgColor, geometryData.go_vColor, stdopacity);

 	//colorOut = mix(geometryData.go_vBgColor, geometryData.go_vColor, opacity);

}
