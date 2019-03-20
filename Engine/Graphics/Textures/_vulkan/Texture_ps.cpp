/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Core/String/U8String.h"
#include "Engine/Core/File/File.h"
#include "Engine/Graphics/Effects/Effect.h"
#include "Engine/Graphics/Device/GFXDevice.h"
#include API_HEADER(Engine/Graphics/Textures, Sampler.h)
#include API_HEADER(Engine/Graphics/Device, GFXDevice_ps.h)
#include API_HEADER(Engine/Graphics/Textures, TextureFormat_ps.h)
#include API_HEADER(Engine/Graphics/Textures, Texture_ps.h)
#include <vector>
// Using gli for now, should eventually exclusively use ktx
#include "gli/gli.hpp"

namespace usg {

struct KtxHeader
{
	uint8		identifier[12];
	uint32		endianness;
	uint32		glType;
	uint32		glTypeSize;
	uint32		glFormat;
	uint32		glInternalFormat;
	uint32		glBaseInternalFormat;
	uint32		pixelWidth;
	uint32		pixelHeight;
	uint32		pixelDepth;
	uint32		numberOfArrayElements;
	uint32		numberOfFaces;
	uint32		numberOfMipmapLevels;
	uint32		bytesOfKeyValueData;
};


VkFormat GetFormat(uint32 uFormat)
{
	switch (uFormat)
	{
	case 0x8F97:	// GL_RGBA8_SNORM
		return VK_FORMAT_R8G8B8_SNORM;
	case 0x83F0: // GL_COMPRESSED_RGB_S3TC_DXT1_EXT
		return VK_FORMAT_BC1_RGB_UNORM_BLOCK;
	case 0x83F1: // GL_COMPRESSED_RGBA_S3TC_DXT1_EXT
		return VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
	case 0x83F2: // GL_COMPRESSED_RGBA_S3TC_DXT3_EXT
		return VK_FORMAT_BC2_UNORM_BLOCK;
	case 0x83F3: // GL_COMPRESSED_RGBA_S3TC_DXT5_EXT
		return VK_FORMAT_BC3_UNORM_BLOCK;
	default:
		ASSERT(false);	// See what we end up getting passed through
	}

	return VK_FORMAT_R8G8B8_SNORM;
}


VkFormat GetFormatGLI(uint32 uFormat)
{
	switch (uFormat)
	{
	case gli::format::FORMAT_RGB8_SNORM_PACK8:
		return VK_FORMAT_R8G8B8_SNORM;
	case gli::format::FORMAT_RGBA8_SNORM_PACK8:
		return VK_FORMAT_R8G8B8A8_SNORM;
	case gli::format::FORMAT_RGBA_DXT1_UNORM_BLOCK8: // GL_COMPRESSED_RGB_S3TC_DXT1_EXT
		return VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
	case gli::format::FORMAT_RGBA_DXT3_UNORM_BLOCK16: // GL_COMPRESSED_RGBA_S3TC_DXT3_EXT
		return VK_FORMAT_BC2_UNORM_BLOCK;
	case gli::format::FORMAT_RGBA_DXT5_UNORM_BLOCK16:
		return VK_FORMAT_BC3_UNORM_BLOCK;
	case gli::format::FORMAT_BGR8_UNORM_PACK8:
		return VK_FORMAT_B8G8R8_UNORM;
	case gli::format::FORMAT_RG8_UNORM_PACK8:
		return VK_FORMAT_R8G8_UNORM;
	case gli::format::FORMAT_RGBA8_UNORM_PACK8:
		return VK_FORMAT_R8G8B8A8_UNORM;
	case gli::format::FORMAT_L8_UNORM_PACK8:
		return VK_FORMAT_R8_UNORM;
	case gli::format::FORMAT_RGBA_BP_UNORM_BLOCK16:
		return VK_FORMAT_BC7_UNORM_BLOCK;
	case gli::format::FORMAT_BGRA8_UNORM_PACK8:
		return VK_FORMAT_B8G8R8A8_UNORM;
	default:
		ASSERT(false);	// See what we end up getting passed through
	}

	return VK_FORMAT_R8G8B8_SNORM;
}

VkImageUsageFlags GetImageUsage(uint32 uUsage)
{
	VkImageUsageFlags flags = 0;

	if (uUsage&TU_FLAG_TRANSFER_SRC)
	{
		flags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	}
	if (uUsage&TU_FLAG_SHADER_READ)
	{
		flags |= VK_IMAGE_USAGE_SAMPLED_BIT;
	}
	if (uUsage&TU_FLAG_COLOR_ATTACHMENT)
	{
		flags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	}
	if (uUsage&TU_FLAG_DEPTH_ATTACHMENT)
	{
		flags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	}

	// Currently lacking flags mapping to the following:
	// VK_IMAGE_USAGE_TRANSFER_DST_BIT = 0x00000002,
	// VK_IMAGE_USAGE_STORAGE_BIT = 0x00000008,
	// VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT = 0x00000040,
	// VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT = 0x00000080,

	return flags;
}


Texture_ps::Texture_ps() :
	  m_memory(VK_NULL_HANDLE)
	, m_image(VK_NULL_HANDLE)
	, m_imageView(VK_NULL_HANDLE)
	, m_imageLayout(VK_IMAGE_LAYOUT_UNDEFINED)
	, m_uUpdateCount(0)
	, m_uWidth(0)
	, m_uHeight(0)
	, m_uDepth(0)
	, m_uFaces(0)
{

}

Texture_ps::~Texture_ps()
{

}


void Texture_ps::InitArray(GFXDevice* pDevice, uint32 uWidth, uint32 uHeight, uint32 uArrayCount, VkImageViewType eViewType, VkFormat eFormat, VkImageUsageFlags eUsage)
{
	VkImageCreateInfo image_create_info = {};

	VkFormatProperties props;
	if (eUsage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
	{
		vkGetPhysicalDeviceFormatProperties(pDevice->GetPlatform().GetPrimaryGPU(), eFormat, &props);
		if (props.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
		{
			image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
		}
		else
		{
			// Try other depth formats? 
			ASSERT_MSG(false, "Depth format unsupported.\n");
			return;
		}
	}

	image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	image_create_info.pNext = NULL;
	image_create_info.imageType = VK_IMAGE_TYPE_2D;
	image_create_info.format = eFormat;
	image_create_info.extent.width = uWidth;
	image_create_info.extent.height = uHeight;
	image_create_info.extent.depth = 1;
	image_create_info.mipLevels = 1;
	image_create_info.arrayLayers = uArrayCount;
	image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
	image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
	image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	image_create_info.usage = eUsage;
	image_create_info.queueFamilyIndexCount = 0;
	image_create_info.pQueueFamilyIndices = NULL;
	image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	image_create_info.flags = 0;
	m_imageCreateInfo = image_create_info;

	Init(pDevice, image_create_info, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	VkImageViewCreateInfo view_info = {};
	view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	view_info.pNext = NULL;
	view_info.image = m_image;
	view_info.format = image_create_info.format;
	view_info.subresourceRange.aspectMask = eUsage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
	view_info.subresourceRange.baseMipLevel = 0;
	view_info.subresourceRange.levelCount = 1;
	view_info.subresourceRange.baseArrayLayer = 0;
	view_info.subresourceRange.layerCount = uArrayCount;
	view_info.viewType = eViewType;
	view_info.flags = 0;
	m_imageViewCreateInfo = view_info;

	// Create the image view
	VkResult res = vkCreateImageView(pDevice->GetPlatform().GetVKDevice(), &view_info, NULL, &m_imageView);
	ASSERT(res == VK_SUCCESS);

	m_imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	m_uWidth = uWidth;
	m_uHeight = uHeight;
	m_uDepth = 1;
	m_uFaces = uArrayCount;
	m_uUpdateCount++;
}

VkImageView Texture_ps::CreateLayerImageView(GFXDevice* pDevice, uint32 uLayer) const
{
	ASSERT(m_uFaces > uLayer);
	VkImageViewCreateInfo view_info = m_imageViewCreateInfo;
	view_info.subresourceRange.baseArrayLayer = uLayer;
	view_info.subresourceRange.layerCount = 1;
	view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;

	VkImageView view;
	VkResult res = vkCreateImageView(pDevice->GetPlatform().GetVKDevice(), &view_info, NULL, &view);
	ASSERT(res == VK_SUCCESS);

	return view;
}

void Texture_ps::InitArray(GFXDevice* pDevice, ColorFormat eFormat, uint32 uWidth, uint32 uHeight, uint32 uSlices)
{
	VkImageUsageFlags imageUsage = GetImageUsage(TU_FLAG_COLOR_ATTACHMENT| TU_FLAG_SHADER_READ);
	InitArray(pDevice, uWidth, uHeight, uSlices, VK_IMAGE_VIEW_TYPE_2D_ARRAY, pDevice->GetPlatform().GetColorFormat(eFormat), imageUsage);
}

void Texture_ps::InitArray(GFXDevice* pDevice, DepthFormat eFormat, uint32 uWidth, uint32 uHeight, uint32 uSlices)
{
	VkImageUsageFlags imageUsage = GetImageUsage(TU_FLAG_DEPTH_ATTACHMENT | TU_FLAG_SHADER_READ);
	InitArray(pDevice, uWidth, uHeight, uSlices, VK_IMAGE_VIEW_TYPE_2D_ARRAY, gDepthFormatViewMap[eFormat], imageUsage);
}

void Texture_ps::InitCubeMap(GFXDevice* pDevice, DepthFormat eFormat, uint32 uWidth, uint32 uHeight)
{
	VkImageUsageFlags imageUsage = GetImageUsage(TU_FLAG_DEPTH_ATTACHMENT | TU_FLAG_SHADER_READ);
	InitArray(pDevice, uWidth, uHeight, 6, VK_IMAGE_VIEW_TYPE_CUBE, gDepthFormatViewMap[eFormat], imageUsage);
}


void Texture_ps::Init(GFXDevice* pDevice, ColorFormat eFormat, uint32 uWidth, uint32 uHeight, uint32 uMipmaps, void* pPixels, TextureDimensions eTexDim, uint32 uTextureFlags)
{
	ASSERT(uTextureFlags & TU_FLAG_COLOR_ATTACHMENT);
    VkImageCreateInfo image_create_info = {};
    image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_create_info.pNext = NULL;
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = pDevice->GetPlatform().GetColorFormat(eFormat);

    image_create_info.extent.width = uWidth;
    image_create_info.extent.height = uHeight;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = uMipmaps;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_create_info.usage = GetImageUsage(uTextureFlags);
    image_create_info.queueFamilyIndexCount = 0;
    image_create_info.pQueueFamilyIndices = NULL;
    image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    image_create_info.flags = 0;
	m_imageCreateInfo = image_create_info;

    Init(pDevice, image_create_info, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	VkImageViewCreateInfo view_info = {};
	view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	view_info.pNext = NULL;
	view_info.image = m_image;
	view_info.format = image_create_info.format;
	view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	view_info.subresourceRange.baseMipLevel = 0;
	view_info.subresourceRange.levelCount = 1;
	view_info.subresourceRange.baseArrayLayer = 0;
	view_info.subresourceRange.layerCount = 1;
	view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
	view_info.flags = 0;
	m_imageViewCreateInfo = view_info;

	// Create the image view
	VkResult res = vkCreateImageView(pDevice->GetPlatform().GetVKDevice(), &view_info, NULL, &m_imageView);
	ASSERT(res == VK_SUCCESS);

	m_imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	m_uWidth = uWidth;
	m_uHeight = uHeight;
	m_uDepth = 1;
	m_uFaces = 1;
}


void Texture_ps::Init(GFXDevice* pDevice, DepthFormat eFormat, uint32 uWidth, uint32 uHeight, uint32 uTextureFlags)
{
	ASSERT(uTextureFlags & TU_FLAG_DEPTH_ATTACHMENT);
	// Check for support
	const VkFormat depth_format = gDepthFormatViewMap[eFormat];

    VkFormatProperties props;
	VkImageCreateInfo image_create_info = {};
    vkGetPhysicalDeviceFormatProperties(pDevice->GetPlatform().GetPrimaryGPU(), depth_format, &props);
    if (props.linearTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
    {
		image_create_info.tiling = VK_IMAGE_TILING_LINEAR;
    }
    else if (props.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
    {
		image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    }
    else
    {
        // Try other depth formats? 
        ASSERT_MSG(false, "Depth format unsupported.\n");
        return;
    }

	image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	image_create_info.pNext = NULL;
	image_create_info.imageType = VK_IMAGE_TYPE_2D;
	image_create_info.format = depth_format;
	image_create_info.extent.width = uWidth;
	image_create_info.extent.height = uHeight;
	image_create_info.extent.depth = 1;
	image_create_info.mipLevels = 1;
	image_create_info.arrayLayers = 1;
	image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
	image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
	image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	image_create_info.usage = GetImageUsage(uTextureFlags);
	image_create_info.queueFamilyIndexCount = 0;
	image_create_info.pQueueFamilyIndices = NULL;
	image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	image_create_info.flags = 0;
	m_imageCreateInfo = image_create_info;

    Init(pDevice, image_create_info, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	VkImageViewCreateInfo view_info = {};
	view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	view_info.pNext = NULL;
	view_info.image = m_image;
	view_info.format = image_create_info.format;
	view_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	view_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	view_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	view_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	view_info.subresourceRange.baseMipLevel = 0;
	view_info.subresourceRange.levelCount = 1;
	view_info.subresourceRange.baseArrayLayer = 0;
	view_info.subresourceRange.layerCount = 1;
	view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
	view_info.flags = 0;
	m_imageViewCreateInfo = view_info;

	// Create the image view
	VkResult res = vkCreateImageView(pDevice->GetPlatform().GetVKDevice(), &view_info, NULL, &m_imageView);
	ASSERT(res == VK_SUCCESS);

	m_imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	m_uWidth = uWidth;
	m_uHeight = uHeight;
	m_uDepth = 1;
	m_uFaces = 1;

}


void Texture_ps::Resize(GFXDevice* pDevice, uint32 uWidth, uint32 uHeight)
{
	m_imageCreateInfo.extent.width = uWidth;
	m_imageCreateInfo.extent.height = uHeight;
	Init(pDevice, m_imageCreateInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, uWidth > m_uWidth || uHeight > m_uHeight);


	// Create the image view
	m_imageViewCreateInfo.image = m_image;
	VkResult res = vkCreateImageView(pDevice->GetPlatform().GetVKDevice(), &m_imageViewCreateInfo, NULL, &m_imageView);
	ASSERT(res == VK_SUCCESS);

	m_uWidth = uWidth;
	m_uHeight = uHeight;
	m_uUpdateCount++;
}


void Texture_ps::Init(GFXDevice* pDevice, VkImageCreateInfo& createInfo, VkMemoryPropertyFlags flags, bool bInitMemory)
{
	VkMemoryAllocateInfo mem_alloc = {};

	VkDevice vKDevice = pDevice->GetPlatform().GetVKDevice();

    // Create the image
	if (m_image)
	{
		vkDestroyImage(vKDevice, m_image, nullptr);
		m_image = VK_NULL_HANDLE;
	}
    VkResult err = vkCreateImage(vKDevice, &createInfo, NULL, &m_image);
    ASSERT(!err);

	VkMemoryRequirements mem_reqs;
    vkGetImageMemoryRequirements(vKDevice, m_image, &mem_reqs);
    ASSERT(!err);

    mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    mem_alloc.pNext = NULL;
    mem_alloc.allocationSize = mem_reqs.size;
    mem_alloc.memoryTypeIndex = pDevice->GetPlatform().GetMemoryTypeIndex(mem_reqs.memoryTypeBits, flags);

	if (bInitMemory)
	{
		if (m_memory)
		{
			vkFreeMemory(vKDevice, m_memory, nullptr);
			m_memory = nullptr;
		}

		// Allocate memory
		err = vkAllocateMemory(vKDevice, &mem_alloc, NULL,
			&m_memory);
		ASSERT(!err);
	}

    // Bind memory
    err = vkBindImageMemory(vKDevice, m_image, m_memory, 0);
    ASSERT(!err);
	m_uUpdateCount++;

}


void Texture_ps::CleanUp(GFXDevice* pDevice)
{
	VkDevice vKDevice = pDevice->GetPlatform().GetVKDevice();
	if (m_image)
	{
		vkDestroyImage(vKDevice, m_image, nullptr);
		m_image = VK_NULL_HANDLE;
	}

	if (m_memory != VK_NULL_HANDLE)
	{
		vkFreeMemory(vKDevice, m_memory, nullptr);
		vkDestroyImage(vKDevice, m_image, nullptr);
		m_memory = VK_NULL_HANDLE;
	}
}


bool Texture_ps::Load(GFXDevice* pDevice, const char* szFileName, GPULocation eLocation)
{
	U8String filename = szFileName;

	U8String tmp = filename + ".ktx";

	if (File::FileStatus(tmp.CStr()) == FILE_STATUS_VALID)
	{
		return LoadWithGLI(pDevice, tmp.CStr());
	}
	else
	{
		tmp = filename + ".dds";
		return LoadWithGLI(pDevice, tmp.CStr());
	}

	m_uUpdateCount++;

	return false;
}


bool Texture_ps::LoadWithGLI(GFXDevice* pDevice, const char* szFileName)
{
	bool bReturn = true;
	{
		VkResult res;
		mem::setConventionalMemManagement(true);
		usg::File texFile(szFileName);
		void* scratchMemory = NULL;
		ScratchRaw::Init(&scratchMemory, texFile.GetSize(), 4);
		texFile.Read(texFile.GetSize(), scratchMemory);
		gli::texture Texture = gli::load((char*)scratchMemory, texFile.GetSize());

		VkFormatProperties formatProperties;
		VkFormat eFormatVK = GetFormatGLI(Texture.format());

		glm::tvec3<uint32> const Extent(Texture.extent());
		uint32 const FaceTotal = static_cast<uint32>(Texture.layers() * Texture.faces());
		m_uWidth = Extent.x;
		m_uHeight = Extent.y;
		m_uDepth = Extent.z;
		m_uFaces = FaceTotal;

		vkGetPhysicalDeviceFormatProperties(pDevice->GetPlatform().GetPrimaryGPU(), eFormatVK, &formatProperties);
		GFXDevice_ps& devicePS = pDevice->GetPlatform();
		VkDevice device = devicePS.GetVKDevice();

		VkMemoryAllocateInfo memAllocInfo = {};
		memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		VkMemoryRequirements memReqs = {};

		VkImageType eVKImageType = VK_IMAGE_TYPE_2D;
		VkImageViewType eVKImageViewType = VK_IMAGE_VIEW_TYPE_2D;
		if (m_uHeight > 1)
		{
			if (m_uDepth > 1)
			{
				eVKImageType = VK_IMAGE_TYPE_3D;
				eVKImageViewType = VK_IMAGE_VIEW_TYPE_3D;
			}
			else if (m_uFaces > 1)
			{
				eVKImageType = VK_IMAGE_TYPE_2D;
				eVKImageViewType = VK_IMAGE_VIEW_TYPE_CUBE;
			}
			else
			{
				eVKImageType = VK_IMAGE_TYPE_2D;
				eVKImageViewType = VK_IMAGE_VIEW_TYPE_2D;
			}
		}
		else
		{
			eVKImageType = VK_IMAGE_TYPE_1D;
			eVKImageViewType = VK_IMAGE_VIEW_TYPE_1D;
		}

		// Create a staging area that contains the raw image data
		VkBuffer stagingBuffer;
		VkDeviceMemory stagingMemory;

		VkBufferCreateInfo bufferCreateInfo = {};
		bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferCreateInfo.size = Texture.size();
		bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		res = vkCreateBuffer(device, &bufferCreateInfo, nullptr, &stagingBuffer);

		vkGetBufferMemoryRequirements(device, stagingBuffer, &memReqs);

		memAllocInfo.allocationSize = memReqs.size;
		memAllocInfo.memoryTypeIndex = devicePS.GetMemoryTypeIndex(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		res = vkAllocateMemory(device, &memAllocInfo, nullptr, &stagingMemory);
		res = vkBindBufferMemory(device, stagingBuffer, stagingMemory, 0);

		// Copy texture data into staging buffer
		uint8_t *data;
		res = vkMapMemory(device, stagingMemory, 0, memReqs.size, 0, (void **)&data);
		memcpy(data, Texture.data(), Texture.size());
		vkUnmapMemory(device, stagingMemory);

		// Regions of the buffer to copy for each mip level
		std::vector<VkBufferImageCopy> bufferCopyRegions;
		uint32_t offset = 0;

		for (uint32_t uFace = 0; uFace < m_uFaces; uFace++)
		{
			for (uint32_t uLevel = 0; uLevel < Texture.levels(); uLevel++)
			{
				VkBufferImageCopy bufferCopyRegion = {};
				bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				bufferCopyRegion.imageSubresource.mipLevel = uLevel;
				bufferCopyRegion.imageSubresource.baseArrayLayer = uFace;
				bufferCopyRegion.imageSubresource.layerCount = 1;
				glm::tvec3<uint32> LevelExtent(Texture.extent(uLevel));
				bufferCopyRegion.imageExtent.width = static_cast<uint32_t>(LevelExtent.x);
				bufferCopyRegion.imageExtent.height = static_cast<uint32_t>(LevelExtent.y);
				bufferCopyRegion.imageExtent.depth = static_cast<uint32_t>(LevelExtent.z);
				bufferCopyRegion.bufferOffset = offset;

				bufferCopyRegions.push_back(bufferCopyRegion);

				offset += static_cast<uint32_t>(Texture.size(uLevel));
			}
		}

		// Set the target texture as optimal tiled
		VkImageCreateInfo imageCreateInfo = {};
		imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageCreateInfo.imageType = eVKImageType;
		imageCreateInfo.format = eFormatVK;
		imageCreateInfo.mipLevels = (uint32)Texture.levels();
		imageCreateInfo.arrayLayers = m_uFaces;
		imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageCreateInfo.extent = { m_uWidth, m_uHeight, m_uDepth };
		imageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		imageCreateInfo.flags = eVKImageViewType == VK_IMAGE_VIEW_TYPE_CUBE ? VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT : 0;

		res = vkCreateImage(device, &imageCreateInfo, nullptr, &m_image);

		vkGetImageMemoryRequirements(device, m_image, &memReqs);

		memAllocInfo.allocationSize = memReqs.size;
		memAllocInfo.memoryTypeIndex = devicePS.GetMemoryTypeIndex(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		res = vkAllocateMemory(device, &memAllocInfo, nullptr, &m_memory);
		res = vkBindImageMemory(device, m_image, m_memory, 0);

		VkCommandBuffer copyCmd = devicePS.CreateCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

		// Image barrier for optimal image
		VkImageSubresourceRange subresourceRange = {};
		subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subresourceRange.baseMipLevel = 0;
		subresourceRange.levelCount = (uint32)Texture.levels();
		subresourceRange.layerCount = m_uFaces;

		// Set the image layout to transfer destination before copying the image
		SetImageLayout(copyCmd,	m_image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, subresourceRange);

		// Copy the image
		vkCmdCopyBufferToImage(copyCmd, stagingBuffer, m_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, static_cast<uint32_t>(bufferCopyRegions.size()), bufferCopyRegions.data());

		// All mip levels have been copied so change to read only
		m_imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		SetImageLayout(copyCmd,	m_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, m_imageLayout, subresourceRange);

		devicePS.FlushCommandBuffer(copyCmd, true);

		// Clean up staging resources
		vkFreeMemory(device, stagingMemory, nullptr);
		vkDestroyBuffer(device, stagingBuffer, nullptr);

		// The image view
		VkImageViewCreateInfo view = {};
		view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		view.viewType = eVKImageViewType;
		view.format = eFormatVK;
		view.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };

		view.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		view.subresourceRange.baseMipLevel = 0;
		view.subresourceRange.baseArrayLayer = 0;
		view.subresourceRange.layerCount = m_uFaces;
		view.subresourceRange.levelCount = (uint32)Texture.levels();
		view.image = m_image;
		res = vkCreateImageView(device, &view, nullptr, &m_imageView);

	}
	mem::setConventionalMemManagement(false);
	return bReturn;
}


// Standard ktx loading below this point (should switch back to this as gli has a pretty slow memcopy)
#if 0
bool Texture_ps::Load(GFXDevice* pDevice, const char* szFileName, GPULocation eLocation)
{
	U8String tmp = szFileName;
	tmp += ".ktx";
	File file;

	VkDevice vkDevice = pDevice->GetPlatform().GetVKDevice();
	file.Open(tmp.CStr());

	ScratchRaw rawData;
	rawData.Init(file.GetSize(), FILE_READ_ALIGN);
	
	KtxHeader* pHeader = (KtxHeader*)rawData.GetRawData();

	m_uWidth = pHeader->pixelWidth;
	m_uHeight = pHeader->pixelHeight;
	m_uDepth = pHeader->pixelDepth;
	m_uFaces = pHeader->numberOfFaces;

	VkImageCreateInfo image_create_info = {};
	VkImageViewCreateInfo view_info = {};
	// FIXME: Cubemaps
	if (m_uHeight > 1)
	{
		if (m_uDepth > 1)
		{
			image_create_info.imageType = VK_IMAGE_TYPE_3D;
			view_info.viewType = VK_IMAGE_VIEW_TYPE_3D;
		}
		else if (m_uFaces > 1)
		{
			image_create_info.imageType = VK_IMAGE_TYPE_2D;
			view_info.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
		}
		else
		{
			image_create_info.imageType = VK_IMAGE_TYPE_2D;
			view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
		}
	}
	else
	{
		image_create_info.imageType = VK_IMAGE_TYPE_1D;
		view_info.viewType = VK_IMAGE_VIEW_TYPE_1D;
	}
	image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	image_create_info.pNext = NULL;
	image_create_info.format = GetFormat(pHeader->glInternalFormat);
	image_create_info.extent.width = m_uWidth;
	image_create_info.extent.height = m_uHeight;
	image_create_info.extent.depth = m_uDepth;
	image_create_info.mipLevels = pHeader->numberOfMipmapLevels;
	image_create_info.arrayLayers = pHeader->numberOfFaces;
	image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
	image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
	image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	image_create_info.queueFamilyIndexCount = 0;
	image_create_info.pQueueFamilyIndices = NULL;
	image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	image_create_info.flags = 0;

	Init(pDevice, image_create_info);

	uint8* pDataBase = ((uint8*)rawData.GetRawData()) + pHeader->bytesOfKeyValueData;
	uint8* pImageData = pDataBase;

	std::vector<VkBufferImageCopy> bufferCopyRegions;
	uint32 offset = 0;

	VkMemoryAllocateInfo memAllocInfo = {};
	memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	VkMemoryRequirements memReqs = {};

	bool bUseStaging = true;

	if (bUseStaging)
	{
		VkBuffer stagingBuffer;
		VkDeviceMemory stagingMemory;

		for (uint32 i = 0; i < pHeader->numberOfMipmapLevels; i++)
		{
			uint32 uImageSize = *(uint32*)pImageData;

			VkBufferImageCopy bufferCopyRegion = {};
			bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			bufferCopyRegion.imageSubresource.mipLevel = i;
			bufferCopyRegion.imageSubresource.baseArrayLayer = 0;
			bufferCopyRegion.imageSubresource.layerCount = pHeader->numberOfArrayElements;
			bufferCopyRegion.imageExtent.width = pHeader->pixelWidth;
			bufferCopyRegion.imageExtent.height = pHeader->pixelHeight;
			bufferCopyRegion.imageExtent.depth = pHeader->pixelDepth;
			bufferCopyRegion.bufferOffset = offset;

			bufferCopyRegions.push_back(bufferCopyRegion);

			offset += static_cast<uint32_t>(uImageSize);
			// Mip padding
			offset += 3 - ((uImageSize + 3) % 4);
		}

		VkBufferCreateInfo bufCreateInfo = {};
		bufCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufCreateInfo.size = offset;
		// This buffer is used as a transfer source for the buffer copy
		bufCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		bufCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VkResult res = vkCreateBuffer(vkDevice, &bufCreateInfo, nullptr, &stagingBuffer);

		// Get memory requirements for the staging buffer (alignment, memory type bits)
		vkGetBufferMemoryRequirements(vkDevice, stagingBuffer, &memReqs);

		memAllocInfo.allocationSize = memReqs.size;
		// Get memory type index for a host visible buffer
		memAllocInfo.memoryTypeIndex = pDevice->GetPlatform().GetMemoryTypeIndex(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

		res = vkAllocateMemory(vkDevice, &memAllocInfo, nullptr, &stagingMemory);
		ASSERT(res == VK_SUCCESS);
		res = vkBindBufferMemory(vkDevice, stagingBuffer, stagingMemory, 0);
		ASSERT(res == VK_SUCCESS);

		// Copy texture data into staging buffer
		uint8_t *data;
		res = vkMapMemory(vkDevice, stagingMemory, 0, memReqs.size, 0, (void **)&data);
		ASSERT(res == VK_SUCCESS);
		MemCpy(data, pImageData + sizeof(uint32), offset);
		vkUnmapMemory(vkDevice, stagingMemory);


		VkCommandBuffer copyCmd = pDevice->GetPlatform().CreateCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

		// Image barrier
		VkImageSubresourceRange subresourceRange = {};
		subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subresourceRange.baseMipLevel = 0;
		subresourceRange.levelCount = pHeader->numberOfMipmapLevels;
		subresourceRange.layerCount = 1;

		SetImageLayout(copyCmd, m_image, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, subresourceRange);

		// Copy mip levels from staging buffer
		vkCmdCopyBufferToImage(copyCmd, stagingBuffer, m_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,	static_cast<uint32_t>(bufferCopyRegions.size()), bufferCopyRegions.data());

		// Change texture image layout to shader read after all mip levels have been copied
		m_imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		SetImageLayout(copyCmd, m_image, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, m_imageLayout, subresourceRange);

		pDevice->GetPlatform().FlushCommandBuffer(copyCmd, true);

		// Clean up staging resources
		vkFreeMemory(vkDevice, stagingMemory, nullptr);
		vkDestroyBuffer(vkDevice, stagingBuffer, nullptr);
	}
	else
	{
		ASSERT(false);
	}

	view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	view_info.image = VK_NULL_HANDLE;
	view_info.format = image_create_info.format;
	// TODO: Using VK_COMPONENT_SWIZZLE_ZERO or VK_COMPONENT_SWIZZLE_ONE we can control the value of color channels not present in the texture
	view_info.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
	view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	view_info.subresourceRange.baseMipLevel = 0;
	view_info.subresourceRange.baseArrayLayer = 0;
	view_info.subresourceRange.layerCount = 1;
	view_info.subresourceRange.levelCount = pHeader->numberOfMipmapLevels;
	view_info.image = m_image;
	VkResult res = vkCreateImageView(pDevice->GetPlatform().GetVKDevice(), &view_info, nullptr, &m_imageView);
	ASSERT(res == VK_SUCCESS);

	return true;
}
#endif

uint32 Texture_ps::GetWidth() const
{
	return m_uWidth;
}

uint32 Texture_ps::GetHeight() const
{
	return m_uHeight;
}

uint32 Texture_ps::GetDepth() const
{
	return m_uDepth;
}

uint32 Texture_ps::GetFaces() const
{
	return m_uFaces;
}

bool Texture_ps::FileExists(const char* szFileName)
{
	U8String file(szFileName);
	file += ".dds";
	if (File::FileStatus(file.CStr()) == FILE_STATUS_VALID)
		return true;

	file = szFileName;
	file += ".ktx";
	return File::FileStatus(file.CStr()) == FILE_STATUS_VALID;
}

}
