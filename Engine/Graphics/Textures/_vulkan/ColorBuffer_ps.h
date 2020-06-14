/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef _USG_GRAPHICS_PC_COLORBUFFER_H
#define _USG_GRAPHICS_PC_COLORBUFFER_H

#include "Engine/Graphics/Textures/Texture.h"
#include <vulkan/vulkan.h>

namespace usg {

class ColorBuffer_ps
{
public:
	ColorBuffer_ps();
	~ColorBuffer_ps();

	void Init(GFXDevice* pDevice, uint32 uWidth, uint32 uHeight, ColorFormat eFormat, SampleCount eSamples, uint32 uFlags, uint32 uLoc, uint32 uMipmaps);
	void InitArray(GFXDevice* pDevice, uint32 uBufferId, uint32 uWidth, uint32 uHeight, uint32 uSlices, ColorFormat eFormat, SampleCount eSamples, uint32 uFlags);
	void CleanUp(GFXDevice* pDevice);

	void Resize(GFXDevice* pDevice, uint32 uWidth, uint32 uHeight);

	const TextureHndl& GetTexture() const { return m_texHndl; }

	// Do nothing, no memory management on PC
	void SetActive(bool bActive) { }
	void Resolve(GFXContext* pContext, bool bTex) {}

	VkImageView GetViewEx(uint32 uLayer, uint32 uMip) { return m_pExtraViews[ (uLayer*m_uMips) + uMip ]; }

private:
	void InitExViews(GFXDevice* pDevice);
	void FreeExViews(GFXDevice* pDevice);

	TextureHndl				m_texHndl;
	Texture					m_texture;

	VkImageView*			m_pExtraViews;
	uint32					m_uMips;
};

}


#endif
