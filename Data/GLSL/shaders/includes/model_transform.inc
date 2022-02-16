#ifndef _MODEL_TRANSFORM_VS_INC
#define _MODEL_TRANSFORM_VS_INC

#ifdef HAS_SKELETON
BUFFER_LAYOUT(1,  UBO_CUSTOM_2_ID) uniform Bones
{
	mat3x4 	mBonePalette[255];
};

void ApplyBoneWeight(inout vec4 vVector, in vec4 vVertex, int boneIndex, float boneWeight)
{
	vVector.xyz += ( (vVertex * mBonePalette[boneIndex]) * (boneWeight));
}


vec4 ApplyWorldTransform(in vec4 vVertex, in int iBoneCount)
{
	vec4 vOut = vec4( vVertex * mBonePalette[ao_boneIndex.x] * ao_boneWeight.x, vVertex.w );
	
	if(iBoneCount == 1) return vOut;
	vOut.xyz +=  ( (vVertex * mBonePalette[ao_boneIndex.y]) * ao_boneWeight.y);	

	if(iBoneCount == 2) return vOut;
	vOut.xyz +=  ( (vVertex * mBonePalette[ao_boneIndex.z]) * ao_boneWeight.z);	
	
	if(iBoneCount == 3) return vOut;
	vOut.xyz +=  ( (vVertex * mBonePalette[ao_boneIndex.w]) * ao_boneWeight.w);	

	return vOut;
}

#else

vec4 ApplyWorldTransform(in vec4 vVertex, in int iBoneCount)
{
	return vec4( vVertex * mModelMat, vVertex.w);
}

#endif


#endif
