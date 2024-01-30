#include "../includes/platformdefines.inc"

ATTRIB_LOC(0) in vec2 go_vPos;
ATTRIB_LOC(1) in vec4 go_vColor;
ATTRIB_LOC(2) in vec4 go_vBgColor;
ATTRIB_LOC(3) in vec2 go_vTexCoord;
ATTRIB_LOC(4) in vec4 go_vFgColor;


SAMPLER_LOC(1, 0) uniform sampler2D sampler0;

layout(location = 0) out vec4 colorOut;

const float smoothing = 1.0/16.0;

void main(void)
{	
	const float pxRange = 2.0;	// TODO: Pass this in
    vec2 msdfUnit = pxRange/vec2(textureSize(sampler0, 0));

	//vec4 vRead =  texture(sampler0, go_vTexCoord).rgba;


	vec2 duv = dFdx(go_vTexCoord) + dFdy(go_vTexCoord);
	// Blurring with neighbours to smooth out
	//duv *= 0.5f;
    vec4 box = vec4(go_vTexCoord - duv, go_vTexCoord + duv);

    float avgOpacity = 0.0f;

    duv /= 2.f;
    vec2 uv = box.xy;
    for(int i=0; i<4; i++)
    {
    	for(int j=0; j<4; j++)	
    	{
    		vec4 vRead = texture(sampler0,uv).xyzw;

		 	//float sigDist = median(vRead.r, vRead.g, vRead.b) - 0.5;
		 	//sigDist *= dot(msdfUnit, 0.5/fwidth(go_vPos));
		 	//float opacity = clamp(sigDist/fwidth(sigDist) + 0.5, 0.0, 1.0);
		 	float stdopacity = smoothstep(0.5 - smoothing, 0.5 + smoothing, vRead.a);

		 	avgOpacity += stdopacity;

    		uv.y += duv.y;
    	}
    	uv.x += duv.x;
    	uv.y = box.y;
    }

    avgOpacity /= 16.0f;
 	colorOut = mix(go_vBgColor, go_vColor, avgOpacity);

}
