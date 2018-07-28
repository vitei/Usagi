/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Graphics/Color.h"
#include "Engine/Graphics/Viewports/Viewport.h"
#include "Engine/Graphics/Textures/DepthStencilBuffer.h"
#include "Engine/Graphics/GFX.h"
#include "Engine/Graphics/Device/GFXDevice.h"
#include API_HEADER(Engine/Graphics/Device, GFXDevice_ps.h)
#include API_HEADER(Engine/Graphics/Textures, TextureFormat_ps.h)
#include API_HEADER(Engine/Graphics/Textures, ColorBuffer_ps.h)

namespace usg {

ColorBuffer_ps::ColorBuffer_ps()
	: m_pLayerViews(nullptr)
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
}


void ColorBuffer_ps::InitArray(GFXDevice* pDevice, uint32 uBufferId, uint32 uWidth, uint32 uHeight, uint32 uSlices, ColorFormat eFormat, SampleCount eSamples, uint32 uFlags)
{
	m_texture.GetPlatform().InitArray(pDevice, eFormat, uWidth, uHeight, uSlices);
	if (uSlices > 1)
	{
		InitLayerViews(pDevice);
	}

}

void ColorBuffer_ps::InitLayerViews(GFXDevice* pDevice)
{
	if (m_texture.GetPlatform().GetFaces() > 1)
	{
		m_pLayerViews = vnew(ALLOC_GFX_RENDER_TARGET) VkImageView[m_texture.GetPlatform().GetFaces()];

		for (uint32 i = 0; i < m_texture.GetPlatform().GetFaces(); i++)
		{
			m_pLayerViews[i] = m_texture.GetPlatform().CreateLayerImageView(pDevice, i);
		}
	}
}

void ColorBuffer_ps::FreeLayerViews(GFXDevice* pDevice)
{
	if (m_pLayerViews)
	{
		for (uint32 i = 0; i < m_texture.GetPlatform().GetFaces(); i++)
		{
			vkDestroyImageView(pDevice->GetPlatform().GetVKDevice(), m_pLayerViews[i], nullptr);
		}
	}
}

void ColorBuffer_ps::Resize(GFXDevice* pDevice, uint32 uWidth, uint32 uHeight)
{
	FreeLayerViews(pDevice);
	m_texture.GetPlatform().Resize(pDevice, uWidth, uHeight);
	InitLayerViews(pDevice);

}

void ColorBuffer_ps::CleanUp(GFXDevice* pDevice)
{
	if (m_pLayerViews)
	{
		vdelete[] m_pLayerViews;
		m_pLayerViews = nullptr;
	}
	m_texture.CleanUp(pDevice);
}


}