#include "includes/platformdefines.inc"


ATTRIB_LOC(0) in vec4 	go_vColor;


layout(location = 0) out vec4 colorOut;

void main(void)
{
    /*
	vec3 vFog = vec3(0.5, 0.5, 1.0);
	vec3 vFogLerp = vo_vLerp.www;
	vFogLerp = clamp(vFogLerp, 0.0, 1.0);
	vOut = (vOut * (vec3(1.0, 1.0, 1.0)-vFogLerp)) + (vFog * vFogLerp);
    */
    
    //OutputLinearDepth(geometryData.vo_vColor.w);

	colorOut = go_vColor;
}
