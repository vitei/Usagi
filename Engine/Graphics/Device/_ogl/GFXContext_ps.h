/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef _USG_GRAPHICS_PC_GFXCONTEXT_
#define _USG_GRAPHICS_PC_GFXCONTEXT_
#include "Engine/Common/Common.h"
#include "Engine/Graphics/Device/GFXHandles.h"
#include OS_HEADER(Engine/Graphics/Device, OpenGLIncludes.h)

namespace usg {

class AlphaState;
class RasterizerState;
class DepthStencilState;
class Viewport;
class GFXContext;
class IndexBuffer;
class VertexBuffer;
class ConstantSet;
class Light;
class DescriptorSet;
class RenderTarget;
class Display;
class GFXDevice;
struct GFXBounds;
class IHeadMountedDisplay;


class GFXContext_ps
{
public:
	GFXContext_ps();
	~GFXContext_ps();

	void Init(GFXDevice* pDevice, GFXContext* pParent, bool bDeferred, uint32 uSizeMul);

	void Begin(bool bApplyDefaults) {}
	void Transfer(RenderTarget* pTarget, Display* pDisplay);
	void TransferRect(RenderTarget* pTarget, Display* pDisplay, const GFXBounds& srcBounds, const GFXBounds& dstBounds);
	void TransferSpectatorDisplay(IHeadMountedDisplay* pHMD, Display* pDisplay);

	void End() {}
	void ApplyDefaults() {}
	void InvalidateStates() {}
	
	void ApplyViewport(const RenderTarget* pActiveRT, const Viewport &viewport);
	void SetScissorRect(const RenderTarget* pActiveTarget, uint32 uLeft, uint32 uBottom, uint32 uWidth, uint32 uHeight);
	void DisableScissor(const RenderTarget* pActiveTarget, uint32 uLeft, uint32 uBottom, uint32 uWidth, uint32 uHeight);
	void SetRenderTarget(const RenderTarget* pTarget);
	void SetRenderTargetLayer(const RenderTarget* pTarget, uint32 uLayer, uint32 uClearFlags);
	void EndRTDraw(const RenderTarget* pTarget);
	void RenderToDisplay(Display* pDisplay, uint32 uClearFlags);

	void SetPipelineState(PipelineStateHndl& hndl, PipelineStateHndl& prev);
	void SetDescriptorSet(const DescriptorSet* pSet, uint32 uIndex);

	void ClearRenderTarget(RenderTarget* pRT, uint32 uFlags);

	void SetVertexBuffer(const VertexBuffer* pBuffer, const InputBinding* pBinding, uint32 uSlot);
	void DrawImmediate(uint32 uCount, uint32 uOffset);
	void DrawIndexed(const IndexBuffer* pBuffer, uint32 uStartIndex, uint32 uIndexCount, uint32 uInstances);

	void SetBlendColor(const Color& color);


	void BeginGPUTag(const char* szName) {}
	void EndGPUTag() {}
	void EnableProfiling(bool bProfile) {}


private:
	void SetEffect(const Effect* pEffect);
	void SetDepthWrite();
	void RestorePipelineState();

	SamplerHndl				m_samplerHndl;
	SamplerHndl				m_lutSamplerHndl;

	GLuint					m_ePrimitiveType;

	GFXContext*				m_pParent;
};

}

#endif