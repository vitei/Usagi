/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Graphics/Device/GFXDevice.h"
#include API_HEADER(Engine/Graphics/Device, GFXDevice_ps.h)
#include API_HEADER(Engine/Graphics/Textures, TextureFormat_ps.h)
#include "DepthStencilBuffer_ps.h"

namespace usg {

DepthStencilBuffer_ps::DepthStencilBuffer_ps() 
	: m_uWidth(0)
	, m_uHeight(0)
	, m_pLayerViews(nullptr)
{

	m_textureHndl = &m_texture;
}

DepthStencilBuffer_ps::~DepthStencilBuffer_ps()
{
	m_textureHndl.reset(NULL);
	ASSERT(m_pLayerViews == nullptr);
}


void DepthStencilBuffer_ps::Init(GFXDevice* pDevice, uint32 uWidth, uint32 uHeight, DepthFormat eFormat, SampleCount eSamples, uint32 uFlags)
{
	m_uWidth = uWidth;
	m_uHeight = uHeight;

	/* VULKAN_KEY_START */
    VkImageCreateInfo image_info = {};
    
    m_texture.GetPlatform().Init(pDevice, eFormat, uWidth, uHeight, uFlags);

	m_bHasStencil = (eFormat == DepthFormat::DEPTH_24_S8);
}


void DepthStencilBuffer_ps::InitArray(GFXDevice* pDevice, uint32 uWidth, uint32 uHeight, uint32 uSlices, DepthFormat eFormat, SampleCount eSamples, uint32 uFlags)
{
	m_uWidth = uWidth;
	m_uHeight = uHeight;
	
	m_texture.GetPlatform().InitArray(pDevice, eFormat, uWidth, uHeight, uSlices, uFlags);
	m_texture.GetPlatform().SetName(pDevice, "Depth Buffer");

	InitLayerViews(pDevice);


	m_bHasStencil = (eFormat == DepthFormat::DEPTH_24_S8);
}


void DepthStencilBuffer_ps::InitCube(GFXDevice* pDevice, uint32 uWidth, uint32 uHeight, DepthFormat eFormat, SampleCount eSamples, uint32 uFlags)
{
	m_uWidth = uWidth;
	m_uHeight = uHeight;

	m_texture.GetPlatform().InitCubeMap(pDevice, eFormat, uWidth, uHeight, uFlags);
	m_texture.GetPlatform().SetName(pDevice, "Depth Buffer");

	ASSERT(m_pLayerViews == nullptr);
	InitLayerViews(pDevice);

	m_bHasStencil = (eFormat == DepthFormat::DEPTH_24_S8);
}

void DepthStencilBuffer_ps::Cleanup(GFXDevice* pDevice)
{
	FreeLayerViews(pDevice);
	m_texture.Cleanup(pDevice);
}

void DepthStencilBuffer_ps::Resize(GFXDevice* pDevice, uint32 uWidth, uint32 uHeight)
{
	FreeLayerViews(pDevice);
	m_texture.GetPlatform().Resize(pDevice, uWidth, uHeight);
	InitLayerViews(pDevice);
	m_uWidth = uWidth;
	m_uHeight = uHeight;
}

void DepthStencilBuffer_ps::InitLayerViews(GFXDevice* pDevice)
{
	if (m_texture.GetPlatform().GetFaces() > 1)
	{
		m_pLayerViews = vnew(ALLOC_GFX_RENDER_TARGET) VkImageView[m_texture.GetPlatform().GetFaces()];

		for (uint32 i = 0; i < m_texture.GetPlatform().GetFaces(); i++)
		{
			m_pLayerViews[i] = m_texture.GetPlatform().CreateImageView(pDevice, i, 0);
		}
	}
}

void DepthStencilBuffer_ps::FreeLayerViews(GFXDevice* pDevice)
{
	if (m_pLayerViews)
	{
		for (uint32 i = 0; i < m_texture.GetPlatform().GetFaces(); i++)
		{
			vkDestroyImageView(pDevice->GetPlatform().GetVKDevice(), m_pLayerViews[i], nullptr);
		}
		vdelete[] m_pLayerViews;
		m_pLayerViews = nullptr;
	}
}

}