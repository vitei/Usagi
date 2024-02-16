/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: The graphics object (avoiding a static class as we want to
//	have strit control over where in code this can be accessed)
*****************************************************************************/
#ifndef _USG_GRAPHICS_GFXCONTEXT_H_
#define _USG_GRAPHICS_GFXCONTEXT_H_

#include "Engine/Graphics/Textures/RenderTarget.h"
#include API_HEADER(Engine/Graphics/Device, GFXContext_ps.h)

namespace usg {

class DescriptorSet;
class VertexBuffer;
class Display;
struct GFXBounds;
class InputBinding;

#define DISABLE_STATE_SHADOWING 0

class GFXContext
{
public:
	GFXContext();
	~GFXContext();

	void Init(GFXDevice* pDevice, bool bDeferred, uint32 uSizeMul=1);

	void Begin(bool bApplyDefaults);
	void End();
	void Cleanup(GFXDevice* pDevice);

	// Transfer a render target to the scan buffer
	void Transfer(RenderTarget* pTarget, Display* pDisplay);
	void TransferRect(RenderTarget* pTarget, Display* pDisplay, const GFXBounds& srcBounds, const GFXBounds& dstBounds);
	void TransferSpectatorDisplay(IHeadMountedDisplay* pHMD, Display* pDisplay);
	void TransferToHMD(RenderTarget* pTarget, IHeadMountedDisplay* pDisplay, bool bLeftEye);

	void ApplyDefaults();
	void SetPipelineState(PipelineStateHndl pipeline);
	void SetDescriptorSet(const DescriptorSet* pSet, uint32 uIndex);

	void SetRenderTarget(RenderTarget* pTarget, const Viewport* pViewport = NULL);
	void SetRenderTargetLayer(RenderTarget* pTarget, uint32 uLayer);
	void SetRenderTargetMip(RenderTarget* pTarget, uint32 uMip);
	void RenderToDisplay(Display* pDisplay, uint32 uClearFlags = 0);

	void ClearRenderTarget(uint32 uFlags = RenderTarget::RT_FLAG_COLOR);
	void ClearImage(const TextureHndl& texture, const Color& col);

	
	PipelineStateHndl	GetActivePipeline() { return m_activeStateGroup; }

	void ApplyViewport(const Viewport& viewport);
	void SetScissorRect(uint32 uLeft, uint32 uBottom, uint32 uWidth, uint32 uHeight);
	void DisableScissor();
	void SetBlendColor(const Color& color) { m_platform.SetBlendColor(color); }

	GFXContext_ps&	GetPlatform()	{ return m_platform; }

	void SetVertexBuffer(const VertexBuffer* pBuffer, uint32 uSlot = 0, uint32 uVertOffset = 0);
	void DrawImmediate(uint32 uCount, uint32 uOffset = 0);
	void DrawIndexed(const IndexBuffer* pBuffer);	
	void DrawIndexedEx(const IndexBuffer* pBuffer, uint32 uStartIndex, uint32 uIndexCount, uint32 uInstanceCount = 1);

	void BeginGPUTag(const char* szName, const Color& color = Color::White);
	void EndGPUTag();
	void EnableProfiling(bool bProfile) { m_platform.EnableProfiling(bProfile); }

	RenderTarget* GetActiveRenderTarget() const{ return m_pActiveRT; }
	const GFXBounds& GetActiveViewport() const { return m_activeViewport; }

	void	InvalidatePipelineOnly();
	void	InvalidateStates();
	void	ValidateDescriptors();
private:

	PRIVATIZE_COPY(GFXContext);

	
	GFXContext_ps			m_platform;

	const InputBinding*		m_pActiveBinding;
	RenderTarget*			m_pActiveRT;

	PipelineStateHndl		m_activeStateGroup;
	// TODO: When OpenGL is removed we should have to cache these anymore
	GFXBounds				m_activeViewport;
	GFXBounds				m_activeTargetBounds;
	
	const VertexBuffer*		m_pActiveVBOs[MAX_VERTEX_BUFFERS];
	const DescriptorSet*	m_pActiveDescSets[MAX_DESCRIPTOR_SETS];

	bool					m_bActive;
	bool					m_bRenderToDisplay;
	uint32					m_uRTMask;

	uint32					m_uDirtyDescSetFlags;

	// Need to be kept here as the PC needs to rebind these on changing effect
	//const ConstantSet*		m_pStaticConstSets[SHADER_CONSTANT_COUNT];
};

inline void GFXContext::ClearRenderTarget(uint32 uFlags)
{
	m_platform.ClearRenderTarget(m_pActiveRT, uFlags);
}

inline void GFXContext::ClearImage(const TextureHndl& texture, const Color& col)
{
	m_platform.ClearImage(texture, col);
}

inline void GFXContext::SetVertexBuffer(const VertexBuffer* pBuffer, uint32 uSlot, uint32 uVertOffset)
{
#if !DISABLE_STATE_SHADOWING	
	if(m_pActiveVBOs[uSlot] != pBuffer)
#endif
	{
		ASSERT(m_pActiveBinding!=NULL);
		m_platform.SetVertexBuffer(pBuffer, m_pActiveBinding, uSlot, uVertOffset);
		m_pActiveVBOs[uSlot] = pBuffer;
	}
}

inline void GFXContext::BeginGPUTag(const char* szName, const Color& color)
{
	m_platform.BeginGPUTag(szName, color);
}

inline void GFXContext::EndGPUTag()
{
	m_platform.EndGPUTag();
}


inline void GFXContext::SetDescriptorSet(const DescriptorSet* pSet, uint32 uIndex)
{
	if (m_pActiveDescSets[uIndex] != pSet)
	{
		// Apply the descriptor set
		// TODO: Remove the internal function once we are passed the point of a platform independent test
		m_platform.SetDescriptorSet(pSet, uIndex);
		m_pActiveDescSets[uIndex] = pSet;
		m_uDirtyDescSetFlags |= (1 << uIndex);
	}
}


}


#endif
