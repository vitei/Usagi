/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef _USG_GRAPHICS_DEPTH_STENCIL_BUFFER_H
#define _USG_GRAPHICS_DEPTH_STENCIL_BUFFER_H
#include "Engine/Common/Common.h"
#include "Engine/Graphics/Textures/Texture.h"
#include "Engine/Resource/ResourceDecl.h"
#include API_HEADER(Engine/Graphics/Textures, DepthStencilBuffer_ps.h)

namespace usg {

class DepthStencilBuffer
{
public:

	DepthStencilBuffer();
	~DepthStencilBuffer();

	void Init(GFXDevice* pDevice, uint32 uWidth, uint32 uHeight, DepthFormat eFormat, SampleCount eSamples = SAMPLE_COUNT_1_BIT, uint32 uFlags = TU_FLAGS_DEPTH_BUFFER);
	void InitArray(GFXDevice* pDevice, uint32 uWidth, uint32 uHeight, uint32 uSlices, DepthFormat eFormat, SampleCount eSamples = SAMPLE_COUNT_1_BIT, uint32 uFlags = TU_FLAGS_DEPTH_BUFFER);
	void InitCube(GFXDevice* pDevice, uint32 uWidth, uint32 uHeight, DepthFormat eFormat, SampleCount eSamples = SAMPLE_COUNT_1_BIT, uint32 uFlags = TU_FLAGS_DEPTH_BUFFER);
	void CleanUp(GFXDevice* pDevice);

	void Resize(GFXDevice* pDevice, uint32 uWidth, uint32 uHeight);

	uint32 GetWidth() const { return m_platform.GetWidth(); }
	uint32 GetHeight() const { return m_platform.GetHeight(); }
	const TextureHndl& GetTexture() const { return m_platform.GetTexture(); }
	bool HasStencil() const { return m_platform.HasStencil(); }
	DepthStencilBuffer_ps& GetPlatform() { return m_platform; }
	const DepthStencilBuffer_ps& GetPlatform() const { return m_platform; }
	uint32 GetFlags() { return m_uFlags; }
	uint32 GetSlices() const { return m_uSlices; }
	SampleCount GetSampleCount() const { return m_eSamples; }
	DepthFormat GetFormat() const { return m_eFormat; }

	// Should only be called by the render target
	void IncRef();
	void DecRef();

private:
	DepthStencilBuffer_ps	m_platform;
	DepthFormat				m_eFormat;
	SampleCount				m_eSamples;
	uint32					m_uRefCount;
	uint32					m_uFlags;
	uint32					m_uSlices;
};


}

#endif
