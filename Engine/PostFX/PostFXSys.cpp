/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Maths/MathUtil.h"
#include "Engine/Graphics/GFX.h"
#include "Engine/Graphics/Device/GFXDevice.h"
#include "Engine/Graphics/Device/GFXContext.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Graphics/RenderConsts.h"
#include "Engine/Resource/ResourceMgr.h"
#include "Engine/Graphics/StandardVertDecl.h"
#include "PostFXSys.h"


namespace usg {

static const DescriptorDeclaration g_descriptorDecl[] =
{
	DESCRIPTOR_ELEMENT(0,		DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, SHADER_FLAG_PIXEL),
	DESCRIPTOR_END()
};

static const DescriptorDeclaration g_postDepthDescriptorDecl[] =
{
	DESCRIPTOR_ELEMENT(14,		DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, SHADER_FLAG_PIXEL),
	DESCRIPTOR_END()
};


PostFXSys::PostFXSys()
{
	m_uPostEffects = 0;
	m_pDepthTarget = NULL;
}

PostFXSys::~PostFXSys()
{

}

const GFXBounds PostFXSys::GetBounds() const
{
	GFXBounds r;
	r.x = 0;
	r.y = 0;
	r.width = m_uTargetWidth;
	r.height = m_uTargetHeight;
	return r;
}

void PostFXSys::Init(GFXDevice* pDevice, uint32 uWidth, uint32 uHeight, uint32 uInitFlags)
{
	// TODO: Support various render targets
	
	m_uTargetWidth = uWidth;
	m_uTargetHeight = uHeight;


	// Shared vertex data
	PositionVertex verts[4] =
	{
		{ -1.f,  1.f,  0.5f }, // 0 - TL
		{  1.f,  1.f,  0.5f }, // 1 - TR
		{ -1.f,  -1.f, 0.5f }, // 2 - BL
		{  1.f,  -1.f, 0.5f }, // 3 - BR
	};

	uint8 iIndices[6] = 
	{
		2, 1, 0, 2, 3, 1, 
	};

	m_uEffectsSupported = uInitFlags;
	m_uEffectsEnabled = uInitFlags;
	m_fullScreenVB.Init(pDevice, verts, sizeof(PositionVertex), 4, "FullScreenVB");
	m_fullScreenIB.Init(pDevice, iIndices, 6, PT_TRIANGLES);

	SamplerDecl pointDecl(SF_POINT, SC_CLAMP);
	PipelineStateDecl pipelineDecl;
	pipelineDecl.inputBindings[0].Init(GetVertexDeclaration(VT_POSITION));
	pipelineDecl.uInputBindingCount = 1;
	pipelineDecl.ePrimType = PT_TRIANGLES;

	DescriptorSetLayoutHndl matDescriptors = pDevice->GetDescriptorSetLayout(g_descriptorDecl);
	pipelineDecl.layout.descriptorSets[0] = matDescriptors;
	pipelineDecl.layout.uDescriptorSetCount = 1;

	pipelineDecl.pEffect = ResourceMgr::Inst()->GetEffect(pDevice, "PostCopyScreen");

	// FIXME: PostFX stuff needs fixing for the new interface
//	m_copyMat.SetFilter(pDevice, 0, pointDecl);

	m_platform.Init(this, pDevice, uInitFlags, uWidth, uHeight);

	pipelineDecl.renderPass = m_platform.GetRenderPass();
	m_copyMat.Init(pDevice, pDevice->GetPipelineState(pipelineDecl), matDescriptors);

	m_postDepthDescriptor.Init(pDevice, pDevice->GetDescriptorSetLayout(g_postDepthDescriptorDecl));
	m_postDepthDescriptor.SetImageSamplerPair(0, m_platform.GetLinearDepthTex(), pDevice->GetSampler(pointDecl));
	m_postDepthDescriptor.UpdateDescriptors(pDevice);
}

void PostFXSys::CleanUp(GFXDevice* pDevice)
{
	m_postDepthDescriptor.CleanUp(pDevice);
	m_copyMat.Cleanup(pDevice);
	m_fullScreenIB.CleanUp(pDevice);
	m_fullScreenVB.CleanUp(pDevice);
	m_platform.CleanUp(pDevice);
}

void PostFXSys::EnableEffects(GFXDevice* pDevice, uint32 uEffectFlags)
{
	bool bSupport = (m_uEffectsSupported & uEffectFlags) == uEffectFlags;
	ASSERT(bSupport);
	if (bSupport && m_uEffectsEnabled != uEffectFlags)
	{
		m_platform.EnableEffects(pDevice, uEffectFlags);
		m_uEffectsEnabled = uEffectFlags;
	}
}

void PostFXSys::Resize(GFXDevice* pDevice, uint32 uWidth, uint32 uHeight)
{
	m_platform.Resize(pDevice, uWidth, uHeight);
	m_uTargetWidth = uWidth;
	m_uTargetHeight = uHeight; 
	m_postDepthDescriptor.UpdateDescriptors(pDevice);
}

RenderTarget* PostFXSys::BeginScene(GFXContext* pContext, uint32 uTransferFlags)
{
	m_pActiveScene = NULL;

	RenderTarget* pTarget = m_platform.GetInitialRT();
	
	pContext->SetRenderTarget(pTarget);
	pContext->DisableScissor();
	if(uTransferFlags & TRANSFER_FLAGS_CLEAR)
	{
		pContext->ClearRenderTarget(RenderTarget::CLEAR_FLAG_DS|RenderTarget::CLEAR_FLAG_COLOR_0|RenderTarget::CLEAR_FLAG_COLOR_1 );
	}
	
	return pTarget;
}

void PostFXSys::SetPostDepthDescriptors(GFXContext* pCtxt)
{
	pCtxt->SetDescriptorSet(&m_postDepthDescriptor, 4);
}

void PostFXSys::SetSkyTexture(GFXDevice* pDevice, const TextureHndl& hndl)
{
	m_platform.SetSkyTexture(pDevice, hndl);
}


void PostFXSys::UpdateRTSize(GFXDevice* pDevice, Display* pDisplay)
{
	m_platform.UpdateRTSize(pDevice, pDisplay);
}

void PostFXSys::EndScene()
{
	m_pActiveScene = NULL;
}

RenderTarget* PostFXSys::GetFinalRT()
{
	return m_platform.GetFinalRT();
}


PostEffect* PostFXSys::GetEffect(uint32 uEffectId)
{
	ASSERT(uEffectId < m_uPostEffects);
	return m_pPostEffects[uEffectId];
}

void PostFXSys::RegisterEffect(PostEffect* pEffect)
{
	ASSERT(m_uPostEffects < MAX_POST_EFFECTS);
	m_pPostEffects[m_uPostEffects] = pEffect;
	m_uPostEffects++;
}


void PostFXSys::DrawFullScreenQuad(GFXContext* pContext) const
{
	pContext->SetVertexBuffer(&m_fullScreenVB);
	pContext->DrawIndexed(&m_fullScreenIB);
}


void PostFXSys::Copy(GFXContext* pContext, RenderTarget* pSrc, RenderTarget* pDst)
{
	ASSERT(false);
#if 0
	pContext->SetRenderTarget(pDst);
	m_copyMat.SetTexture( 0, pSrc->GetColorTexture() );
	m_copyMat.Apply(pContext);
	DrawFullScreenQuad(pContext);
#endif
}

}

