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

	uint16 iIndices[6] = 
	{
		2, 1, 0, 2, 3, 1, 
	};

	m_uEffectsSupported = uInitFlags;
	m_uEffectsEnabled = uInitFlags;
	m_fullScreenVB.Init(pDevice, verts, sizeof(PositionVertex), 4, "FullScreenVB");
	m_fullScreenIB.Init(pDevice, iIndices, 6, PT_TRIANGLES);

	m_dummyDepth = ResourceMgr::Inst()->GetTexture(pDevice, "white_default");

	m_platform.Init(this, pDevice, uInitFlags, uWidth, uHeight);
}

void PostFXSys::CleanUp(GFXDevice* pDevice)
{
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
}

RenderTarget* PostFXSys::BeginScene(GFXContext* pContext, uint32 uTransferFlags)
{
	m_pActiveScene = NULL;

	RenderTarget* pTarget = m_platform.GetInitialRT();
	
	pContext->SetRenderTarget(pTarget);
	pContext->DisableScissor();
	if(uTransferFlags & TRANSFER_FLAGS_CLEAR)
	{
		pContext->ClearRenderTarget(RenderTarget::RT_FLAG_DS|RenderTarget::RT_FLAG_COLOR_0|RenderTarget::RT_FLAG_COLOR_1 );
	}
	
	return pTarget;
}

void PostFXSys::SetPostDepthDescriptors(GFXContext* pCtxt)
{
	m_platform.DepthWriteEnded(pCtxt, m_uEffectsEnabled);
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

