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
#include "FilmGrain.h"

namespace usg {

struct FilmGrainConstants
{
	Vector2f	vResolution;
	float		fTime;
};


static const ShaderConstantDecl g_filmGrainConstantDef[] = 
{
	SHADER_CONSTANT_ELEMENT(FilmGrainConstants,	vResolution,	CT_VECTOR_2, 1 ),
	SHADER_CONSTANT_ELEMENT(FilmGrainConstants,	fTime,			CT_FLOAT, 1 ),
	SHADER_CONSTANT_END()
};

static const DescriptorDeclaration g_descriptorDecl[] =
{
	DESCRIPTOR_ELEMENT(0,						 DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, SHADER_FLAG_PIXEL),
	DESCRIPTOR_ELEMENT(1,						 DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, SHADER_FLAG_PIXEL),
	DESCRIPTOR_ELEMENT(SHADER_CONSTANT_MATERIAL, DESCRIPTOR_TYPE_CONSTANT_BUFFER, 1, SHADER_FLAG_VS_PS),
	DESCRIPTOR_END()
};


FilmGrain::FilmGrain():
	PostEffect(),
	m_pSys(nullptr),
	m_pDestTarget(nullptr)
{
	SetRenderMask(RENDER_MASK_POST_EFFECT);
	SetLayer(LAYER_POST_PROCESS);
	SetPriority(127);
}


FilmGrain::~FilmGrain()
{

}


void FilmGrain::Update(Scene* pScene, float fElapsed)
{
	FilmGrainConstants* pConsts = m_constantSet.Lock<FilmGrainConstants>();
	pConsts->fTime += fElapsed;
	m_constantSet.Unlock();
}

void FilmGrain::UpdateBuffer(usg::GFXDevice* pDevice)
{
	m_constantSet.UpdateData(pDevice); 
	m_material.UpdateDescriptors(pDevice); 
}

void FilmGrain::Init(GFXDevice* pDevice, ResourceMgr* pResource, PostFXSys* pSys, RenderTarget* pDst)
{
	m_pSys = pSys;
	m_pDestTarget = pDst;

	SamplerDecl pointDecl(SF_POINT, SC_CLAMP);
	SamplerDecl linearDecl(SF_LINEAR, SC_WRAP);

	PipelineStateDecl pipelineDecl;
	pipelineDecl.inputBindings[0].Init(usg::GetVertexDeclaration(usg::VT_POSITION));
	pipelineDecl.uInputBindingCount = 1;
	pipelineDecl.ePrimType = PT_TRIANGLES;
	pipelineDecl.pEffect = pResource->GetEffect(pDevice, "PostProcess.FilmNoise");
	pipelineDecl.alphaState.SetColor0Only();

	usg::DescriptorSetLayoutHndl matDescriptors = pDevice->GetDescriptorSetLayout(g_descriptorDecl);
	
	m_sampler = pDevice->GetSampler(pointDecl);
	usg::SamplerHndl sampLin = pDevice->GetSampler(linearDecl);

	pipelineDecl.layout.descriptorSets[0] = pDevice->GetDescriptorSetLayout(SceneConsts::g_globalDescriptorDecl);
	pipelineDecl.layout.descriptorSets[1] = matDescriptors;
	pipelineDecl.layout.uDescriptorSetCount = 2;
	pipelineDecl.rasterizerState.eCullFace = CULL_FACE_NONE;

	m_material.Init(pDevice, pDevice->GetPipelineState(pDst->GetRenderPass(), pipelineDecl), matDescriptors);

	m_constantSet.Init(pDevice, g_filmGrainConstantDef);
	
	usg::ResourceMgr* pRes = usg::ResourceMgr::Inst();
	usg::TextureHndl vol = pRes->GetTexture(pDevice, "FilmNoise");
	m_material.SetTexture(1, vol, sampLin);
	m_material.SetConstantSet(SHADER_CONSTANT_MATERIAL, &m_constantSet);

	FilmGrainConstants* pConsts = m_constantSet.Lock<FilmGrainConstants>();

	pConsts->vResolution.x = (float)pSys->GetFinalTargetWidth();
	pConsts->vResolution.y = (float)pSys->GetFinalTargetHeight();
	pConsts->fTime = 0.0f;

	m_constantSet.Unlock();
	m_constantSet.UpdateData(pDevice);
}

void FilmGrain::Cleanup(GFXDevice* pDevice)
{
	m_constantSet.Cleanup(pDevice);
	m_material.Cleanup(pDevice);
}


void FilmGrain::SetDestTarget(GFXDevice* pDevice, RenderTarget* pDst)
{
	if (m_pDestTarget != pDst)
	{
		m_pDestTarget = pDst;
		PipelineStateDecl decl;
		RenderPassHndl hndlTmp;
		
		FilmGrainConstants* pConsts = m_constantSet.Lock<FilmGrainConstants>();
		pConsts->vResolution.x = (float)pDst->GetWidth();
		pConsts->vResolution.y = (float)pDst->GetHeight();
		m_constantSet.Unlock();

		pDevice->GetPipelineDeclaration(m_material.GetPipelineStateHndl(), decl, hndlTmp);
		m_material.SetPipelineState(pDevice->GetPipelineState(pDst->GetRenderPass(), decl));
	}
}
 
void FilmGrain::SetSourceTarget(GFXDevice* pDevice, RenderTarget* pTarget)
{
	m_material.SetTexture(0, pTarget->GetColorTexture(), m_sampler);
	m_material.UpdateDescriptors(pDevice);
}

void FilmGrain::Resize(GFXDevice* pDevice, uint32 uWidth, uint32 uHeight)
{
	FilmGrainConstants* pConsts = m_constantSet.Lock<FilmGrainConstants>();
	pConsts->vResolution.x = (float)uWidth;
	pConsts->vResolution.y = (float)uHeight;
	m_constantSet.Unlock(); 
	// The internal texture info has changed
	m_material.UpdateDescriptors(pDevice);  
} 
   
bool FilmGrain::Draw(GFXContext* pContext, RenderContext& renderContext)
{
	if (!GetEnabled())
		return false;

	pContext->BeginGPUTag("FilmGrain", Color::Green);

	pContext->SetRenderTarget(m_pDestTarget);
	m_material.Apply(pContext);
	m_pSys->DrawFullScreenQuad(pContext);

	pContext->EndGPUTag();

	return true;
}

}
