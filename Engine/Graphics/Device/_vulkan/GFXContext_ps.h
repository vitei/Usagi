/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef _USG_GRAPHICS_PC_GFXCONTEXT_
#define _USG_GRAPHICS_PC_GFXCONTEXT_

#include "Engine/Graphics/Device/Display.h"
#include "Engine/Graphics/Primitives/VertexBuffer.h"
#include OS_HEADER(Engine/Graphics/Device, VulkanIncludes.h)

namespace usg {

class DescriptorSet;
class Viewport;
class GFXContext;
class IndexBuffer;
class VertexBuffer;
class ConstantSet;
class Fog;
class IHeadMountedDisplay;


class GFXContext_ps
{
public:
	GFXContext_ps();
	~GFXContext_ps();

	void Init(GFXDevice* pDevice, GFXContext* pParent, bool bDeferred, uint32 uSizeMul);
	void Cleanup(GFXDevice* pDevice);

	void Begin(bool bApplyDefaults);
	void Transfer(RenderTarget* pTarget, Display* pDisplay);
	void TransferRect(RenderTarget* pTarget, Display* pDisplay, const GFXBounds& srcBounds, const GFXBounds& dstBounds);
	void TransferSpectatorDisplay(IHeadMountedDisplay* pHMD, Display* pDisplay);
	void TransferToHMD(RenderTarget* pTarget, IHeadMountedDisplay* pDisplay, bool bLeftEye);

	void End();
	void ForceFinishDraw() { }
	void ApplyDefaults() {}
	void InvalidateStates() {}
	
	void ApplyViewport(const RenderTarget* pActiveRT, const Viewport &viewport, const GFXBounds& targetBounds);
	void SetScissorRect(const RenderTarget* pActiveTarget, uint32 uLeft, uint32 uBottom, uint32 uWidth, uint32 uHeight, const GFXBounds& targetBounds);
	void DisableScissor(const RenderTarget* pActiveTarget, uint32 uLeft, uint32 uBottom, uint32 uWidth, uint32 uHeight, const GFXBounds& targetBounds);
	void SetRenderTarget(const RenderTarget* pTarget);
	void SetRenderTargetLayer(const RenderTarget* pTarget, uint32 uLayers);
	void SetRenderTargetMip(const RenderTarget* pTarget, uint32 uMip);
	void EndRTDraw(const RenderTarget* pTarget);
	void RenderToDisplay(Display* pDisplay, uint32 uClearFlags);

	void SetPipelineState(PipelineStateHndl& hndl, PipelineStateHndl& prev);
	void SetDescriptorSet(const DescriptorSet* pSet, uint32 uIndex);

	void ClearRenderTarget(RenderTarget* pRT, uint32 uFlags);
	void ClearImage(const TextureHndl& texture, const Color& col);

	void SetVertexBuffer(const VertexBuffer* pBuffer, const InputBinding* pBinding, uint32 uSlot);
	void DrawImmediate(uint32 uCount, uint32 uOffset);
	void DrawIndexed(const IndexBuffer* pBuffer, uint32 uStartIndex, uint32 uIndexCount, uint32 uInstances);

	void SetBlendColor(const Color& color);

	void BeginGPUTag(const char* szName, const Color& color);
	void EndGPUTag();
	void EnableProfiling(bool bProfile) {}
	void UpdateDescriptors(const PipelineStateHndl& activePipeline, const DescriptorSet** pDescriptors, uint32 uDirtyFlags);

	// PS Specific functions
	VkCommandBuffer GetVkCmdBuffer() { return m_cmdBuff; }
	void ImageBarrier(VkImage image, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask, VkImageLayout oldLayout, VkImageLayout newLayout);
	void SetImageLayout(VkImage image, VkImageLayout oldImageLayout, VkImageLayout newImageLayout, VkImageSubresourceRange subresourceRange, VkPipelineStageFlags srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);

private:

	GFXContext*			m_pParent;
	VkCommandBuffer		m_cmdBuff;
	VkCommandPool		m_cmdPool;
	VkPipelineLayout	m_pipelineLayout;

	PFN_vkCmdDebugMarkerBeginEXT m_pfnCmdDebugMarkerBegin;
	PFN_vkCmdDebugMarkerEndEXT m_pfnCmdDebugMarkerEnd;
};

}

#endif