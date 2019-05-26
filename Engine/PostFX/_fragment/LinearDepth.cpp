/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Maths/Vector2f.h"
#include "Engine/PostFX/PostFXSys.h"
#include "Engine/Graphics/GFX.h"
#include "Engine/Graphics/Device/GFXDevice.h"
#include "Engine/Resource/ResourceMgr.h"
#include "LinearDepth.h"

namespace usg {

static const DescriptorDeclaration g_descriptorDecl[] =
{
	DESCRIPTOR_ELEMENT(0,						 DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, SHADER_FLAG_PIXEL),
	DESCRIPTOR_END()
};

LinearDepth::LinearDepth():
PostEffect()
{
	SetRenderMask(RENDER_MASK_POST_EFFECT);
	SetLayer(LAYER_SKY);
	SetPriority(0);	// Priority needs to be lower than the sky
}


LinearDepth::~LinearDepth()
{

}

void LinearDepth::Init(GFXDevice* pDevice, ResourceMgr* pResource, PostFXSys* pSys, RenderTarget* pResult)
{
	ASSERT(false);
#if 0
	m_pSys = pSys;

	SamplerDecl pointDecl(SF_POINT, SC_CLAMP);

	PipelineStateDecl pipelineDecl;
	pipelineDecl.inputBindings[0].Init(GetVertexDeclaration(VT_POSITION));
	pipelineDecl.uInputBindingCount = 1;
	pipelineDecl.ePrimType = PT_TRIANGLES;
	pipelineDecl.pEffect = pResource->GetEffect(pDevice, "lineardepth");


	m_material.Init(pDevice, pDevice->GetPipelineState(pipelineDecl), pDevice->GetDescriptorSetLayout(g_descriptorDecl));
	
	m_material.SetFilter(pDevice, 0, pointDecl);

	uint32 uScrWidth	= pSys->GetFinalTargetWidth();
	uint32 uScrHeight	= pSys->GetFinalTargetHeight();

	// TODO: As linear may want to use a normalized R_32/R16
	m_depthBuffer.Init(pDevice, uScrWidth,	uScrWidth,	CF_R_32F, 0);// ColorBuffer::FLAG_FAST_MEM);
	m_depthRT.Init(&m_depthBuffer);

	// FIXME: Set in use only when valid and then remove and the end of the scene
	m_depthRT.SetInUse(true);
#endif
}

void LinearDepth::CleanUp(GFXDevice* pDevice)
{

}

bool LinearDepth::Draw(GFXContext* pContext, RenderContext& renderContext)
{
	ASSERT(false);
#if 0
	pContext->BeginGPUTag("LinearDepth");
	
	pContext->SetRenderTarget(&m_depthRT);
	RenderTarget* pSceneRT = pPostFXSys->GetActiveRT();

	m_material.SetTexture( 0, pSceneRT->GetDepthTexture());
	m_material.Apply(pContext);
	m_pSys->DrawFullScreenQuad(pContext);

	// Restore the previous render target
	pContext->SetRenderTarget(m_pSys->GetActiveRT());

	pContext->EndGPUTag();
#endif

	return true;
}


}

