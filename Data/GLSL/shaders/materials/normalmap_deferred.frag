#include "../includes/platformdefines.inc"
#include "../includes/global_3d.inc"
#include "../includes/depth_write.inc"
#include "../includes/deferred.inc"

SAMPLER_LOC(1, 0) uniform sampler2D sampler0;
SAMPLER_LOC(1, 1) uniform sampler2D sampler1;
SAMPLER_LOC(1, 2) uniform sampler2D sampler2;
SAMPLER_LOC(1, 3) uniform sampler2D sampler3;
SAMPLER_LOC(1, 4) uniform sampler2D sampler4;

in vec2 vo_vTexCoord;
in vec3 vo_vEyeNormal;
in vec3 vo_vEyeTangent;
in vec3 vo_vEyeBinormal;
in vec3 vo_vEyePosOut;


void main(void)
{
	vec3 vDiffuse =  texture(sampler0, vo_vTexCoord).xyz;
	vec3 vNormal = texture(sampler1, vo_vTexCoord).xyz;

	vNormal = (vNormal * 2.0) - 1.0;

	mat3 tangentToView = mat3(vo_vEyeTangent.x, vo_vEyeBinormal.x, vo_vEyeNormal.x,
                           vo_vEyeTangent.y, vo_vEyeBinormal.y, vo_vEyeNormal.y,
                           vo_vEyeTangent.z, vo_vEyeBinormal.z, vo_vEyeNormal.z);

	vNormal = vNormal * tangentToView;
	
	// For now just using the green channel for the specular
	OutputDeferredDataLinDepth(vo_vEyePosOut,vNormal, vDiffuse, vec3(0.0, 0.0, 0.0), vec3(1.0, 1.0, 1.0), vDiffuse.g);

	//colorOut = vec4(1.0, 0.0, 0.0, 1.0);
}
