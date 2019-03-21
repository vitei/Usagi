/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: The graphics object (avoiding a static class as we want to
//	have strit control over where in code this can be accessed)
*****************************************************************************/
#ifndef _USG_GRAPHICS_GFXCONTEXT_H_
#define _USG_GRAPHICS_GFXCONTEXT_H_
#include "Engine/Common/Common.h"
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

	// Transfer a render target to the scan buffer
	void Transfer(RenderTarget* pTarget, Display* pDisplay);
	void TransferRect(RenderTarget* pTarget, Display* pDisplay, const GFXBounds& srcBounds, const GFXBounds& dstBounds);
	void TransferSpectatorDisplay(IHeadMountedDisplay* pHMD, Display* pDisplay);

	void ApplyDefaults();
	void SetPipelineState(PipelineStateHndl pipeline);
	void SetDescriptorSet(const DescriptorSet* pSet, uint32 uIndex);

	void SetRenderTarget(RenderTarget* pTarget, const Viewport* pViewport = NULL);
	void SetRenderTargetLayer(RenderTarget* pTarget, uint32 uLayer);
	void RenderToDisplay(Display* pDisplay, uint32 uClearFlags = 0);

	void ClearRenderTarget(uint32 uFlags = RenderTarget::RT_FLAG_COLOR);

	
	PipelineStateHndl	GetActivePipeline() { return m_activeStateGroup; }

	void ApplyViewport(const Viewport& viewport);
	void SetScissorRect(uint32 uLeft, uint32 uBottom, uint32 uWidth, uint32 uHeight);
	void DisableScissor();
	void SetBlendColor(const Color& color) { m_platform.SetBlendColor(color); }

	GFXContext_ps&	GetPlatform()	{ return m_platform; }

	void SetVertexBuffer(const VertexBuffer* pBuffer, uint32 uSlot);
	void DrawImmediate(uint32 uCount, uint32 uOffset = 0);
	void DrawIndexed(const IndexBuffer* pBuffer);	
	void DrawIndexedEx(const IndexBuffer* pBuffer, uint32 uStartIndex, uint32 uIndexCount, uint32 uInstanceCount = 1);

	void BeginGPUTag(const char* szName);
	void EndGPUTag();
	void EnableProfiling(bool bProfile) { m_platform.EnableProfiling(bProfile); }

	RenderTarget* GetActiveRenderTarget() const{ return m_pActiveRT; }

	void	InvalidatePipelineOnly();
	void	InvalidateStates();
	void	ValidateDescriptors();
private:

	PRIVATIZE_COPY(GFXContext);

	struct ViewportCache
	{
		uint32 uBottom;
		uint32 uLeft;
		uint32 uWidth;
		uint32 uHeight;
	};

	
	GFXContext_ps			m_platform;

	const InputBinding*		m_pActiveBinding;
	RenderTarget*			m_pActiveRT;

	PipelineStateHndl		m_activeStateGroup;
	ViewportCache			m_activeViewport;
	
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

inline void GFXContext::SetVertexBuffer(const VertexBuffer* pBuffer, uint32 uSlot=0)
{
#if !DISABLE_STATE_SHADOWING	
	if(m_pActiveVBOs[uSlot] != pBuffer)
#endif
	{
		ASSERT(m_pActiveBinding!=NULL);
		m_platform.SetVertexBuffer(pBuffer, m_pActiveBinding, uSlot);
		m_pActiveVBOs[uSlot] = pBuffer;
	}
}

inline void GFXContext::BeginGPUTag(const char* szName)
{
	m_platform.BeginGPUTag(szName);
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
