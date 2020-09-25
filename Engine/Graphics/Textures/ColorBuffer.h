/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef _USG_GRAPHICS_COLORBUFFER_H
#define _USG_GRAPHICS_COLORBUFFER_H

#include "Engine/Graphics/Textures/Texture.h"
#include "Engine/Graphics/Viewports/Viewport.h"
#include "Engine/Resource/ResourceDecl.h"
#include API_HEADER(Engine/Graphics/Textures, ColorBuffer_ps.h)

namespace usg {

class ColorBuffer
{
public:
	ColorBuffer();
	~ColorBuffer();

	enum Type
	{
		TYPE_2D = 0,
		TYPE_CUBE,
		TYPE_INVALID,

		TYPE_FORCE_ENUM_SIZE = 0xFFFFFFFF
	};

	void Init(GFXDevice* pDevice, uint32 uWidth, uint32 uHeight, ColorFormat eFormat, SampleCount eSampleCount = SAMPLE_COUNT_1_BIT, uint32 uFlags = TU_FLAGS_OFFSCREEN_COLOR, uint32 uRTLoc = 0, uint32 uMipCount=1);
	void InitCube(GFXDevice* pDevice, uint32 uWidth, uint32 uHeight, uint32 uSlices, ColorFormat eFormat, SampleCount eSampleCount = SAMPLE_COUNT_1_BIT, uint32 uFlags = TU_FLAGS_OFFSCREEN_COLOR);
	void Cleanup(GFXDevice* pDevice);

	void			Resize(GFXDevice* pDevice, uint32 uWidth, uint32 uHeight);

	uint32			GetWidth() const	{ return m_uWidth; }
	uint32			GetHeight() const	{ return m_uHeight; }

	ColorBuffer_ps &GetPlatform()		{ return m_platform; }
	const ColorBuffer_ps &GetPlatform() const { return m_platform; }
	const TextureHndl&	GetTexture() const { return m_platform.GetTexture(); }
	Type			GetType() const { return m_eType; };
	void			ResolveTexture(GFXContext* pContext) { m_platform.Resolve(pContext, true); }

	uint32			GetFlags() { return m_uFlags; }
	uint32			GetRTLoc() const { return m_uRTLoc; }
	uint32			GetMipCount() const { return m_uMipmaps; }
	uint32			GetSlices() const { return m_uSlices; }
	bool			IsValid() const { return m_eType != TYPE_INVALID; }

	ColorFormat		GetFormat() const { return m_eFormat; }

	SampleCount		GetSampleCount() const { return m_eSampleCount; }
	// Should only be called by the render target
	void			IncRef();
	void			DecRef();

private:
	ColorBuffer_ps	m_platform;
	ColorFormat		m_eFormat;
	SampleCount		m_eSampleCount;
	Type			m_eType;
	uint32			m_uWidth;
	uint32			m_uHeight;
	uint32			m_uRefCount;
	uint32			m_uFlags;
	uint32			m_uRTLoc;
	uint32			m_uMipmaps;
	uint32			m_uSlices;
};


}


#endif
