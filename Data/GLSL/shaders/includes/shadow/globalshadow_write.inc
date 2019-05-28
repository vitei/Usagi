#ifndef _GLOBAL_SHADOW_INC
#define _GLOBAL_SHADOW_INC


BUFFER_LAYOUT(1, UBO_GLOBAL_ID) uniform Globals
{
	mat4 	mProjMat;
	mat4 	mViewMat;
};


BUFFER_LAYOUT(1, UBO_CUSTOM_0_ID) uniform Instance
{
	mat3x4	mModelMat;
};


#endif

