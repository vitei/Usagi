#include "../includes/platformdefines.inc"
#include "../includes/global_3d.inc"
//include "../includes/shadow/globalshadow_read.inc"
#include "../includes/lighting_buffer.inc"
#include "../includes/depth_write.inc"
#include "../includes/forward_lighting.inc"
#include "../includes/model_common_ps.inc"


BUFFER_LAYOUT(1, UBO_MATERIAL_1_ID) uniform Material1
{
	// Materials
	vec4	emission;
	vec4	diffuse;
	bool	bDiffuseMap;
	bool	bBumpMap;
	bool	bSpecMap;
	bool 	bEmissiveMap;
	bool 	bReflectionMap;
} uMaterial;



SAMPLER_LOC(1, 0) uniform sampler2D sampler0;
SAMPLER_LOC(1, 1) uniform sampler2D sampler1;
SAMPLER_LOC(1, 2) uniform sampler2D sampler2;
SAMPLER_LOC(1, 3) uniform sampler2D sampler3;
SAMPLER_LOC(1, 4) uniform samplerCube sampler4;

in vec4 vo_vTexCoord01;
in vec4 vo_vTexCoord23;
in vec4 vo_vColor;
in vec3 vo_vNormal;
in vec3 vo_vTangent;
in vec3 vo_vBinormal;
in vec3 vo_vWorldPos;
in vec3 vo_vViewDir;

layout(location = 0) out vec4 colorOut;


// Entry point
void main(void)
{
	vec3	vSrcColArg[3];
	float	vSrcAlphaArg[3];

	vec3 vNormal = vo_vNormal;

	if(uMaterial.bBumpMap)
	{
        // FIXME: Use 2 component dxt5 at some point
        vec3 nrm_l = texture(sampler1, vo_vTexCoord01.zw).xyz;;
        nrm_l.xyz = normalize((nrm_l * 2.0) - 1.0);
        vec3 tan_v = normalize(vo_vTangent);
        vec3 binrm_v = normalize(vo_vBinormal);
        vNormal = normalize(nrm_l.x * tan_v + nrm_l.y * binrm_v + nrm_l.z * vNormal);
    }

	vec3  vViewDir	= normalize( vo_vViewDir );

	vec4 vFragPrimaryColor = vec4(0);
	vec4 vFragSecondaryColor = vec4(0.0, 0.0, 0.0, 0.0);
	vec3 vEmissive = vec3(0.0, 0.0, 0.0);
	vec4 vSpecular = vec4(1.0);


	if(uMaterial.bEmissiveMap)
	{
    	vEmissive = texture( sampler2, vo_vTexCoord01.xy ).rgb; 
	}

	vFragPrimaryColor = clamp(vFragPrimaryColor, 0.0, 1.0);
	vFragSecondaryColor = clamp(vFragSecondaryColor, 0.0, 1.0);

	DirectionLighting( vNormal, vViewDir, vo_vWorldPos, -vo_vViewDir.z, vFragPrimaryColor, vFragSecondaryColor, 16.0);
	PointLighting( -vo_vViewDir, vNormal, vViewDir, vFragPrimaryColor, vFragSecondaryColor, 16.0);
	SpotLighting( -vo_vViewDir, vNormal, vViewDir, vFragPrimaryColor, vFragSecondaryColor, 16.0);

	vFragPrimaryColor += vAmbientLight;
	vFragPrimaryColor  *= uMaterial.diffuse;

	if(uMaterial.bSpecMap)
	{
    	vSpecular.rgb *= texture( sampler3, vo_vTexCoord01.zw ).rgb; 
	}
	vSpecular *= vo_vColor;

	if(uMaterial.bReflectionMap)
	{
		vec3 fvReflection =  normalize(reflect(vec3(0.0, 0.0, 1.0), vNormal));
		vEmissive += (texture( sampler4, fvReflection ).rgb) * vSpecular.rgb * vec3(0.5);
	}


	// other lighting methods will be here.

	vec4 vResult = vec4(1.0, 1.0, 1.0, 1.0);
	vec4 vResultBuffer = vec4(0.0, 0.0, 0.0, 0.0);

	if(uMaterial.bDiffuseMap)
	{
		vResult *= texture(sampler0, vo_vTexCoord01.xy);
	}
	vResult *= vo_vColor;

	vec3 rim = vec3(GetRimValue(1.0, 4.0, vNormal.z)) * vSpecular.rgb;


	vec4 vOut = (vResult * vFragPrimaryColor) + (vSpecular * vFragSecondaryColor);
	vOut.xyz += vEmissive;
	vOut.xyz += rim;
	//vOut.xyz += ApplyHemisphereLighting(vec3(0.4)*vSpecular.rgb, vNormal);

	OutputLinearDepth(-vo_vViewDir.z);

	colorOut = vOut;

}
