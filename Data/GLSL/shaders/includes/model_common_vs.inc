#ifndef _MODEL_COMMON_VS_INC
#define _MODEL_COMMON_VS_INC


vec4 ApplyWorldTransform(in vec4 vVertex)
{
	return vec4( vVertex * mModelMat, vVertex.w);
}


#endif
