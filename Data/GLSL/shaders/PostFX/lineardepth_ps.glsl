#include "../includes/platformdefines.inc"
#include "../includes/global_3d.inc"

in vec2 vo_vTexCoord;
in vec3 vo_vFSVector;

layout(location = 0) out vec4 colorOut;

SAMPLER_LOC(1, 0) uniform sampler2D sampler0;

// Massively inefficent, we only want the z component, but for now it's ok
vec3 VSPositionFromDepth(vec2 vTexCoord)
{
    // Get the depth value for this pixel
    float z = (texture(sampler0, vTexCoord).r*2.0)-1.0;  	// OpenGL stores the depth in the range -1 to 1, not 0 to 1
    // Get x/w and y/w from the viewport position
    float x = vTexCoord.x * 2.0 - 1.0;
    float y = (vTexCoord.y) * 2.0 - 1.0;
    vec4 vProjectedPos = vec4(x, y, z, 1.0);
    // Transform by the inverse projection matrix
    vec4 vPositionVS = vProjectedPos * mInvProjMat;  
    // Divide by w to get the view-space position
    return vPositionVS.xyz / vPositionVS.w;  
}

void main(void)
{
	vec3 vPos = VSPositionFromDepth(vo_vTexCoord);
	float fLinDepth = vPos.z/vNearFar.y;

	colorOut	= vec4(fLinDepth, fLinDepth, fLinDepth, fLinDepth);
}
