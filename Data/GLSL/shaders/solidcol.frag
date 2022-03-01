#include "includes/platformdefines.inc"
#include "includes/global_3d.inc"

ATTRIB_LOC(0) in vec4 vo_vColor;
ATTRIB_LOC(1) in vec3 vo_vPosition;
ATTRIB_LOC(2) in vec3 vo_vEyePosOut;
ATTRIB_LOC(3) in vec3 vo_vWorldPos;

// in vec4 vo_vScreenPos;
// in vec4 vo_vPrevScreenPos;

layout(location = 0) out vec4 colorOut;

void main(void)
{
	// PerformClipping();

	vec3 vOut = vec3( 1.0, 0.0, 0.0 );
	vec3 vViewDir = normalize( -vo_vEyePosOut );


//	OutputLinearDepth(vo_vEyePosOut);
	colorOut = vo_vColor;

	// OutputDeferredDataLinDepth(vo_vEyePosOut, vo_vNormal, vOut, 0.0);
	// OutputScreenSpaceVelocity(vec4( 0.0, 0.0, 0.0, 0.0 ), vec4( 0.0, 0.0, 0.0, 0.0 ));
}
