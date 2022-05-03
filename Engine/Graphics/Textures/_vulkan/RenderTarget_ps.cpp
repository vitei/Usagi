/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Graphics/Color.h"
#include "Engine/Graphics/Viewports/Viewport.h"
#include "Engine/Graphics/Textures/DepthStencilBuffer.h"
#include "Engine/Graphics/Textures/ColorBuffer.h"
#include "Engine/Graphics/GFX.h"
#include "Engine/Graphics/Device/GFXDevice.h"
#include OS_HEADER(Engine/Graphics/Device, VulkanIncludes.h)
#include API_HEADER(Engine/Graphics/Device, GFXDevice_ps.h)
#include API_HEADER(Engine/Graphics/Device, RenderPass.h)
#include API_HEADER(Engine/Graphics/Textures, RenderTarget_ps.h)

namespace usg {


RenderTarget_ps::RenderTarget_ps()
	: m_framebuffer(VK_NULL_HANDLE)
	, m_uCurrentImage(0)
	, m_uWidth(0)
	, m_uHeight(0)
{

	MemClear(&m_colorClearValues, sizeof(m_colorClearValues));
}

RenderTarget_ps::~RenderTarget_ps()
{
	MemClear(&m_fbCreateInfo, sizeof(m_fbCreateInfo));
	ASSERT(m_framebuffer == VK_NULL_HANDLE);
}


void RenderTarget_ps::InitMRT(GFXDevice* pDevice, uint32 uColorCount, ColorBuffer** ppColorBuffers, DepthStencilBuffer* pDepth)
{
	// The depth stencil is the last bound attachment
	m_colorClearValues[uColorCount].depthStencil.depth = 1.0f;
	m_colorClearValues[uColorCount].depthStencil.stencil = 0;

	uint32 uViewsPerFB = uColorCount + (pDepth != nullptr ? 1 : 0);
	uint32 uLayerCount = uColorCount ? ppColorBuffers[0]->GetTexture()->GetPlatform().GetFaces() : pDepth->GetTexture()->GetPlatform().GetFaces();
	uint32 uMipCount = uColorCount ? ppColorBuffers[0]->GetMipCount() : 1;
	uint32 uLayerFBs = uLayerCount > 1 ? uLayerCount : 0;
	uint32 uMipFBs = uMipCount > 1 ? uMipCount : 0;

	m_imageViews.resize(uViewsPerFB * (uLayerFBs+uMipFBs+1));
	for (uint32 i = 0; i < uColorCount; i++)
	{
		if(uMipCount <= 1)
		{
			m_imageViews[i] = ppColorBuffers[i]->GetTexture()->GetPlatform().GetImageView();
		}
		else
		{
			// More than 1 mip means we can't use the default texture view
			m_imageViews[i] = ppColorBuffers[i]->GetPlatform().GetViewEx(0, 0);
		}
	}

	if (pDepth)
	{
		m_imageViews[uColorCount] = pDepth->GetTexture()->GetPlatform().GetImageView();
	}

	if (pDepth)
	{	
		m_uWidth = pDepth->GetWidth();
		m_uHeight = pDepth->GetHeight();
	}
	else
	{
		m_uWidth = ppColorBuffers[0]->GetWidth();
		m_uHeight = ppColorBuffers[0]->GetHeight();
	}

	usg::MemSet(&m_fbCreateInfo, 0, sizeof(m_fbCreateInfo));
	m_fbCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	m_fbCreateInfo.pNext = NULL;
	m_fbCreateInfo.attachmentCount = uViewsPerFB;
	m_fbCreateInfo.pAttachments = m_imageViews.data();
	m_fbCreateInfo.width = m_uWidth;
	m_fbCreateInfo.height = m_uHeight;
	m_fbCreateInfo.layers = uLayerCount;

	uint32 uViewIdx = uViewsPerFB;
	if (uLayerCount > 1)
	{
		m_layerInfo.resize(uLayerFBs);
		for (uint32 uLayer = 0; uLayer < uLayerCount; uLayer++)
		{
			for (uint32 i = 0; i < uColorCount; i++)
			{
				m_imageViews[uViewIdx + i] = ppColorBuffers[i]->GetPlatform().GetViewEx(uLayer, 0);
			}

			if (pDepth)
			{
				m_imageViews[uViewIdx + uColorCount] = pDepth->GetPlatform().GetLayerView(uLayer);
			}
			m_layerInfo[uLayer].frameBuffer = VK_NULL_HANDLE;
			m_layerInfo[uLayer].createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			m_layerInfo[uLayer].createInfo.pNext = NULL;
			m_layerInfo[uLayer].createInfo.attachmentCount = uViewsPerFB;
			m_layerInfo[uLayer].createInfo.pAttachments = &m_imageViews[uViewIdx];
			m_layerInfo[uLayer].createInfo.width = m_uWidth;
			m_layerInfo[uLayer].createInfo.height = m_uHeight;
			m_layerInfo[uLayer].createInfo.layers = 1;

			uViewIdx += uViewsPerFB;
		}
	}

	if (uMipCount > 1)
	{
		m_mipInfo.resize(uMipFBs);
		for (uint32 uMip = 0; uMip < uMipCount; uMip++)
		{
			for (uint32 i = 0; i < uColorCount; i++)
			{
				m_imageViews[uViewIdx + i] = ppColorBuffers[i]->GetPlatform().GetViewEx(0, uMip);
			}

			ASSERT(!pDepth);
			m_mipInfo[uMip].frameBuffer = VK_NULL_HANDLE;
			m_mipInfo[uMip].createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			m_mipInfo[uMip].createInfo.pNext = NULL;
			m_mipInfo[uMip].createInfo.attachmentCount = uViewsPerFB;
			m_mipInfo[uMip].createInfo.pAttachments = &m_imageViews[uViewIdx];
			m_mipInfo[uMip].createInfo.width = m_uWidth >> uMip;
			m_mipInfo[uMip].createInfo.height = m_uHeight >> uMip;
			m_mipInfo[uMip].createInfo.layers = 1;

			uViewIdx += uViewsPerFB;
		}
	}

	m_fullScreenVP.InitViewport(0, 0, m_uWidth, m_uHeight);

}


void RenderTarget_ps::RenderPassUpdated(usg::GFXDevice* pDevice, const RenderPassHndl &renderPass)
{
	Cleanup(pDevice);
	m_fbCreateInfo.renderPass = renderPass.GetContents()->GetPass();

	usg::GFXDevice_ps& devicePS = pDevice->GetPlatform();

	VkResult res = vkCreateFramebuffer(pDevice->GetPlatform().GetVKDevice(), &m_fbCreateInfo, NULL, &m_framebuffer);
	ASSERT(res == VK_SUCCESS);

	for (auto& itr : m_layerInfo)
	{
		itr.createInfo.renderPass = renderPass.GetContents()->GetPass();
		VkResult res = vkCreateFramebuffer(pDevice->GetPlatform().GetVKDevice(), &itr.createInfo, NULL, &itr.frameBuffer);
		ASSERT(res == VK_SUCCESS);
	}

	for (auto& itr : m_mipInfo)
	{
		itr.createInfo.renderPass = renderPass.GetContents()->GetPass();
		VkResult res = vkCreateFramebuffer(pDevice->GetPlatform().GetVKDevice(), &itr.createInfo, NULL, &itr.frameBuffer);
		ASSERT(res == VK_SUCCESS);
	}

}

void RenderTarget_ps::FreeFramebuffers(GFXDevice* pDevice)
{
	if (m_framebuffer != VK_NULL_HANDLE)
	{
		pDevice->GetPlatform().ReqDestroyFrameBuffer(m_framebuffer);
		m_framebuffer = VK_NULL_HANDLE;
	}

	for (auto& itr : m_layerInfo)
	{
		if (itr.frameBuffer != VK_NULL_HANDLE)
		{
			pDevice->GetPlatform().ReqDestroyFrameBuffer(itr.frameBuffer);
			itr.frameBuffer = VK_NULL_HANDLE;
		}
	}

	for (auto& itr : m_mipInfo)
	{
		if (itr.frameBuffer != VK_NULL_HANDLE)
		{
			pDevice->GetPlatform().ReqDestroyFrameBuffer(itr.frameBuffer);
			itr.frameBuffer = VK_NULL_HANDLE;
		}
	}
}


void RenderTarget_ps::Cleanup(GFXDevice* pDevice)
{
	FreeFramebuffers(pDevice);
}


void RenderTarget_ps::SetClearColor(const Color& col, uint32 uTarget)
{
	m_colorClearValues[uTarget].color.float32[0] = col.r();
	m_colorClearValues[uTarget].color.float32[1] = col.g();
	m_colorClearValues[uTarget].color.float32[2] = col.b();
	m_colorClearValues[uTarget].color.float32[3] = col.a();
}


void RenderTarget_ps::Resize(GFXDevice* pDevice, uint32 uCount, ColorBuffer** ppColorBuffers, DepthStencilBuffer* pDepth)
{
	FreeFramebuffers(pDevice);
	InitMRT(pDevice, uCount, ppColorBuffers, pDepth);
}


}

