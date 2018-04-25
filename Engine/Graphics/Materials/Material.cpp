/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Graphics/Effects/Effect.h"
#include "Engine/Graphics/Textures/Texture.h"
#include "Engine/Graphics/GFX.h"
#include "Engine/Graphics/Device/GFXContext.h"
#include "Material.h"

namespace usg {

Material::Material()
{

}

Material::Material(GFXDevice* pDevice, PipelineStateHndl pipelineState, const DescriptorSetLayoutHndl& descriptorDecl)
{
	Init(pDevice, pipelineState, descriptorDecl);
}

Material::~Material(void)
{

}

bool Material::Init(GFXDevice* pDevice, PipelineStateHndl pipelineState, const DescriptorSetLayoutHndl& descriptorDecl)
{
	m_pipelineState = pipelineState;

	SamplerDecl samplerDecl(SF_LINEAR, SC_CLAMP);

	m_descriptorSet.Init(pDevice, descriptorDecl);


	return true;
}

bool Material::Init(GFXDevice* pDevice, const Material& copyMat)
{
	m_descriptorSet.Init(pDevice, copyMat.m_descriptorSet);

	m_pipelineState = copyMat.m_pipelineState;

	m_pipelineState = copyMat.m_pipelineState;
	SetBlendColor(copyMat.m_blendColor);
	m_name = copyMat.m_name;

	return true;
}

void Material::Cleanup(GFXDevice* pDevice)
{
	m_descriptorSet.CleanUp(pDevice);
}


void Material::SetPipelineState(PipelineStateHndl pipelineState)
{
	m_pipelineState = pipelineState;
}

void Material::SetDescriptorLayout(GFXDevice* pDevice, const DescriptorSetLayoutHndl& descriptorDecl)
{
	m_descriptorSet.Init(pDevice, descriptorDecl);
}

void Material::Apply(GFXContext* pContext) const
{
	pContext->SetPipelineState(m_pipelineState);
	pContext->SetBlendColor(m_blendColor);
	pContext->SetDescriptorSet(&m_descriptorSet, 1);	// For now hard-coding material descriptor sets to slot 1
}

//void Material::SetAlpha(AlphaStateHndl alphaHndl)
//{
//	m_renderStates.SetAlpha(alphaHndl);
//}



}
