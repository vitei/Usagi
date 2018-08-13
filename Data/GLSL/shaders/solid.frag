#include "includes/platformdefines.inc"
#include "includes/global_3d.inc"
#include "includes/shadow/poisson_values.inc"
#include "includes/lighting_buffer.inc"
#include "includes/shadow/globalshadow_read.inc"
#include "includes/forward_lighting.inc"
#include "includes/depth_write.inc"

ATTRIB_LOC(0) in vec3 vo_vNormal;
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

	vec4 vDiffuse = vAmbientLight;//vec4(0.0, 0.0, 0.0, 0.0);
	vec4 vSpecular = vec4(0.0, 0.0, 0.0, 0.0);
	DirectionLighting( vo_vNormal, vViewDir, vo_vWorldPos, vo_vEyePosOut.z, vDiffuse, vSpecular, 16.0 );
	vOut *= vDiffuse.xyz;

	OutputLinearDepth(vo_vEyePosOut);
	colorOut = vec4( vOut, 1.0 );

	// OutputDeferredDataLinDepth(vo_vEyePosOut, vo_vNormal, vOut, 0.0);
	// OutputScreenSpaceVelocity(vec4( 0.0, 0.0, 0.0, 0.0 ), vec4( 0.0, 0.0, 0.0, 0.0 ));
}
