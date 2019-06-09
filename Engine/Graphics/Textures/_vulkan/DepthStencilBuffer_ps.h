/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef _USG_GRAPHICS_PC_DEPTH_STENCIL_BUFFER_PS_H_
#define _USG_GRAPHICS_PC_DEPTH_STENCIL_BUFFER_PS_H_

#include "Engine/Graphics/Textures/Texture.h"
#include "Engine/Resource/ResourceDecl.h"
#include OS_HEADER(Engine/Graphics/Device, VulkanIncludes.h)

namespace usg {

class DepthStencilBuffer_ps
{
public:
	DepthStencilBuffer_ps();
	~DepthStencilBuffer_ps();

	void Init(GFXDevice* pDevice, uint32 uWidth, uint32 uHeight, DepthFormat eFormat, SampleCount eSamples, uint32 uFlags);
	void InitArray(GFXDevice* pDevice, uint32 uWidth, uint32 uHeight, uint32 uSlices, DepthFormat eFormat, SampleCount eSamples, uint32 uFlags);
	void InitCube(GFXDevice* pDevice, uint32 uWidth, uint32 uHeight, DepthFormat eFormat, SampleCount eSamples, uint32 uFlags);
	void CleanUp(GFXDevice* pDevice);
	
	void Resize(GFXDevice* pDevice, uint32 uWidth, uint32 uHeight);
	uint32 GetWidth() const { return m_uWidth; }
	uint32 GetHeight() const { return m_uHeight; }
	const TextureHndl& GetTexture() const { return m_textureHndl; }
	bool HasStencil() const { return m_bHasStencil; }

	void SetActive(bool bActive) {}

	VkImageView GetLayerView(uint32 uLayer) { return m_pLayerViews[uLayer]; }

private:
	void InitLayerViews(GFXDevice* pDevice);
	void FreeLayerViews(GFXDevice* pDevice);

	TextureHndl				m_textureHndl;
	Texture					m_texture;
	uint32					m_uWidth;
	uint32					m_uHeight;
	bool					m_bHasStencil;
	VkImageView*			m_pLayerViews;
};

}

#endif