/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef _USG_GRAPHICS_PC_TEXTURE_H
#define _USG_GRAPHICS_PC_TEXTURE_H
#include "Engine/Common/Common.h"
#include "Engine/Graphics/RenderConsts.h"
#include <vulkan/vulkan.h>

namespace usg {

class Texture;
class Effect;
class GFXDevice;
typedef struct _TexturePak TexturePak;

class Texture_ps
{
public:
	Texture_ps();
	~Texture_ps();

	void Init(GFXDevice* pDevice, ColorFormat eFormat, uint32 uWidth, uint32 uHeight, uint32 uMipmaps, void* pPixels = NULL, TextureDimensions eTexDim = TD_TEXTURE2D, uint32 uTextureFlags = TU_FLAG_SHADER_READ);
	void Init(GFXDevice* pDevice, DepthFormat eFormat, uint32 uWidth, uint32 uHeight, uint32 uTextureFlags);
	void InitArray(GFXDevice* pDevice, ColorFormat eFormat, uint32 uWidth, uint32 uHeight, uint32 uSlices);
	void InitArray(GFXDevice* pDevice, DepthFormat eFormat, uint32 uWidth, uint32 uHeight, uint32 uSlices);
	void InitCubeMap(GFXDevice* pDevice, DepthFormat eFormat, uint32 uWidth, uint32 uHeight);
	void CleanUp(GFXDevice* pDevice);
	void Resize(GFXDevice* pDevice, uint32 uWidth, uint32 uHeight);

	bool Load(GFXDevice* pDevice, const char* szFileName, GPULocation eLocation);
	bool Load(GFXDevice* pDevice, const TexturePak& pak, const void* pData) { ASSERT(false); return false; }	// Not yet implemented on PC

	static bool FileExists(const char* szFileName);
	void NotifyOfTextureID(uint32 uTexID) { }

	uint32 GetWidth () const;
	uint32 GetHeight() const;
	uint32 GetDepth() const;
	uint32 GetFaces() const;
	uint32 GetUpdateIdx() const { return m_uUpdateCount; }

#ifdef DEBUG_BUILD
	uint32 GetSizeInMemory() const { return 0; }
#endif

	VkImage GetImage() const { return m_image; }
	VkImageView GetImageView() const { return m_imageView; }
	VkImageLayout GetImageLayout() const { return m_imageLayout; }
	// Caller is responsible for cleaning up this view
	VkImageView CreateLayerImageView(GFXDevice* pDevice, uint32 uLayer) const;

private:
	void Init(GFXDevice* pDevice, VkImageCreateInfo& createInfo, VkMemoryPropertyFlags flags, bool bInitMemory = true);
	bool LoadWithGLI(GFXDevice* pDevice, const char* szFileName);
	void InitArray(GFXDevice* pDevice, uint32 uWidth, uint32 uHeight, uint32 uArrayCount, VkImageViewType eViewType, VkFormat eFormat, VkImageUsageFlags eUsage);

	VkDeviceMemory	m_memory;
	VkImage			m_image;
	VkImageView		m_imageView;
	VkImageLayout	m_imageLayout;
	VkImageCreateInfo m_imageCreateInfo;
	VkImageViewCreateInfo m_imageViewCreateInfo;
	uint32			m_uUpdateCount;
	uint32			m_uWidth;
	uint32			m_uHeight;
	uint32			m_uDepth;
	uint32			m_uFaces;
};

}

#endif
