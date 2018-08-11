#ifndef _MODEL_TRANSFORM_VS_INC
#define _MODEL_TRANSFORM_VS_INC

#ifdef HAS_SKELETON
BUFFER_LAYOUT(1,  UBO_BONE_ID) uniform Bones
{
	mat3x4 	mBonePalette[255];
};

void ApplyBoneWeight(inout vec4 vVector, in vec4 vVertex, int boneIndex, float boneWeight)
{
	vVector.xyz += ( (vVertex * mBonePalette[boneIndex]) * (boneWeight));
}

vec4 ApplyWorldTransformSkinning(in vec4 vVertex, in int iBoneCount)
{
	// TODO: Maybe the indices should be passed as indices
	vec4 vOut = vec4( 0.0, 0.0, 0.0, vVertex.w );
	// Always at least two weights
	if(iBoneCount == 1.0)
	{
		vOut.xyz = vVertex * mBonePalette[ao_boneIndex.x];
		return vOut;
	}

	vOut.xyz +=  ( (vVertex * mBonePalette[ao_boneIndex.x]) * ao_boneWeight.x);	
	vOut.xyz +=  ( (vVertex * mBonePalette[ao_boneIndex.y]) * ao_boneWeight.y);	

	if(iBoneCount == 2) return vOut;
	vOut.xyz +=  ( (vVertex * mBonePalette[ao_boneIndex.z]) * ao_boneWeight.z);	
	
	if(iBoneCount == 3) return vOut;
	vOut.xyz +=  ( (vVertex * mBonePalette[ao_boneIndex.w]) * ao_boneWeight.w);	

	return vOut;
}

vec4 ApplyWorldTransform(in vec4 vVertex, in int iBoneCount)
{
	if(iBoneCount == 0)
	{
		return vec4( vVertex * mModelMat, vVertex.w);
	}
	else
	{
		return ApplyWorldTransformSkinning( vVertex, iBoneCount );
	}
}

#else

vec4 ApplyWorldTransform(in vec4 vVertex, in int iBoneCount)
{
	return vec4( vVertex * mModelMat, vVertex.w);
}

#endif


#endif
