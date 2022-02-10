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
	: m_pExtraViews(nullptr)
{
	m_texHndl = &m_texture;
	m_texture.SetReady(true);
	m_uMips = 1;
}

ColorBuffer_ps::~ColorBuffer_ps()
{
	m_texHndl.reset(NULL);
}

void ColorBuffer_ps::Init(GFXDevice* pDevice, uint32 uWidth, uint32 uHeight, ColorFormat eFormat, SampleCount eSamples, uint32 uFlags, uint32 uLoc, uint32 uMipmaps)
{
	m_uMips = uMipmaps;
	m_texture.GetPlatform().Init(pDevice, eFormat, uWidth, uHeight, uMipmaps, nullptr, TD_TEXTURE2D, uFlags);
	if (uMipmaps > 1)
	{
		InitExViews(pDevice);
	}
}


void ColorBuffer_ps::InitArray(GFXDevice* pDevice, uint32 uBufferId, uint32 uWidth, uint32 uHeight, uint32 uSlices, ColorFormat eFormat, SampleCount eSamples, uint32 uFlags)
{
	m_uMips = 1;
	m_texture.GetPlatform().InitArray(pDevice, eFormat, uWidth, uHeight, uSlices);
	if (uSlices > 1)
	{
		InitExViews(pDevice);
	}

}

void ColorBuffer_ps::InitExViews(GFXDevice* pDevice)
{
	if (m_texture.GetPlatform().GetFaces() > 1 || m_uMips > 1)
	{
		m_pExtraViews = vnew(ALLOC_GFX_RENDER_TARGET) VkImageView[m_texture.GetPlatform().GetFaces() * m_uMips];

		for (uint32 i = 0; i < m_texture.GetPlatform().GetFaces(); i++)
		{
			for(uint32 j=0; j<m_uMips; j++)
			{
				m_pExtraViews[(i*m_uMips)+j] = m_texture.GetPlatform().CreateImageView(pDevice, i, j);
			}
		}
	}
}

void ColorBuffer_ps::FreeExViews(GFXDevice* pDevice)
{
	if (m_pExtraViews)
	{
		for (uint32 i = 0; i < m_texture.GetPlatform().GetFaces() * m_uMips; i++)
		{
			vkDestroyImageView(pDevice->GetPlatform().GetVKDevice(), m_pExtraViews[i], nullptr);
		}
	}
}

void ColorBuffer_ps::Resize(GFXDevice* pDevice, uint32 uWidth, uint32 uHeight)
{
	FreeExViews(pDevice);
	m_texture.GetPlatform().Resize(pDevice, uWidth, uHeight);
	InitExViews(pDevice);

}

void ColorBuffer_ps::Cleanup(GFXDevice* pDevice)
{
	if (m_pExtraViews)
	{
		vdelete[] m_pExtraViews;
		m_pExtraViews = nullptr;
	}
	m_texture.Cleanup(pDevice);
}


}