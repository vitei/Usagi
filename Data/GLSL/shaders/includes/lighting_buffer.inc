#ifndef _LIGHTING_BUFFER_INC
#define _LIGHTING_BUFFER_INC

#define MAX_LIGHTS 8

// Structures are passed by value so confine this to as small a size as possible
// Forward shaded point/ spot lights should be avoided in general
struct Light
{
	vec4	vPosition;
	vec4	vDirection;
	vec4	vDiffuse;
	vec4	vAmbient;
	vec4	vSpecular;
	vec4	vRange;
	float	fCosSpotCutoff;
	float	fOuterSpotRadius;
	float	fCosInnerSpotCutoff;
};


// All lights (the lights needed for forward shading are always set)
BUFFER_LAYOUT(0, UBO_LIGHTING_ID) uniform Lighting
{
	// Will always be dirlights then point lights
	int 		iDirLightCount;
	int 		iCascadeLightStart;	// The first shadowed light, should be ordered so the first is the brightest
	int 		iPointLightCount;
	int 		iSpotLightCount;
	vec4		vAmbientLight;
	Light 		lights[MAX_LIGHTS];
};




#endif
