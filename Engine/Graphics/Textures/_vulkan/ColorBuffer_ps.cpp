/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Graphics/Color.h"
#include "Engine/Graphics/Viewports/Viewport.h"
#include "Engine/Graphics/Textures/DepthStencilBuffer.h"
#include "Engine/Graphics/GFX.h"
#include "Engine/Graphics/Device/GFXDevice.h"
#include API_HEADER(Engine/Graphics/Textures, TextureFormat_ps.h)
#include API_HEADER(Engine/Graphics/Textures, ColorBuffer_ps.h)

namespace usg {

ColorBuffer_ps::ColorBuffer_ps()
{
	m_texHndl = &m_texture;
}

ColorBuffer_ps::~ColorBuffer_ps()
{
	m_texHndl.reset(NULL);
}

void ColorBuffer_ps::Init(GFXDevice* pDevice, uint32 uWidth, uint32 uHeight, ColorFormat eFormat, SampleCount eSamples, uint32 uFlags, uint32 uLoc, uint32 uMipmaps)
{
	m_texture.GetPlatform().Init(pDevice, eFormat, uWidth, uHeight, uMipmaps, nullptr, TD_TEXTURE2D, uFlags);

	m_attachDesc.format = gColorFormatMap[eFormat];
	m_attachDesc.samples = VK_SAMPLE_COUNT_1_BIT;
	m_attachDesc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	m_attachDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	m_attachDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	m_attachDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	m_attachDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	m_attachDesc.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
}


void ColorBuffer_ps::InitArray(GFXDevice* pDevice, uint32 uBufferId, uint32 uWidth, uint32 uHeight, uint32 uSlices, ColorFormat eFormat, SampleCount eSamples, uint32 uFlags)
{
	ASSERT(false); // Untested
	m_texture.GetPlatform().InitArray(pDevice, eFormat, uWidth, uHeight, uSlices);
}

void ColorBuffer_ps::Resize(GFXDevice* pDevice, uint32 uWidth, uint32 uHeight)
{
	ASSERT(false);
}

void ColorBuffer_ps::CleanUp(GFXDevice* pDevice)
{
	
}


}