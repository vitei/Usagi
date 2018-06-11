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

	void Init(GFXDevice* pDevice, ColorFormat eFormat, uint32 uWidth, uint32 uHeight, uint32 uMipmaps, void* pPixels = NULL, TextureDimensions eTexDim = TD_TEXTURE2D);
	void Init(GFXDevice* pDevice, DepthFormat eFormat, uint32 uWidth, uint32 uHeight);
	void InitArray(GFXDevice* pDevice, ColorFormat eFormat, uint32 uWidth, uint32 uHeight, uint32 uSlices);
	void InitArray(GFXDevice* pDevice, DepthFormat eFormat, uint32 uWidth, uint32 uHeight, uint32 uSlices);
	void InitCubeMap(GFXDevice* pDevice, DepthFormat eFormat, uint32 uWidth, uint32 uHeight);
	void CleanUp(GFXDevice* pDevice);

	bool Load(GFXDevice* pDevice, const char* szFileName, GPULocation eLocation);
	bool Load(GFXDevice* pDevice, const TexturePak& pak, const void* pData) { ASSERT(false); return false; }	// Not yet implemented on PC

	static bool FileExists(const char* szFileName);
	void NotifyOfTextureID(uint32 uTexID) { }

	uint32 GetWidth () const;
	uint32 GetHeight() const;
	uint32 GetDepth() const;
	uint32 GetFaces() const;

#ifdef DEBUG_BUILD
	uint32 GetSizeInMemory() const { return 0; }
#endif

	VkImage GetImage() const { return m_image; }
	VkImageView GetImageView() const { return m_imageView; }
	VkImageLayout GetImageLayout() const { return m_imageLayout; }

private:
	void Init(GFXDevice* pDevice, VkImageCreateInfo& createInfo, VkMemoryPropertyFlags flags);
	bool LoadWithGLI(GFXDevice* pDevice, const char* szFileName);

	VkDeviceMemory	m_memory;
	VkImage			m_image;
	VkImageView		m_imageView;
	VkImageLayout	m_imageLayout;
	uint32			m_uWidth;
	uint32			m_uHeight;
	uint32			m_uDepth;
	uint32			m_uFaces;
};

}

#endif
