#ifndef _LAYOUT_3D_GS_INC
#define _LAYOUT_3D_GS_INC


// The VS material data is used for placing the pane in the world rather than the global 2d constants
BUFFER_LAYOUT(2, UBO_MATERIAL_1_ID) uniform Material1
{
	mat4 proj;
};

#endif

