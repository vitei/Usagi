#ifndef _GLOBAL_2D_INC
#define _GLOBAL_2D_INC


// Constant buffer for the projection matrix.
BUFFER_LAYOUT(1, UBO_GLOBAL_ID) uniform Globals
{
	mat4 proj;
};

#endif

