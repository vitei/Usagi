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

}


BlitImage::~BlitImage()
{

}

void BlitImage::Init(GFXDevice* pDevice, ResourceMgr* pResource, const RenderPassHndl& pass)
{
	PipelineStateDecl pipelineDecl;
	pipelineDecl.inputBindings[0].Init(usg::GetVertexDeclaration(usg::VT_POSITION));
	pipelineDecl.uInputBindingCount = 1;
	pipelineDecl.ePrimType = PT_TRIANGLES;
	pipelineDecl.pEffect = pResource->GetEffect(pDevice, "PostProcess.Copy");

	pipelineDecl.alphaState.SetColor0Only();

	usg::DescriptorSetLayoutHndl matDescriptors = pDevice->GetDescriptorSetLayout(g_descriptorDecl);
	
	SamplerDecl pointDecl(SF_LINEAR, SC_CLAMP);
	m_sampler = pDevice->GetSampler(pointDecl);


	pipelineDecl.layout.descriptorSets[0] = pDevice->GetDescriptorSetLayout(g_sGlobalDescriptors2D	);
	pipelineDecl.layout.descriptorSets[1] = matDescriptors;
	pipelineDecl.layout.uDescriptorSetCount = 2;
	pipelineDecl.rasterizerState.eCullFace = CULL_FACE_NONE;

	m_material.Init(pDevice, pDevice->GetPipelineState(pass, pipelineDecl), matDescriptors);

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

void BlitImage::CleanUp(GFXDevice* pDevice)
{
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
