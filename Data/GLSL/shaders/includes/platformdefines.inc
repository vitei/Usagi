#if 1
#ifndef PLATFORM_PC
#version 330
#endif
#extension GL_ARB_shading_language_420pack : enable
//#extension GL_ARB_separate_shader_objects : enable
//#extension GL_KHR_vulkan_glsl : enable
#define BUFFER_LAYOUT(setIn, val) layout(binding = val, std140) // set = setIn,
#define SAMPLER_LOC(setIn, val) layout(binding=val) // set = setIn,
#define ATTRIB_LOC(val) layout(location=val)

#define SAMPLER_MSAA sampler2D


vec2 GetRTUV(in vec2 texCoord)
{
	return texCoord;
}

vec2 GetUV(in vec2 texCoord)
{
	return vec2(texCoord.x, 1.0-texCoord.y);
}

vec3 GetUV(in vec3 texCoord)
{
	return vec3(texCoord.x, 1.0-texCoord.y, texCoord.z);
}

vec4 GetUV(in vec4 texCoord)
{
	//return vec4(texCoord.x, 1.0-texCoord.y, texCoord.z, texCoord.w);
	return texCoord;
}


#elif (defined PLATFORM_OSX)

#define BUFFER_LAYOUT(1, set, val) layout(std140)
//#define SAMPLER_LOC(1, val) layout(location=val)

#define SAMPLER_LOC(1, val)

#define SAMPLER_MSAA sampler2D

vec2 GetRTUV(in vec2 texCoord)
{
	return texCoord;
}

vec2 GetUV(in vec2 texCoord)
{
	return vec2(texCoord.x, 1.0-texCoord.y);
}

vec3 GetUV(in vec3 texCoord)
{
	return vec3(texCoord.x, 1.0-texCoord.y, texCoord.z);
}

vec4 GetUV(in vec4 texCoord)
{
	//return vec4(texCoord.x, 1.0-texCoord.y, texCoord.z, texCoord.w);
	return texCoord;
}

#endif



#define USE_LINEAR_DEPTH 1

#ifdef PLATFORM_PC

#define UBO_GLOBAL_ID		1
#define UBO_MATERIAL_ID 	2
#define UBO_MATERIAL_1_ID	3
#define UBO_INSTANCE_ID 	4
#define UBO_INSTANCE_1_ID 	5
#define UBO_BONE_ID			6
#define UBO_CUSTOM0_ID		7
// The below are not valid on the CTR
#define UBO_LIGHTING_ID 8
#define UBO_SHADOW_ID 	9
#define UBO_COLORS		10

#else

#define UBO_GLOBAL_ID		0
#define UBO_MATERIAL_ID 	1
#define UBO_MATERIAL_1_ID	1
#define UBO_INSTANCE_ID 	2
#define UBO_INSTANCE_1_ID 	2
#define UBO_BONE_ID			3
#define UBO_CUSTOM0_ID		3
// The below are not valid on the CTR
#define UBO_LIGHTING_ID 	4
#define UBO_SHADOW_ID 		5	// Slower access from this point on on the switch
#define UBO_COLORS			6

#endif


