
#include "../includes/platformdefines.inc"
#include "../includes/global_3d.inc"

// <<GENERATED_CODE>>

#include "../includes/model_transform.inc"

// Output attributes
ATTRIB_LOC(0) out vec4 vo_vTexCoord01;
#ifndef SHADOW_PASS
ATTRIB_LOC(1) out vec4 vo_vTexCoord23;
ATTRIB_LOC(2) out vec4 vo_vColor;
ATTRIB_LOC(3) out vec3 vo_vNormal;
#endif
#ifdef HAS_BUMP
ATTRIB_LOC(4) out vec3 vo_vTangent;
ATTRIB_LOC(5) out vec3 vo_vBinormal;
#endif
#ifndef OMNI_DEPTH
ATTRIB_LOC(6) out vec3 vo_vWorldPos;
ATTRIB_LOC(7) out vec3 vo_vViewDir;
#endif



void main(void)
{
	vec4 vWorldPos	= vec4(ao_position, 1.0);
	vWorldPos = ApplyWorldTransform(vWorldPos, uVSMaterial.iBoneCount);

	vec4 vTmp = vec4(0.0, 0.0, 1.0, 1.0);

	vTmp.xy = ao_uv0;
	vo_vTexCoord01.x = dot(uVSMaterial.mTexMatrix[0][0].xywz, vTmp);
	vo_vTexCoord01.y = dot(uVSMaterial.mTexMatrix[0][1].xywz, vTmp);

	vTmp.xy = ao_uv1;
	vo_vTexCoord01.z = dot(uVSMaterial.mTexMatrix[1][0].xywz, vTmp);
	vo_vTexCoord01.w = dot(uVSMaterial.mTexMatrix[1][1].xywz, vTmp);

#ifndef SHADOW_PASS	
	vTmp.xy = ao_uv2;
	vo_vTexCoord23.x = dot(uVSMaterial.mTexMatrix[2][0].xywz, vTmp);
	vo_vTexCoord23.y = dot(uVSMaterial.mTexMatrix[2][1].xywz, vTmp);
	

	vTmp.xy = ao_uv3;
	vo_vTexCoord23.z = dot(uVSMaterial.mTexMatrix[3][0].xywz, vTmp);
	vo_vTexCoord23.w = dot(uVSMaterial.mTexMatrix[3][1].xywz, vTmp);
#endif	
	

#ifndef SHADOW_PASS
	// Transform the normal into world space
	vec3 vNormal		= ApplyWorldTransform(vec4( ao_normal, 0.0 ), uVSMaterial.iBoneCount).xyz;
	vec3 vViewNormal	= (vec4( vNormal, 0.0 ) * mViewMat).xyz;

#ifdef HAS_BUMP
	if(uVSMaterial.bBumpMap)
	{
		vec3 vTangent		= ApplyWorldTransform(vec4( ao_tangent, 0.0 ), uVSMaterial.iBoneCount).xyz;
		vec3 vViewTangent	= (vec4( vTangent, 0.0 ) * mViewMat).xyz;
		vo_vTangent = vViewTangent;
	//	if(uVSMaterial.bBinormal)
		{
			vec3 vBinormal		= ApplyWorldTransform(vec4( ao_binormal, 0.0 ), uVSMaterial.iBoneCount).xyz;
			vo_vBinormal	= (vec4( vBinormal, 0.0 ) * mViewMat).xyz;
		}
	/*	else
		{
			vo_vBinormal = cross(vTangent, vViewNormal);
		}*/
	}
#endif

	vo_vColor = ao_color;
	vo_vNormal = vViewNormal;
#endif

#ifdef OMNI_DEPTH
	gl_Position			= vWorldPos;
#else
	vec4 vViewPos	= vWorldPos * mViewMat;	// Reverse ordering as a mat3x4
	vec4 vProjPos	= vec4(vViewPos.xyz, 1.0) * mProjMat;

	vo_vViewDir			= -vViewPos.xyz;
	

	vo_vWorldPos		= vWorldPos.xyz;

	gl_Position			= vProjPos;
#endif
}
