
#define UBO_GLOBAL_ID	1
#define UBO_MATERIAL_ID 2
#define UBO_INSTANCE_ID 3

#define UBO_CUSTOM0_ID	4
#define UBO_CUSTOM1_ID	5
#define UBO_CUSTOM2_ID	6
#define UBO_CUSTOM3_ID	7

#ifdef PLATFORM_PC
#define BUFFER_LAYOUT(1, val) layout(std140)
#else
#extension GL_EXT_Cafe : enable
#define BUFFER_LAYOUT(1, val) layout(std140, binding=val)
#endif
