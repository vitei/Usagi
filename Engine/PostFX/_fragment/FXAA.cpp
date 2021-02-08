/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Maths/Vector2f.h"
#include "Engine/PostFX/PostFXSys.h"
#include "Engine/Graphics/GFX.h"
#include "Engine/Graphics/Device/GFXDevice.h"
#include "Engine/Resource/ResourceMgr.h"
#include "Engine/Graphics/StandardVertDecl.h"
#include "Engine/Core/stl/vector.h"
#include "Engine/Scene/SceneConstantSets.h"
#include "Engine/Graphics/Device/GFXContext.h"
#include "FXAA.h"

namespace usg {

struct FXAAConstants
{
	Vector4f	vRcpFrameOpt;
	Vector4f	vRcpFrameOpt2;
};


static const ShaderConstantDecl g_fxaaConstantDef[] = 
{
	SHADER_CONSTANT_ELEMENT( FXAAConstants, vRcpFrameOpt,	CT_VECTOR_4, 1 ),
	SHADER_CONSTANT_ELEMENT( FXAAConstants, vRcpFrameOpt2,	CT_VECTOR_4, 1 ),
	SHADER_CONSTANT_END()
};

static const DescriptorDeclaration g_descriptorDecl[] =
{
	DESCRIPTOR_ELEMENT(0,						 DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, SHADER_FLAG_PIXEL),
	DESCRIPTOR_ELEMENT(SHADER_CONSTANT_MATERIAL, DESCRIPTOR_TYPE_CONSTANT_BUFFER, 1, SHADER_FLAG_VS_PS),
	DESCRIPTOR_END()
};


FXAA::FXAA():
PostEffect()
{
	SetRenderMask(RENDER_MASK_POST_EFFECT);
	SetLayer(LAYER_POST_PROCESS);
	SetPriority(29);
}


FXAA::~FXAA()
{

}

void FXAA::Init(GFXDevice* pDevice, ResourceMgr* pResource, PostFXSys* pSys, RenderTarget* pDst)
{
	m_pSys = pSys;
	m_pDestTarget = pDst;

	SamplerDecl pointDecl(SAMP_FILTER_LINEAR, SAMP_WRAP_CLAMP);

	PipelineStateDecl pipelineDecl;
	pipelineDecl.inputBindings[0].Init(usg::GetVertexDeclaration(usg::VT_POSITION));
	pipelineDecl.uInputBindingCount = 1;
	pipelineDecl.ePrimType = PT_TRIANGLES;
	pipelineDecl.pEffect = pResource->GetEffect(pDevice, "PostProcess.FXAA");

	usg::DescriptorSetLayoutHndl matDescriptors = pDevice->GetDescriptorSetLayout(g_descriptorDecl);
	
	m_sampler = pDevice->GetSampler(pointDecl);


	pipelineDecl.layout.descriptorSets[0] = pDevice->GetDescriptorSetLayout(SceneConsts::g_globalDescriptorDecl);
	pipelineDecl.layout.descriptorSets[1] = matDescriptors;
	pipelineDecl.layout.uDescriptorSetCount = 2;
	pipelineDecl.rasterizerState.eCullFace = CULL_FACE_NONE;

	m_material.Init(pDevice, pDevice->GetPipelineState(pDst->GetRenderPass(), pipelineDecl), matDescriptors);

	m_constantSet.Init(pDevice, g_fxaaConstantDef);
	
	m_material.SetConstantSet(SHADER_CONSTANT_MATERIAL, &m_constantSet);

	FXAAConstants* pConsts = m_constantSet.Lock<FXAAConstants>();

	float fN = 0.5f; // 0.33f is sharped

	pConsts->vRcpFrameOpt.x = -fN/pSys->GetFinalTargetWidth();
	pConsts->vRcpFrameOpt.y = -fN/pSys->GetFinalTargetHeight();
	pConsts->vRcpFrameOpt.z = fN/pSys->GetFinalTargetWidth();
	pConsts->vRcpFrameOpt.w = fN/pSys->GetFinalTargetHeight();

	pConsts->vRcpFrameOpt2.x = -2.0f/pSys->GetFinalTargetWidth();
	pConsts->vRcpFrameOpt2.y = -2.0f/pSys->GetFinalTargetHeight();
	pConsts->vRcpFrameOpt2.z = 2.0f/pSys->GetFinalTargetWidth();
	pConsts->vRcpFrameOpt2.w = 2.0f/pSys->GetFinalTargetHeight();

	m_constantSet.Unlock();
	m_constantSet.UpdateData(pDevice);
}

void FXAA::Cleanup(GFXDevice* pDevice)
{
	m_constantSet.Cleanup(pDevice);
	m_material.Cleanup(pDevice);
}


void FXAA::SetDestTarget(GFXDevice* pDevice, RenderTarget* pDst)
{
	if (m_pDestTarget != pDst)
	{
		m_pDestTarget = pDst;
		PipelineStateDecl decl;
		RenderPassHndl hndlTmp;
		pDevice->GetPipelineDeclaration(m_material.GetPipelineStateHndl(), decl, hndlTmp);
		m_material.SetPipelineState(pDevice->GetPipelineState(pDst->GetRenderPass(), decl));
	}
}

void FXAA::SetSourceTarget(GFXDevice* pDevice, RenderTarget* pTarget)
{
	m_material.SetTexture(0, pTarget->GetColorTexture(), m_sampler);
	m_material.UpdateDescriptors(pDevice);
}

void FXAA::Resize(GFXDevice* pDevice, uint32 uWidth, uint32 uHeight)
{
	// The internal texture info has changed
	m_material.UpdateDescriptors(pDevice);
}

bool FXAA::Draw(GFXContext* pContext, RenderContext& renderContext)
{
	if (!GetEnabled())
		return false;

	pContext->BeginGPUTag("FXAA", Color::Green);

	pContext->SetRenderTarget(m_pDestTarget);
	m_material.Apply(pContext);
	m_pSys->DrawFullScreenQuad(pContext);

	pContext->EndGPUTag();

	return true;
}

}
