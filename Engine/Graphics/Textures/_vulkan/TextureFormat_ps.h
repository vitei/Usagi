/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef _USG_GRAPHICS_VULKAN_TEXTURE_FORMAT_H
#define _USG_GRAPHICS_VULKAN_TEXTURE_FORMAT_H
#include "Engine/Common/Common.h"
#include OS_HEADER(Engine/Graphics/Device, VulkanIncludes.h)


static const VkFormat gColorFormatMap[] =
{
	VK_FORMAT_R8G8B8A8_UNORM,					// TF_RGBA_8888
	VK_FORMAT_R5G5B5A1_UNORM_PACK16,			// TG_RGBA_5551
	VK_FORMAT_B5G6R5_UNORM_PACK16,				// CF_RGB_565,
	VK_FORMAT_B4G4R4A4_UNORM_PACK16,			// CF_RGBA_4444,
	VK_FORMAT_R8G8B8_UNORM,						// CF_RGB_888,
	VK_FORMAT_R32_SFLOAT,						// CF_SHADOW,
	VK_FORMAT_R16G16B16A16_SFLOAT,				// CF_RGBA_16F
	VK_FORMAT_B10G11R11_UFLOAT_PACK32,			// CF_RGB_HDR,
	VK_FORMAT_R32_SFLOAT,						// CF_R_32F,
	VK_FORMAT_R32_UINT,							// CF_R_32,
	VK_FORMAT_R32G32_SFLOAT,					// CF_RG_32F,
	VK_FORMAT_R16_SFLOAT,						// CF_R_16F,
	VK_FORMAT_R16G16_SFLOAT,					// CF_RG_16F,
	VK_FORMAT_R8_UNORM,							// CF_R_8
	VK_FORMAT_R16G16B16A16_SNORM,				// CF_NORMAL
	VK_FORMAT_R8G8B8A8_UNORM,					// CF_SRGBA
	VK_FORMAT_UNDEFINED,						// CF_UNDEFINED	// Only makes sense for render passes
};

static const VkFormat gDepthFormatViewMap[] =
{
	VK_FORMAT_X8_D24_UNORM_PACK32,		// DF_DEPTH_24,	 // Not technically supported
	VK_FORMAT_D24_UNORM_S8_UINT,		// DF_DEPTH_24_S8,
	VK_FORMAT_D16_UNORM,				// DF_DEPTH_16,
	VK_FORMAT_D32_SFLOAT				// DF_DEPTH_32F,
};

static const VkSampleCountFlagBits gSampleCountMap[] =
{
	VK_SAMPLE_COUNT_1_BIT, // SAMPLE_COUNT_1_BIT = 0,
	VK_SAMPLE_COUNT_2_BIT, // SAMPLE_COUNT_2_BIT,
	VK_SAMPLE_COUNT_4_BIT, // SAMPLE_COUNT_4_BIT,
	VK_SAMPLE_COUNT_8_BIT, // SAMPLE_COUNT_8_BIT,
	VK_SAMPLE_COUNT_16_BIT, // SAMPLE_COUNT_16_BIT,
	VK_SAMPLE_COUNT_32_BIT, // SAMPLE_COUNT_32_BIT,
	VK_SAMPLE_COUNT_64_BIT // SAMPLE_COUNT_64_BIT,
};


#endif
