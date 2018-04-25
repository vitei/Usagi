/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef _USG_GRAPHICS_PC_DEPTH_STENCIL_BUFFER_PS_H_
#define _USG_GRAPHICS_PC_DEPTH_STENCIL_BUFFER_PS_H_
#include "Engine/Common/Common.h"
#include "Engine/Graphics/Textures/Texture.h"
#include OS_HEADER(Engine/Graphics/Device, VulkanIncludes.h)

namespace usg {

class DepthStencilBuffer_ps
{
public:
	DepthStencilBuffer_ps();
	~DepthStencilBuffer_ps();

	void Init(GFXDevice* pDevice, uint32 uWidth, uint32 uHeight, DepthFormat eFormat, SampleCount eSamples, uint32 uFlags);
	void InitArray(GFXDevice* pDevice, uint32 uWidth, uint32 uHeight, uint32 uSlices, DepthFormat eFormat, SampleCount eSamples, uint32 uFlags);
	
	void Resize(GFXDevice* pDevice, uint32 uWidth, uint32 uHeight);
	uint32 GetWidth() const { return m_uWidth; }
	uint32 GetHeight() const { return m_uHeight; }
	const Texture* GetTexture() const { return &m_texture; }
	bool HasStencil() const { return m_bHasStencil; }

	void SetActive(bool bActive) {}

	const VkAttachmentDescription& GetDescription() { return m_attachDesc; }

private:
	VkAttachmentDescription m_attachDesc;
	Texture					m_texture;
	uint32					m_uWidth;
	uint32					m_uHeight;
	bool					m_bHasStencil;
};

}

#endif