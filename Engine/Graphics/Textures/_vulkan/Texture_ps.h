/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef _USG_GRAPHICS_PC_TEXTURE_H
#define _USG_GRAPHICS_PC_TEXTURE_H

#include "Engine/Graphics/RenderConsts.h"
#include "Engine/Graphics/Textures/ImageViewDef.h"
#include <vulkan/vulkan.h>
#include API_HEADER(Engine/Graphics/Device, VkMemAllocator.h)


namespace usg {

class Texture;
class Effect;
class GFXDevice;
class GFXContext;
typedef struct _TexturePak TexturePak;

typedef VkImageView ImageViewHndl;

class Texture_ps
{
public:
	Texture_ps();
	~Texture_ps();

	void Init(GFXDevice* pDevice, ColorFormat eFormat, uint32 uWidth, uint32 uHeight, uint32 uMipmaps, void* pPixels = NULL, TextureDimensions eTexDim = TD_TEXTURE2D, uint32 uTextureFlags = TU_FLAG_SHADER_READ);
	void Init(GFXDevice* pDevice, DepthFormat eFormat, uint32 uWidth, uint32 uHeight, uint32 uTextureFlags);
	void InitArray(GFXDevice* pDevice, ColorFormat eFormat, uint32 uWidth, uint32 uHeight, uint32 uSlices);
	void InitArray(GFXDevice* pDevice, DepthFormat eFormat, uint32 uWidth, uint32 uHeight, uint32 uSlices, uint32 uTextureFlags);
	void InitCubeMap(GFXDevice* pDevice, DepthFormat eFormat, uint32 uWidth, uint32 uHeight, uint32 uTextureFlags);
	void Cleanup(GFXDevice* pDevice);
	void Resize(GFXDevice* pDevice, uint32 uWidth, uint32 uHeight);

	bool Load(GFXDevice* pDevice, const char* szFileName, GPULocation eLocation);
	bool Load(GFXDevice* pDevice, const TexturePak& pak, const void* pData) { ASSERT(false); return false; }	// Not yet implemented on PC
	void SetRawData(GFXDevice* pDevice, GFXContext* pCtx, void* pData);

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
	VkImageView CreateImageView(GFXDevice* pDevice, uint32 uLayer, uint32 uMip) const;

	VkImageView GetImageView(GFXDevice* pDevice, const ImageViewDef& def) const;

private:
	void Init(GFXDevice* pDevice, VkImageCreateInfo& createInfo, VkMemoryPropertyFlags flags, bool bInitMemory = true);
	bool LoadWithGLI(GFXDevice* pDevice, const char* szFileName);
	void InitArray(GFXDevice* pDevice, uint32 uWidth, uint32 uHeight, uint32 uArrayCount, VkImageViewType eViewType, VkFormat eFormat, VkImageUsageFlags eUsage);
	void InitStaging(GFXDevice* pDevice);
	void FreeStaging(GFXDevice* pDevice);
	void ClearViews(GFXDevice* pDevice);

	struct TexStaging
	{
		VkMemoryRequirements memReq;
		VkBuffer		buffer;
		VkDeviceMemory	memory;
		bool			bValid = false;
	};

	struct CustomView
	{
		ImageViewDef def;
		VkImageView  view;
	};

	// FIXME: Refactor to pre-load these views
	mutable usg::vector<CustomView>	m_customViews;
	TexStaging		m_staging;
	VkMemAllocator	m_memoryAlloc;
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
	uint32			m_uMips;
	uint32			m_uBpp;
};

}

#endif
