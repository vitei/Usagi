/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/PostFX/PostFXSys.h"
#include "Engine/Graphics/Device/GFXDevice.h"
#include "Engine/Resource/ResourceMgr.h"
#include "Engine/Graphics/StandardVertDecl.h"
#include "Engine/Scene/SceneConstantSets.h"
#include "Engine/Graphics/Device/GFXContext.h"
#include "Engine/Resource/CustomEffectResource.h"
#include "Engine/Layout/Global2D.h"
#include "BlitImage.h"

namespace usg {


static const DescriptorDeclaration g_descriptorDecl[] =
{
	DESCRIPTOR_ELEMENT(0, DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, SHADER_FLAG_PIXEL),
	DESCRIPTOR_END()
};



BlitImage::BlitImage()
{
	m_bCustomEffect = false;
}


BlitImage::~BlitImage()
{

}


void BlitImage::InitForDisplay(GFXDevice* pDevice, usg::ResourceMgr* pResMgr, uint32 uDisplay)
{
	usg::Display* pDisplay = pDevice->GetDisplay(uDisplay);

	ASSERT(pDisplay);
	const RenderPassHndl pass = pDisplay->GetRenderPass();
	switch (pDisplay->GetRequiredColorCorrection())
	{
		case ColorCorrection::BT2084:
		case ColorCorrection::BT709:
			// TODO: Different values for each
			Init(pDevice, pResMgr->GetEffect(pDevice, "PostProcess.LinearToHDR"), pass);
			break;
		case ColorCorrection::sRGB:
			Init(pDevice, pResMgr->GetEffect(pDevice, "PostProcess.AdjustColorSpace"), pass);
			break;
		default:
			Init(pDevice, pResMgr->GetEffect(pDevice, "PostProcess.Copy"), pass);
			break;
	}
}

void BlitImage::Init(GFXDevice* pDevice, EffectHndl effect, const RenderPassHndl& pass)
{
	PipelineStateDecl pipelineDecl;
	pipelineDecl.inputBindings[0].Init(usg::GetVertexDeclaration(usg::VT_POSITION));
	pipelineDecl.uInputBindingCount = 1;
	pipelineDecl.ePrimType = PT_TRIANGLES;
	pipelineDecl.pEffect = effect;

	if (effect->GetCustomEffect())
	{
		m_runtimeEffect.Init(pDevice, effect->GetCustomEffect());

		m_bCustomEffect = true;
	}

	pipelineDecl.alphaState.SetColor0Only();

	usg::DescriptorSetLayoutHndl matDescriptors;
	matDescriptors = pDevice->GetDescriptorSetLayout(m_bCustomEffect ? m_runtimeEffect.GetResource()->GetDescriptorDecl() : g_descriptorDecl);
	
	SamplerDecl pointDecl(SAMP_FILTER_LINEAR, SAMP_WRAP_CLAMP);
	m_sampler = pDevice->GetSampler(pointDecl);


	pipelineDecl.layout.descriptorSets[0] = pDevice->GetDescriptorSetLayout(g_sGlobalDescriptors2D	);
	pipelineDecl.layout.descriptorSets[1] = matDescriptors;
	pipelineDecl.layout.uDescriptorSetCount = 2;
	pipelineDecl.rasterizerState.eCullFace = CULL_FACE_NONE;

	m_material.Init(pDevice, pDevice->GetPipelineState(pass, pipelineDecl), matDescriptors);

	if (m_bCustomEffect)
	{
		CustomEffectResHndl effectDecl = effect->GetCustomEffect();

		m_runtimeEffect.GPUUpdate(pDevice);

		for (uint32 i = 0; i < effectDecl->GetConstantSetCount(); i++)
		{
			m_material.SetConstantSet(effectDecl->GetConstantSetBinding(i), m_runtimeEffect.GetConstantSet(i));

		}
	}

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

	m_fullScreenVB.Init(pDevice, verts, sizeof(PositionVertex), 4, "FullScreenVB");
	m_fullScreenIB.Init(pDevice, iIndices, 6, PT_TRIANGLES);

}

void BlitImage::Cleanup(GFXDevice* pDevice)
{
	m_fullScreenIB.Cleanup(pDevice);
	m_fullScreenVB.Cleanup(pDevice);
	if (m_bCustomEffect)
	{
		m_runtimeEffect.Cleanup(pDevice);
	}
	m_material.Cleanup(pDevice);
}

void BlitImage::ChangeRenderPass(GFXDevice* pDevice, const RenderPassHndl& pass)
{
	PipelineStateDecl decl;
	RenderPassHndl hndlTmp;
	pDevice->GetPipelineDeclaration(m_material.GetPipelineStateHndl(), decl, hndlTmp);
	m_material.SetPipelineState(pDevice->GetPipelineState(pass, decl));
}


void BlitImage::SetSourceTexture(GFXDevice* pDevice, const TextureHndl& tex)
{
	m_material.SetTexture(0, tex, m_sampler);
	if (m_bCustomEffect)
	{
		m_runtimeEffect.GPUUpdate(pDevice);
	}
	m_material.UpdateDescriptors(pDevice);
}


bool BlitImage::Draw(GFXContext* pContext)
{
	pContext->BeginGPUTag("BlitImage", Color::Green);

	m_material.Apply(pContext);

	pContext->SetVertexBuffer(&m_fullScreenVB);
	pContext->DrawIndexed(&m_fullScreenIB);

	pContext->EndGPUTag();

	return true;
}

}
