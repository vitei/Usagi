/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Graphics/Primitives/IndexBuffer.h"
#include "Engine/Graphics/Primitives/VertexBuffer.h"
#include "Engine/Graphics/Device/GFXDevice.h"
#include "Engine/Graphics/Effects/Effect.h"
#include "Engine/Resource/ResourceMgr.h"
#include "Engine/Graphics/Textures/DepthStencilBuffer.h"
#include "Engine/Graphics/LookupTable.h"
#include "Engine/Graphics/Device/DescriptorSet.h"
#include API_HEADER(Engine/Graphics/Device, RasterizerState.h)
#include API_HEADER(Engine/Graphics/Device, AlphaState.h)
#include API_HEADER(Engine/Graphics/Device, DepthStencilState.h)
#include "Engine/Graphics/Device/GFXContext.h"

#define BUFFER_OFFSET(i) ((void*)(i))

namespace usg {


GFXContext_ps::GFXContext_ps()
{
	m_pParent = NULL;
}

GFXContext_ps::~GFXContext_ps()
{

}

void GFXContext_ps::Init(GFXDevice* pDevice, GFXContext* pParent, bool bDeferred, uint32 uSizeMul)
{
	VkResult err;
	VkCommandBufferAllocateInfo cmd = {};
	cmd.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cmd.pNext = NULL;
	cmd.commandPool = pDevice->GetPlatform().GetCommandPool();
	cmd.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	cmd.commandBufferCount = 1;

	err = vkAllocateCommandBuffers(pDevice->GetPlatform().GetVKDevice(), &cmd, &m_cmdBuff);
	ASSERT(!err);

	m_pParent = pParent;

}

void GFXContext_ps::Begin(bool bApplyDefaults)
{
	VkResult err;

	VkCommandBufferInheritanceInfo cmd_buf_hinfo = {};
	cmd_buf_hinfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
	cmd_buf_hinfo.pNext = NULL;
	cmd_buf_hinfo.renderPass = VK_NULL_HANDLE;
	cmd_buf_hinfo.subpass = 0;
	cmd_buf_hinfo.framebuffer = VK_NULL_HANDLE;
	cmd_buf_hinfo.occlusionQueryEnable = VK_FALSE;
	cmd_buf_hinfo.queryFlags = 0;
	cmd_buf_hinfo.pipelineStatistics = 0;
	
	VkCommandBufferBeginInfo cmd_buf_info = {};
	cmd_buf_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cmd_buf_info.pNext = NULL;
	cmd_buf_info.flags = 0;
	cmd_buf_info.pInheritanceInfo = &cmd_buf_hinfo;

	// Reset the command buffer to re-use it. Later we should just have pregenerated buffers per 
	err = vkResetCommandBuffer(m_cmdBuff, 0);
	ASSERT(!err);

	err = vkBeginCommandBuffer(m_cmdBuff, &cmd_buf_info);
	ASSERT(!err);
}

void GFXContext_ps::End()
{
	VkResult res = vkEndCommandBuffer(m_cmdBuff);
	ASSERT(res == VK_SUCCESS);
}

void GFXContext_ps::Transfer(RenderTarget* pTarget, Display* pDisplay)
{
	pDisplay->GetPlatform().Transfer(m_pParent, pTarget);
}

void GFXContext_ps::TransferRect(RenderTarget* pTarget, Display* pDisplay, uint32 uX, uint32 uY, uint32 uWidth, uint32 uHeight)
{
	pDisplay->GetPlatform().TransferRect(m_pParent, pTarget, uX, uY, uWidth, uHeight);
}

void GFXContext_ps::SetRenderTargetLayer(const RenderTarget* pTarget, uint32 uLayer, uint32 uClearFlags)
{
	// TODO: Implement me

}


void GFXContext_ps::SetRenderTarget(const RenderTarget* pTarget)
{
	if(pTarget)
	{
		const RenderTarget_ps& rtPS = pTarget->GetPlatform();


	}
	else
	{
		
	}
}

void GFXContext_ps::EndRTDraw(const RenderTarget* pTarget)
{

}

void GFXContext_ps::SetPipelineState(PipelineStateHndl& hndl, PipelineStateHndl& prev)
{
	m_pipelineLayout = hndl.GetContents()->GetPlatform().GetLayout();
	vkCmdBindPipeline(m_cmdBuff, VK_PIPELINE_BIND_POINT_GRAPHICS, hndl.GetContents()->GetPlatform().GetPipeline());
}

void GFXContext_ps::SetDescriptorSet(const DescriptorSet* pSet, uint32 uIndex)
{
	// FIXME: Need to re-apply when the layout changes
	pSet->GetPlatform().Bind(m_cmdBuff, m_pipelineLayout, uIndex);
}

void GFXContext_ps::ApplyViewport(const RenderTarget* pActiveRT, const Viewport &viewport)
{
	VkViewport vkViewport;
	memset(&vkViewport, 0, sizeof(vkViewport));
	vkViewport.x = (float)viewport.GetLeft();
	vkViewport.y = (float)viewport.GetBottom();
	vkViewport.height = (float)viewport.GetHeight();
	vkViewport.width = (float)viewport.GetWidth();
	vkViewport.minDepth = 0.0f;
	vkViewport.maxDepth = 1.0f;
	vkCmdSetViewport(m_cmdBuff, 0, 1, &vkViewport);
}

void GFXContext_ps::SetBlendColor(const Color& blendColor)
{
	vkCmdSetBlendConstants(m_cmdBuff, blendColor.m_rgba);
}

void GFXContext_ps::SetScissorRect(const RenderTarget* pActiveTarget, uint32 uLeft, uint32 uBottom, uint32 uWidth, uint32 uHeight)
{
	VkRect2D scissor;
	memset(&scissor, 0, sizeof(scissor));
	scissor.extent.width = uWidth;
	scissor.extent.height = uHeight;
	scissor.offset.x = 0;
	scissor.offset.y = 0;
	vkCmdSetScissor(m_cmdBuff, 0, 1, &scissor);
}

void GFXContext_ps::DisableScissor(const RenderTarget* pActiveTarget, uint32 uLeft, uint32 uBottom, uint32 uWidth, uint32 uHeight)
{
	VkRect2D scissor;
	memset(&scissor, 0, sizeof(scissor));
	scissor.extent.width = uWidth;
	scissor.extent.height = uHeight;
	scissor.offset.x = 0;
	scissor.offset.y = 0;
	vkCmdSetScissor(m_cmdBuff, 0, 1, &scissor);
}


void GFXContext_ps::ClearRenderTarget(RenderTarget* pRT, uint32 uFlags)
{
	static VkClearAttachment attachments[MAX_RENDER_TARGETS];
	uint32 uClearCount = 0;

	uint32 uGlFlags = 0;
	RenderTarget_ps* pRTPS = &pRT->GetPlatform();

	ASSERT(false);
}


void GFXContext_ps::SetVertexBuffer(const VertexBuffer* pBuffer, const EffectBinding* pBinding, uint32 uSlot)
{
	// TODO: Change the interface so we are setting them all at once
	const VertexBuffer_ps& vbPS = pBuffer->GetPlatform();
	const VkBuffer buffers[] = { vbPS.GetBuffer() };
	VkDeviceSize offset = 0;
	vkCmdBindVertexBuffers(m_cmdBuff, uSlot, 1, buffers, &offset); 
}

void GFXContext_ps::DrawImmediate(uint32 uVertCount, uint32 uOffset)
{
	vkCmdDraw(m_cmdBuff, uVertCount, 1, uOffset, 0);
}

void GFXContext_ps::DrawIndexed(const IndexBuffer* pBuffer, uint32 uStartIndex, uint32 uIndexCount, uint32 uInstances)
{
	const IndexBuffer_ps& ibPS = pBuffer->GetPlatform();
	// The primitive type is actually defined by the pipeline
	vkCmdBindIndexBuffer(m_cmdBuff, ibPS.GetBuffer(), 0, ibPS.GetType());
	vkCmdDrawIndexed(m_cmdBuff, uIndexCount, uInstances, uStartIndex, 0, 0);
}



void GFXContext_ps::CopyDepthToSlice(RenderTarget* pSrc, RenderTarget* pDst, uint32 uSlice)
{
	
}


void GFXContext_ps::ImageBarrier(VkImage image, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask, VkImageLayout oldLayout, VkImageLayout newLayout)
{
	VkImageMemoryBarrier imb;
	memset(&imb, 0, sizeof(imb));
	imb.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imb.pNext = NULL;
	imb.srcAccessMask = srcAccessMask;
	imb.dstAccessMask = dstAccessMask;
	imb.oldLayout = oldLayout;
	imb.newLayout = newLayout;
	imb.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imb.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imb.image = image;
	imb.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imb.subresourceRange.baseMipLevel = 0;
	imb.subresourceRange.levelCount = 1;
	imb.subresourceRange.baseArrayLayer = 0;
	imb.subresourceRange.layerCount = 1;

	vkCmdPipelineBarrier(m_cmdBuff, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, NULL, 0, NULL, 1, &imb);
}


}