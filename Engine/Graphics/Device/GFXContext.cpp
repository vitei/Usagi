/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Graphics/Effects/ConstantSet.h"
#include "Engine/Graphics/Device/RenderState.h"
#include "Engine/Graphics/Device/PipelineState.h"
#include "Engine/Graphics/Primitives/IndexBuffer.h"
#include "Engine/Graphics/Effects/InputBinding.h"
#include "Engine/Graphics/Primitives/VertexBuffer.h"
#include "Engine/Graphics/RenderConsts.h"
#include "Engine/Graphics/Device/DescriptorSet.h"
#include API_HEADER(Engine/Graphics/Device, AlphaState.h)
#include API_HEADER(Engine/Graphics/Device, PipelineLayout.h)
#include "GFXDevice.h"
#include "GFXContext.h"

namespace usg {


GFXContext::GFXContext()
{
	InvalidateStates();
}	

GFXContext::~GFXContext()
{

}

void GFXContext::Init(GFXDevice* pDevice, bool bDeferred, uint32 uSizeMul)
{
	m_pActiveBinding	= NULL;
	m_pActiveRT			= NULL;
	m_bRenderToDisplay = false;

	m_platform.Init(pDevice, this, bDeferred, uSizeMul);
}

void GFXContext::ApplyViewport(const Viewport& viewport)
{
	ASSERT(m_pActiveRT!=NULL);
	m_activeViewport.uBottom = viewport.GetBottom();
	m_activeViewport.uLeft = viewport.GetLeft();
	m_activeViewport.uWidth = viewport.GetWidth();
	m_activeViewport.uHeight = viewport.GetHeight();
	m_platform.ApplyViewport(m_pActiveRT, viewport);
}

void GFXContext::SetScissorRect(uint32 uLeft, uint32 uBottom, uint32 uWidth, uint32 uHeight)
{
	m_platform.SetScissorRect(m_pActiveRT, uLeft, uBottom, uWidth, uHeight);
}

void GFXContext::DisableScissor()
{
	m_platform.DisableScissor(m_pActiveRT, m_activeViewport.uLeft, m_activeViewport.uBottom, m_activeViewport.uWidth, m_activeViewport.uHeight);
}

void GFXContext::Begin(bool bApplyDefaults)
{
	m_pActiveRT = NULL;
	m_bRenderToDisplay = false;
	InvalidateStates();
	m_platform.Begin(bApplyDefaults);
}

void GFXContext::ApplyDefaults()
{
	InvalidateStates();
	m_platform.ApplyDefaults();

}


void GFXContext::SetRenderTarget(RenderTarget* pTarget, const Viewport* pViewport)
{
	bool bHadDS = false;
	m_bRenderToDisplay = false;
	if(m_pActiveRT)
	{
		m_platform.EndRTDraw(m_pActiveRT);
		bHadDS = m_pActiveRT->GetDepthStencilBuffer()!=NULL;
		m_pActiveRT = NULL;
	}

	if(pTarget)
	{
		if(pTarget != m_pActiveRT)
		{
			bool bHasDS = pTarget->GetDepthStencilBuffer()!=NULL;

			m_platform.SetRenderTarget(pTarget);
			
			uint32 uRTMask = pTarget->GetTargetMask();
			if(uRTMask != m_uRTMask)
			{
				m_uRTMask = uRTMask;
			}
		}

		m_pActiveRT = pTarget;

		if(pViewport)
		{
			ApplyViewport(*pViewport);
		}
		else
		{
			ApplyViewport(pTarget->GetViewport());
		}
	}
}

void GFXContext::RenderToDisplay(Display* pDisplay, uint32 uClearFlags)
{
	if (m_pActiveRT)
	{
		m_platform.EndRTDraw(m_pActiveRT);
	}
	m_bRenderToDisplay = true;
	m_platform.RenderToDisplay(pDisplay, uClearFlags);
	m_pActiveRT = nullptr;
}

void GFXContext::SetRenderTargetLayer(RenderTarget* pTarget, uint32 uLayer)
{
	bool bHadDS = false;
	if(m_pActiveRT)
	{
		m_platform.EndRTDraw(m_pActiveRT);
		bHadDS = m_pActiveRT->GetDepthStencilBuffer()!=NULL;
		m_pActiveRT = NULL;
	}	

	if(pTarget)
	{

		if(pTarget != m_pActiveRT)
		{
			bool bHasDS = pTarget->GetDepthStencilBuffer()!=NULL;

			m_platform.SetRenderTargetLayer(pTarget, uLayer);
			m_pActiveRT = pTarget;

			
			uint32 uRTMask = pTarget->GetTargetMask();
			if(uRTMask != m_uRTMask)
			{
				m_uRTMask = uRTMask;
			}
		}

		ApplyViewport(pTarget->GetViewport());
	}
}

void GFXContext::End()
{
	if (m_pActiveRT || m_bRenderToDisplay)
	{
		m_platform.EndRTDraw(m_pActiveRT);
	}
	m_bRenderToDisplay = false;
	m_pActiveRT = nullptr;
	m_platform.End();
}

void GFXContext::InvalidatePipelineOnly()
{
	m_activeStateGroup.Invalidate();
}

void GFXContext::InvalidateStates()
{
	m_pActiveBinding = NULL;
	m_activeStateGroup.Invalidate();

	m_uDirtyDescSetFlags = 0xFFFFFFFF;

	/*for(uint32 uConstant = 0; uConstant < SHADER_CONSTANT_COUNT; uConstant++ )
	{
		m_pStaticConstSets[uConstant] = NULL;
	}*/

	for (uint32 uSlot = 0; uSlot < MAX_VERTEX_BUFFERS; uSlot++)
	{
		m_pActiveVBOs[uSlot] = NULL;
	}

	for (uint32 uDesc = 0; uDesc < MAX_DESCRIPTOR_SETS; uDesc++)
	{
		m_pActiveDescSets[uDesc] = NULL;
	}

	m_platform.InvalidateStates();
}

void GFXContext::TransferToHMD(RenderTarget* pTarget, IHeadMountedDisplay* pDisplay, bool bLeftEye)
{
	if (m_pActiveRT)
	{
		m_platform.EndRTDraw(m_pActiveRT);
		m_pActiveRT = nullptr;
	}
	BeginGPUTag("Transfer");
	m_platform.TransferToHMD(pTarget, pDisplay, bLeftEye);
	EndGPUTag();
}


void GFXContext::Transfer(RenderTarget* pTarget, Display* pDisplay)
{
	if (m_pActiveRT)
	{
		m_platform.EndRTDraw(m_pActiveRT);
		m_pActiveRT = nullptr;
	}
	BeginGPUTag("Transfer");
	m_platform.Transfer(pTarget, pDisplay);
	EndGPUTag();
}


void GFXContext::TransferRect(RenderTarget* pTarget, Display* pDisplay, const GFXBounds& srcBounds, const GFXBounds& dstBounds)
{
	if (m_pActiveRT)
	{
		m_platform.EndRTDraw(m_pActiveRT);
		m_pActiveRT = nullptr;
	}
	BeginGPUTag("TransferRect");
	m_platform.TransferRect(pTarget, pDisplay, srcBounds, dstBounds);
	EndGPUTag();
}

void GFXContext::TransferSpectatorDisplay(IHeadMountedDisplay* pHMD, Display* pDisplay)
{
	BeginGPUTag("TransferSpectatorDisplay");
	m_platform.TransferSpectatorDisplay(pHMD, pDisplay);
	EndGPUTag();
}

void GFXContext::SetPipelineState(PipelineStateHndl hndl)
{
#if !DISABLE_STATE_SHADOWING
	if (m_activeStateGroup != hndl)
#endif
	{
		PipelineState& group = *(hndl.GetContents());

#ifdef CONFIRM_PIPELINE_VALIDITY
		if (GetActiveRenderTarget() && (hndl.GetContents()->GetRenderPass() != GetActiveRenderTarget()->GetRenderPass()))
		{
			WARNING("The render pass is not that which was promised\n");
		}
#endif

		m_platform.SetPipelineState(hndl, m_activeStateGroup);
		m_activeStateGroup = hndl;
		m_pActiveBinding = group.GetInputBindingInt();
		m_uDirtyDescSetFlags = 0xFFFFFFFF;
	}
	//m_platform.SetBlendColor(group.GetBlendColor());
}

void GFXContext::ValidateDescriptors()
{
#if CONFIRM_PIPELINE_VALIDITY
	PipelineLayout* layout = m_activeStateGroup.GetContents()->GetPipelineLayoutHndl().GetContents();

	for (uint32 i = 0; i < layout->GetDecl().uDescriptorSetCount; i++)
	{
		if (!m_pActiveDescSets[i] || layout->GetDecl().descriptorSets[i] != m_pActiveDescSets[i]->GetLayoutHndl())
		{
			if (m_pActiveDescSets[i] && m_pActiveDescSets[i]->GetValid())
			{
				WARNING("Incompatible descriptor set bound at slot %u\n", i);
			}
			else
			{
				WARNING("Missing descriptor set at slot %u\n", i);
			}
		}
		if (m_pActiveDescSets[i] && !m_pActiveDescSets[i]->IsUptoDate())
		{
			WARNING("Out of date descriptor set at slot %u\n", i);
		}
	}
#endif
}

void GFXContext::DrawImmediate(uint32 uCount, uint32 uOffset)
{
#if CONFIRM_GFX_CMD_VALIDITY
	bool bValid = false;
	for (int i = 0; i < MAX_VERTEX_BUFFERS; i++)
	{
		if (m_pActiveVBOs[i] && m_pActiveVBOs[i]->GetCount() >= (uCount + uOffset))
		{
			bValid = true;
			break;
		}
	}
	ASSERT(bValid);
#endif
	if (m_uDirtyDescSetFlags)
	{
#if CONFIRM_PIPELINE_VALIDITY
		ValidateDescriptors();
#endif
		m_platform.UpdateDescriptors(m_activeStateGroup, m_pActiveDescSets, m_uDirtyDescSetFlags);
		m_uDirtyDescSetFlags = 0;
	}
	m_platform.DrawImmediate(uCount, uOffset);
}


void GFXContext::DrawIndexed(const IndexBuffer* pBuffer)
{
	if (m_uDirtyDescSetFlags)
	{
#if CONFIRM_PIPELINE_VALIDITY
		ValidateDescriptors();
#endif
		m_platform.UpdateDescriptors(m_activeStateGroup, m_pActiveDescSets, m_uDirtyDescSetFlags);
		m_uDirtyDescSetFlags = 0;
	}
	m_platform.DrawIndexed(pBuffer, 0, pBuffer->GetIndexCount(), 1);
}

void GFXContext::DrawIndexedEx(const IndexBuffer* pBuffer, uint32 uStartIndex, uint32 uIndexCount, uint32 uInstanceCount)
{
	if (m_uDirtyDescSetFlags)
	{
#if CONFIRM_PIPELINE_VALIDITY
		ValidateDescriptors();
#endif
		m_platform.UpdateDescriptors(m_activeStateGroup, m_pActiveDescSets, m_uDirtyDescSetFlags);
		m_uDirtyDescSetFlags = 0;
	}
	ASSERT(uStartIndex + uIndexCount <= pBuffer->GetIndexCount());
	m_platform.DrawIndexed(pBuffer, uStartIndex, uIndexCount, uInstanceCount);
}



}
