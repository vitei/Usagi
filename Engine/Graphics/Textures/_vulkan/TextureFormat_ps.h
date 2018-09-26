/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef _USG_GRAPHICS_VULKAN_TEXTURE_FORMAT_H
#define _USG_GRAPHICS_VULKAN_TEXTURE_FORMAT_H
#include "Engine/Common/Common.h"
#include OS_HEADER(Engine/Graphics/Device, VulkanIncludes.h)


static const VkFormat gDepthFormatViewMap[] =
{
	VK_FORMAT_X8_D24_UNORM_PACK32,		// DF_DEPTH_24,	 // Not technically supported
	VK_FORMAT_D24_UNORM_S8_UINT,		// DF_DEPTH_24_S8,
	VK_FORMAT_D16_UNORM,				// DF_DEPTH_16,
	VK_FORMAT_D32_SFLOAT,				// DF_DEPTH_32F,
	VK_FORMAT_X8_D24_UNORM_PACK32		// DF_DEPTH_32	// There is no 32 unorm
};

static_assert(ARRAY_SIZE(gDepthFormatViewMap) == usg::DF_COUNT, "Depth format count does not match platform independent defines");


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

static_assert(ARRAY_SIZE(gSampleCountMap) == usg::SAMPLE_COUNT_INVALID, "Sample count define size does not match platform independent defines");

#endif
