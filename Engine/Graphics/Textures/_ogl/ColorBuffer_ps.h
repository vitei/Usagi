/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef _USG_GRAPHICS_PC_COLORBUFFER_H
#define _USG_GRAPHICS_PC_COLORBUFFER_H
#include "Engine/Common/Common.h"
#include "Engine/Graphics/Textures/Texture.h"
#include "Engine/Resource/ResourceDecl.h"
#include OS_HEADER(Engine/Graphics/Device, OpenGLIncludes.h)

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
	//Texture* GetTexture() { return &m_texture; }
	// Do nothing, no memory management on PC
	void SetActive(bool bActive) { }
	void Resolve(GFXContext* pContext, bool bTex) {}

private:
	TextureHndl	m_texHndl;
	Texture		m_texture;
};

}


#endif
