/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Graphics/Device/GFXDevice.h"
#include "Engine/Scene/RenderGroup.h"
#include "Engine/PostFX/PostFXSys.h"
#include "Engine/Scene/SceneConstantSets.h"
#include "Engine/Resource/CustomEffectResource.h"
#include "Engine/Graphics/Device/GFXContext.h"
#include "Engine/Scene/Common/CFXMesh.h"

namespace usg {

CFXMesh::CFXMesh()
: RenderNode()
{

}


CFXMesh::~CFXMesh()
{

}

void CFXMesh::Init(GFXDevice* pDevice, EffectHndl hndl, RenderPassHndl renderPass, usg::PipelineStateDecl& declInOut)
{
	m_effect.Init(pDevice, hndl->GetCustomEffect());

	usg::CustomEffectResHndl cfxRes = m_effect.GetResource();

	declInOut.pEffect = hndl;
	
	declInOut.layout.descriptorSets[0] = pDevice->GetDescriptorSetLayout(SceneConsts::g_globalDescriptorDecl);
	declInOut.layout.descriptorSets[1] = cfxRes->GetDescriptorLayoutHndl();
	declInOut.layout.uDescriptorSetCount = 2;

	declInOut.inputBindings[0].Init(cfxRes->GetVertexElements());
	declInOut.uInputBindingCount = 1;

	m_descriptor.Init(pDevice, cfxRes->GetDescriptorLayoutHndl());

	for(uint32 i=0; i<cfxRes->GetConstantSetCount(); i++)
	{
		m_descriptor.SetConstantSetAtBinding(cfxRes->GetConstantSetBinding(i), m_effect.GetConstantSet(i));
		m_descriptor.UpdateDescriptors(pDevice);
	}

	m_pipeline = pDevice->GetPipelineState(renderPass, declInOut);

}


void CFXMesh::GPUUpdate(usg::GFXDevice* pDevice)
{
	m_effect.GPUUpdate(pDevice);
	m_descriptor.UpdateDescriptors(pDevice);
}

void CFXMesh::Cleanup(usg::GFXDevice* pDevice)
{
	m_effect.Cleanup(pDevice);
	m_descriptor.Cleanup(pDevice);
	m_vertexBuffer.Cleanup(pDevice);
	m_indexBuffer.Cleanup(pDevice);
}

bool CFXMesh::Draw(GFXContext* pContext, RenderContext& renderContext)
{
	uint32 uIndexCount = m_indexBuffer.GetIndexCount();
	uint32 uVertexCount = m_vertexBuffer.GetCount();
	if(uIndexCount > 0 || uVertexCount > 0)
	{

		const RenderGroup* pParent = GetParent();

		pContext->BeginGPUTag("CFXMesh", Color::Blue);

		pContext->SetPipelineState(m_pipeline);
		pContext->SetDescriptorSet(&m_descriptor, 1);

		pContext->SetVertexBuffer(&m_vertexBuffer);

		if(uIndexCount > 0)
		{
			pContext->DrawIndexedEx(&m_indexBuffer, 0, uIndexCount);
		}
		else
		{
			pContext->DrawImmediate(uVertexCount);
		}

		pContext->EndGPUTag();

		return true;
	}

	return false;
}

void CFXMesh::RenderPassChanged(GFXDevice* pDevice, uint32 uContextId, const RenderPassHndl &renderPass, const SceneRenderPasses& passes)
{
	pDevice->ChangePipelineStateRenderPass(renderPass, m_pipeline);
}

}
