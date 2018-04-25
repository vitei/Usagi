/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "DepthStencilBuffer_ps.h"

namespace usg {

DepthStencilBuffer_ps::DepthStencilBuffer_ps()
{
	m_uWidth = 0;
	m_uHeight = 0;
	m_textureHndl = &m_texture;
}

DepthStencilBuffer_ps::~DepthStencilBuffer_ps()
{
	m_textureHndl.reset(NULL);
}


void DepthStencilBuffer_ps::Init(GFXDevice* pDevice, uint32 uWidth, uint32 uHeight, DepthFormat eFormat, SampleCount eSamples, uint32 uFlags)
{
	m_uWidth = uWidth;
	m_uHeight = uHeight;

	m_texture.GetPlatform().Init(pDevice, eFormat, uWidth, uHeight);
	m_texture.GetPlatform().BindInt(0);

	m_bHasStencil = (eFormat == DF_DEPTH_24_S8);

//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
}


void DepthStencilBuffer_ps::InitArray(GFXDevice* pDevice, uint32 uWidth, uint32 uHeight, uint32 uSlices, DepthFormat eFormat, SampleCount eSamples, uint32 uFlags)
{
	m_uWidth = uWidth;
	m_uHeight = uHeight;
	
	m_texture.GetPlatform().InitArray(pDevice, eFormat, uWidth, uHeight, uSlices);

	m_texture.GetPlatform().BindInt(0);

	m_bHasStencil = (eFormat == DF_DEPTH_24_S8);
}

void DepthStencilBuffer_ps::InitCube(GFXDevice* pDevice, uint32 uWidth, uint32 uHeight, DepthFormat eFormat, SampleCount eSamples, uint32 uFlags)
{
	m_uWidth = uWidth;
	m_uHeight = uHeight;

	m_texture.GetPlatform().InitCubeMap(pDevice, eFormat, uWidth, uHeight);

	m_texture.GetPlatform().BindInt(0);

	m_bHasStencil = (eFormat == DF_DEPTH_24_S8);
}

void DepthStencilBuffer_ps::Resize(GFXDevice* pDevice, uint32 uWidth, uint32 uHeight)
{
	m_texture.GetPlatform().Resize(pDevice, uWidth, uHeight);
}

void DepthStencilBuffer_ps::CleanUp(GFXDevice* pDevice)
{
	m_texture.CleanUp(pDevice);
}

}