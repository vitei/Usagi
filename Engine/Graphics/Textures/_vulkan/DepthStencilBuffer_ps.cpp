/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Graphics/Device/GFXDevice.h"
#include API_HEADER(Engine/Graphics/Textures, TextureFormat_ps.h)
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

	/* VULKAN_KEY_START */
    VkImageCreateInfo image_info = {};
    
    m_texture.GetPlatform().Init(pDevice, eFormat, uWidth, uHeight, uFlags);


	m_attachDesc.format = gDepthFormatViewMap[eFormat];
	m_attachDesc.samples = VK_SAMPLE_COUNT_1_BIT;
	m_attachDesc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	m_attachDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	m_attachDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	m_attachDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	m_attachDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	m_attachDesc.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;


	m_bHasStencil = (eFormat == DF_DEPTH_24_S8);
}


void DepthStencilBuffer_ps::InitArray(GFXDevice* pDevice, uint32 uWidth, uint32 uHeight, uint32 uSlices, DepthFormat eFormat, SampleCount eSamples, uint32 uFlags)
{
	m_uWidth = uWidth;
	m_uHeight = uHeight;
	
	m_texture.GetPlatform().InitArray(pDevice, eFormat, uWidth, uHeight, uSlices);


	m_bHasStencil = (eFormat == DF_DEPTH_24_S8);
}


void DepthStencilBuffer_ps::InitCube(GFXDevice* pDevice, uint32 uWidth, uint32 uHeight, DepthFormat eFormat, SampleCount eSamples, uint32 uFlags)
{
	m_uWidth = uWidth;
	m_uHeight = uHeight;

	m_texture.GetPlatform().InitCubeMap(pDevice, eFormat, uWidth, uHeight);

	m_bHasStencil = (eFormat == DF_DEPTH_24_S8);
}

void DepthStencilBuffer_ps::CleanUp(GFXDevice* pDevice)
{
	m_texture.CleanUp(pDevice);
}

void DepthStencilBuffer_ps::Resize(GFXDevice* pDevice, uint32 uWidth, uint32 uHeight)
{
	m_texture.GetPlatform().Resize(pDevice, uWidth, uHeight);
	m_uWidth = uWidth;
	m_uHeight = uHeight;
}

}