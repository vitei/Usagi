#include "../includes/platformdefines.inc"
#include "../includes/global_3d.inc"

BUFFER_LAYOUT(1,  UBO_BONE_ID) uniform Bones
{
	mat3x4 	mBonePalette[16];
};


// Input attributes
ATTRIB_LOC(0) in vec3 ao_position;
ATTRIB_LOC(1) in vec4 ao_boneIndex;
ATTRIB_LOC(2) in vec4 ao_boneWeight;

BUFFER_LAYOUT(1,  UBO_MATERIAL_ID) uniform Material
{
	// Materials
	vec4	vScaling0;

} uVSMaterial;

// Output attributes


void ApplyBoneWeight(inout vec4 vVector, in vec4 vVertex, int boneIndex, float boneWeight)
{
	vVector.xyz += ( (vVertex * mBonePalette[boneIndex]) * (boneWeight*uVSMaterial.vScaling0.w));
} 

vec4 ApplyWorldTransformSkinning(in vec4 vVertex)
{
	// TODO: Maybe the indices should be passed as indices
	vec4 vOut = vec4( 0.0, 0.0, 0.0, vVertex.w );
	// Always at least two weights
	if(uVSMaterial.vScaling0.y <= 1.0)
	{
		vOut.xyz = vVertex * mBonePalette[int(ao_boneIndex.x)];
		return vOut;
	}

	ApplyBoneWeight(vOut, vVertex, int(ao_boneIndex.x), ao_boneWeight.x);	
	ApplyBoneWeight(vOut, vVertex, int(ao_boneIndex.y), ao_boneWeight.y);

	if(uVSMaterial.vScaling0.y <= 2.0) return vOut;
	ApplyBoneWeight(vOut, vVertex, int(ao_boneIndex.z), ao_boneWeight.z);

	if(uVSMaterial.vScaling0.y <= 3.0) return vOut;
	ApplyBoneWeight(vOut, vVertex, int(ao_boneIndex.w), ao_boneWeight.w);

	return vOut;
}

#if defined (INSTANCE_TRANSFORM)
vec4 ApplyWorldTransform(in vec4 vVertex)
{
	return vec4( vVertex * ao_transform, vVertex.w);
}
#else
vec4 ApplyWorldTransform(in vec4 vVertex)
{
	if(uVSMaterial.vScaling0.y <= 0.5)
	{
		return vec4( vVertex * mModelMat, vVertex.w);
	}
	else
	{
		return ApplyWorldTransformSkinning( vVertex );
	}
}
#endif


void main(void)
{
	vec4 vWorldPos	= vec4(ao_position*uVSMaterial.vScaling0.xxx, 1.0);
	vWorldPos = ApplyWorldTransform(vWorldPos);
	vec4 vViewPos	= vWorldPos * mViewMat;	// Reverse ordering as a mat3x4
	vec4 vProjPos	= vec4(vViewPos.xyz, 1.0) * mProjMat;

	gl_Position			= vProjPos;
}
