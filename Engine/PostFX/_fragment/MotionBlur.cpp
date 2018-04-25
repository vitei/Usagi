/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#if 0
#include "Engine/Maths/Vector2f.h"
#include "Engine/PostFX/PostFXSys.h"
#include "Engine/Graphics/GFX.h"
#include "Engine/Graphics/Device/GFXDevice.h"
#include "Engine/Resource/ResourceMgr.h"
#include "MotionBlur.h"



MotionBlur::MotionBlur():
PostEffect(),
m_bFirstFrame(true)
{
	SetRenderMask(RENDER_MASK_POST_EFFECT);
	SetLayer(LAYER_POST_PROCESS);
	SetPriority(10);
}


MotionBlur::~MotionBlur()
{

}

void MotionBlur::Init(GFXDevice* pDevice, PostFXSys* pSys)
{
	m_pSys = pSys;

	m_material.Init(pDevice, ResourceMgr::Inst()->GetEffectBinding(pDevice, "motion_blur", GetStandardDeclarationId(VT_POSITION)));

	//m_material.AddConstantSet(g_blurConstantDef, 1, SHADER_FLAG_ALL);
	m_prevVelTex.Init(0, pSys->GetFinalTargetWidth(), pSys->GetFinalTargetHeight(), TF_DEFERRED_VELOCITY);
	m_prevVel.Init(pSys->GetFinalTargetWidth(), pSys->GetFinalTargetHeight(), &m_prevVelTex);
	m_prevVel.SetInUse(true);

	SamplerDecl pointDecl(SF_MIN_MAG_POINT, SC_CLAMP);
	SamplerDecl linearDecl(SF_MIN_MAG_LINEAR, SC_CLAMP);

	m_material.SetFilter(0, linearDecl);
	m_material.SetFilter(1, pointDecl);
	m_material.SetFilter(2, pointDecl);
}


bool MotionBlur::DrawPostProcess(GFXContext* pContext)
{
	pContext->BeginGPUTag("Motion blur");

	// Needs to remain in HDR memory
	RenderTarget* pSrcTex	= m_pSys->BeginTransfer(pContext, true);
	RenderTarget* pGBuffer	= m_pSys->GetGBuffer();

	m_material.SetTexture( 0, pSrcTex->GetColorTexture());
	m_material.SetTexture( 1, pGBuffer->GetColorTexture(DT_VELOCITY));
	if(m_bFirstFrame)
	{
		m_material.SetTexture( 2, pGBuffer->GetColorTexture(DT_VELOCITY));
		m_bFirstFrame = false;
	}
	else
	{
		m_material.SetTexture( 2, m_prevVel.GetColorTexture());
	}
	m_material.Apply(pContext);
	m_pSys->DrawFullScreenQuad(pContext);

	m_pSys->EndTransfer();

	// Copy the velocity information so that it is available for the subsequent frame
	m_pSys->Copy(pContext, pGBuffer, &m_prevVel, DT_VELOCITY);

	pContext->SetRenderTarget(m_pSys->GetActiveRT());

	pContext->EndGPUTag();

	return true;
}
#endif
