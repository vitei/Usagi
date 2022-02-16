#include "../includes/platformdefines.inc"
#include "../includes/global_3d.inc"
#include "../includes/shadow/poisson_values.inc"
#include "../includes/lighting_buffer.inc"
#include "../includes/shadow/globalshadow_read.inc"
#include "../includes/model_common_ps.inc"
#ifndef TRANSLUCENT_PASS
#include "../includes/depth_write.inc"
#endif
#ifdef DEFERRED_SHADING
#include "../includes/deferred.inc"
#else
#include "../includes/forward_lighting.inc"
#endif


// <<GENERATED_CODE>>

#ifndef SHADOW_PASS
ATTRIB_LOC(0) in vec4 vo_vTexCoord01;
ATTRIB_LOC(1) in vec4 vo_vTexCoord23;
ATTRIB_LOC(2) in vec4 vo_vColor;
ATTRIB_LOC(3) in vec3 vo_vNormal;
#ifdef HAS_BUMP
ATTRIB_LOC(4) in vec3 vo_vTangent;
ATTRIB_LOC(5) in vec3 vo_vBinormal;
#endif
#ifndef OMNI_DEPTH
ATTRIB_LOC(6) in vec3 vo_vWorldPos;
ATTRIB_LOC(7) in vec3 vo_vViewDir;
#endif
#endif

#ifndef DEFERRED_SHADING
#ifndef SHADOW_PASS
layout(location = 0) out vec4 colorOut;
#endif
#endif


// Entry point
void main(void)
{
#ifndef SHADOW_PASS
	vec3	vSrcColArg[3];
	float	vSrcAlphaArg[3];

	vec3 vNormal = vo_vNormal;

#ifdef HAS_BUMP
	if(uMaterial.bBumpMap)
	{
        // FIXME: Use 2 component dxt5 at some point
        vec3 nrm_l = texture(sampler1, vo_vTexCoord01.zw).xyz;
        nrm_l.xyz = normalize((nrm_l * 2.0) - 1.0);
        vec3 tan_v = normalize(vo_vTangent);
        vec3 binrm_v = normalize(vo_vBinormal);
        vNormal = normalize(nrm_l.x * tan_v + nrm_l.y * binrm_v + nrm_l.z * vNormal);
    }
#endif
	vec3  vViewDir	= normalize( vo_vViewDir );

	vec3 vEmissive = vec3(0.0, 0.0, 0.0);
	vec4 vSpecular = vec4(1.0);

	if(uMaterial.bEmissiveMap)
	{
    	vEmissive = texture( sampler2, vo_vTexCoord01.xy ).rgb; 
	}
	
	vEmissive += uMaterial.emission.rgb;

#ifndef DEFERRED_SHADING
	vec4 vFragPrimaryColor = vec4(0);
	vec4 vFragSecondaryColor = vec4(0.0, 0.0, 0.0, 0.0);
	DirectionLighting( vNormal, vViewDir, vo_vWorldPos, -vo_vViewDir.z, vFragPrimaryColor, vFragSecondaryColor, uMaterial.specularpow);
	PointLighting( -vo_vViewDir, vNormal, vViewDir, vFragPrimaryColor, vFragSecondaryColor, uMaterial.specularpow);
	SpotLighting( -vo_vViewDir, vNormal, vViewDir, vFragPrimaryColor, vFragSecondaryColor, uMaterial.specularpow);

	vFragPrimaryColor += vAmbientLight;
#endif

	vec4 vSpecRead = vec4(1.0, 1.0, 1.0, 0.0);
	if(uMaterial.bSpecMap)
	{
    	vSpecRead = texture( sampler3, vo_vTexCoord01.zw ); 
	}
	else
	{
		// TODO: Should be applied regardless but defaults coming through too low
		vSpecular.rgb *= uMaterial.specular.rgb;
	}
	vSpecular.rgb *= vSpecRead.rgb; 
	vSpecular *= vo_vColor;

	if(uMaterial.bReflectionMap)
	{
		vec3 fvReflection =  normalize(reflect(vo_vViewDir, vNormal));
		fvReflection = (vec4(fvReflection, 0.0) * mInvViewMat).rgb;
		vEmissive += (texture( sampler4, fvReflection ).rgb) * uMaterial.reflectionfactor * vSpecRead.aaa;
	}


	// other lighting methods will be here.

	vec4 vDiffuse = vec4(1.0, 1.0, 1.0, 1.0);

	if(uMaterial.bDiffuseMap)
	{
		vec4 vRead = texture(sampler0, vo_vTexCoord01.xy);
		vDiffuse.rgb *= vRead.rgb;
		vDiffuse.a = vRead.a;
	}
	vDiffuse *= vo_vColor;
	vDiffuse *= uMaterial.diffuse;

	vec3 rim = vec3(GetRimValue(1.0, 4.0, vNormal.z)) * vSpecular.rgb;

	//vOut.xyz += rim;
	//vOut.xyz += ApplyHemisphereLighting(vec3(0.4)*vSpecular.rgb, vNormal);

#ifdef DEFERRED_SHADING
	// TODO: Make all specular single channel only
	OutputDeferredDataLinDepth(-vo_vViewDir.z, vNormal, vDiffuse.rgb, vEmissive, vSpecular.rgb, uMaterial.specularpow);
#else
	vec4 vOut;
	vOut.rgb = (vDiffuse.rgb * vFragPrimaryColor.rgb) + (vSpecular.rgb * vFragSecondaryColor.rgb);
	vOut.a = vDiffuse.a;
	vOut.xyz += vEmissive;
#ifndef TRANSLUCENT_PASS
	OutputLinearDepth(-vo_vViewDir.z);
#endif
	vOut.a *= uMaterial.alpha;
	//vOut.a = 1.0;
	colorOut = vOut;
#endif

#else
	// Shadow pass
	// colorOut = vec4(1.0, 0.0, 0.0, 0.0);
#endif
}
