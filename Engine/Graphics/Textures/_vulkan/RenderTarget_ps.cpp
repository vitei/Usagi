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
}


void RenderTarget_ps::InitMRT(GFXDevice* pDevice, uint32 uColorCount, ColorBuffer** ppColorBuffers, DepthStencilBuffer* pDepth)
{
	// The depth stencil is the last bound attachment
	m_colorClearValues[uColorCount].depthStencil.depth = 1.0f;
	m_colorClearValues[uColorCount].depthStencil.stencil = 0;

	m_imageViews.resize(uColorCount + (pDepth!=nullptr ? 1 : 0));
	for (uint32 i = 0; i < uColorCount; i++)
	{
		m_imageViews[i] = ppColorBuffers[i]->GetTexture()->GetPlatform().GetImageView();
	}

	if (pDepth)
	{
		m_imageViews[uColorCount] = pDepth->GetTexture()->GetPlatform().GetImageView();
	}

	uint32 uLayerCount = uColorCount ? ppColorBuffers[0]->GetTexture()->GetPlatform().GetFaces() : pDepth->GetTexture()->GetPlatform().GetFaces();

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

	m_fbCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	m_fbCreateInfo.pNext = NULL;
	m_fbCreateInfo.attachmentCount = (uint32)m_imageViews.size();
	m_fbCreateInfo.pAttachments = m_imageViews.data();
	m_fbCreateInfo.width = m_uWidth;
	m_fbCreateInfo.height = m_uHeight;
	m_fbCreateInfo.layers = uLayerCount;

	m_fullScreenVP.InitViewport(0, 0, m_uWidth, m_uHeight);

}


void RenderTarget_ps::RenderPassUpdated(usg::GFXDevice* pDevice, const RenderPassHndl &renderPass)
{
	CleanUp(pDevice);
	m_fbCreateInfo.renderPass = renderPass.GetContents()->GetPass();

	VkResult res = vkCreateFramebuffer(pDevice->GetPlatform().GetVKDevice(), &m_fbCreateInfo, NULL, &m_framebuffer);
	ASSERT(res == VK_SUCCESS);

}


void RenderTarget_ps::CleanUp(GFXDevice* pDevice)
{
	if (m_framebuffer != VK_NULL_HANDLE)
	{
		vkDestroyFramebuffer(pDevice->GetPlatform().GetVKDevice(), m_framebuffer, nullptr);
	}
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
	InitMRT(pDevice, uCount, ppColorBuffers, pDepth);
}


}

