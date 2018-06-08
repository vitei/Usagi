/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Simple model from binary
*****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Scene/Model/ModelRenderNodes.h"
#include "Engine/Scene/Model/Skeleton.h"
#include "Engine/Scene/Model/Bone.h"
#include "Engine/Graphics/Device/GFXContext.h"
#include "Engine/Graphics/Device/GFXDevice.h"
#include "Engine/PostFX/PostFXSys.h"
#include "Engine/Scene/Scene.h"
 

namespace usg {

	
Model::RenderMesh::RenderMesh() : RenderNodeEx()
{
	m_uOverrides = 0;
	m_uReqOverrides = 0;
	m_bCanHaveShadow = false;
	for (uint32 i = 0; i < OVERRIDE_COUNT; i++)
	{
		m_pOverridesConstants[i] = NULL;
	}
	m_blendColor = usg::Color::White;
}

Model::RenderMesh::~RenderMesh()
{

}

void Model::RenderMesh::Init(GFXDevice* pDevice, Scene* pScene, const ModelResource::Mesh* pMesh, const Model* pModel, bool bDepth)
{
	const char* pszName = pModel->GetResource()->GetName().CStr();
	for (uint32 i = 0; i < ARRAY_SIZE(pMesh->vertexBuffer); ++i) {
		SetVertexBuffer(i, &pMesh->vertexBuffer[i]);
	}
	SetIndexBuffer(&pMesh->primitive.indexBuffer);

	//	SetMaterial(&pMesh->material);
	m_pszName = pszName;
	m_uLod = pMesh->uLodIndex;
	m_bCanHaveShadow = pMesh->layer < RenderNode::LAYER_TRANSLUCENT;

	if (bDepth)
	{
		SetLayer(usg::RenderNode::LAYER_TRANSLUCENT);
		SetPriority(0);
	}
	else
	{
		SetLayer(pMesh->layer);
		SetPriority(pMesh->priority);
	}

	RenderPassHndl renderPass = pScene->GetRenderPasses(0).GetRenderPass(*this);

	if (bDepth)
	{
		m_deferredPipelineState = pMesh->GetPipeline(renderPass).depthPassPipeline;
		m_pipelineState = pMesh->GetPipeline(renderPass).depthPassPipeline;
	}
	else
	{
		m_deferredPipelineState = pMesh->GetPipeline(renderPass).deferredPipeline;
		m_pipelineState = pMesh->GetPipeline(renderPass).defaultPipeline;
	}
	m_omniDepthPipelineState = pMesh->GetPipeline(renderPass).omniDepthPassPipeline;

	m_descriptorSet.Init(pDevice, pMesh->defaultPipelineDescLayout);

	// FIXME: Get the index from the custom fx declaration
	m_descriptorSet.SetConstantSetAtBinding(SHADER_CONSTANT_MATERIAL_1, pMesh->effectRuntime.GetConstantSet(1), 0, SHADER_FLAG_PIXEL);
	m_descriptorSet.SetConstantSetAtBinding(SHADER_CONSTANT_MATERIAL, pMesh->effectRuntime.GetConstantSet(0), 0, SHADER_FLAG_VERTEX);

	switch (pMesh->primitive.eSkinningMode)
	{
	case usg::exchange::SkinningType_NO_SKINNING:
		m_descriptorSet.SetConstantSetAtBinding(SHADER_CONSTANT_INSTANCE, pModel->GetSkeleton().GetBone(pMesh->primitive.uRootIndex)->GetConstantSet(), 0, SHADER_FLAG_VERTEX);
		break;
	case usg::exchange::SkinningType_RIGID_SKINNING:
		m_descriptorSet.SetConstantSetAtBinding(SHADER_CONSTANT_BONES, &pModel->GetRigidBones(), 0, SHADER_FLAG_VERTEX);
		break;
	case usg::exchange::SkinningType_SMOOTH_SKINNING:
		m_descriptorSet.SetConstantSetAtBinding(SHADER_CONSTANT_BONES, &pModel->GetSkinnedBones(), 0, SHADER_FLAG_VERTEX);
		break;
	default:
		ASSERT(false);
	}

	for (uint32 i = 0; i < ModelResource::Mesh::MAX_UV_STAGES; i++)
	{
		if (pMesh->pTextures[i])
		{
			m_descriptorSet.SetImageSamplerPairAtBinding(i, pMesh->pTextures[i], pMesh->samplers[i]);
		}
	}

	SetMaterialCmpVal(pMesh->GetPipeline(renderPass).defaultPipeline, pMesh->pTextures[0].get());
	m_descriptorSet.UpdateDescriptors(pDevice);
}

void Model::RenderMesh::CleanUp(GFXDevice* pDevice)
{
	for (uint32 i = 0; i < OVERRIDE_COUNT; i++)
	{
		if (m_pOverridesConstants[i])
		{
			m_pOverridesConstants[i]->CleanUp(pDevice);
		}
	}
	Inherited::CleanUp(pDevice);
}

void Model::RenderMesh::SetRenderMaskWithShadowCheck(uint32 uMask)
{
	if (!m_bCanHaveShadow)
	{
		uMask &= ~RenderNode::RENDER_MASK_SHADOW_CAST;
	}
	SetRenderMaskIncShadow(uMask);
}

bool Model::RenderMesh::Draw(GFXContext* pContext, RenderContext& renderContext)
{
	pContext->BeginGPUTag(m_pszName);

	// FIXME: Cleaner system?
	switch (renderContext.eRenderPass)
	{
	case RenderNode::RENDER_PASS_DEFERRED:
		pContext->SetPipelineState(m_deferredPipelineState);
		break;
	case RenderNode::RENDER_PASS_DEPTH:
	case RenderNode::RENDER_PASS_FORWARD:
		pContext->SetPipelineState(m_pipelineState);
		break;
	case RenderNode::RENDER_PASS_DEPTH_OMNI:
		pContext->SetPipelineState(m_omniDepthPipelineState);
		break;
	default:
		ASSERT(false);
	}
	pContext->SetDescriptorSet(&m_descriptorSet, 1);
	pContext->SetBlendColor(m_blendColor);
	
	for (uint32 i = 0; i < m_uVertexBuffers; ++i)
	{
		pContext->SetVertexBuffer(m_vertexBuffer[i].pBuffer, m_vertexBuffer[i].uIndex);
	}

	pContext->DrawIndexed(m_pIndexBuffer);

	pContext->EndGPUTag();

	return true;
}


void Model::RenderMesh::VisibilityUpdate(GFXDevice* pDevice, const Vector4f& vTransformOffset)
{
	// FIXME: Have the UV mappers set a dirty flag in the render mesh so we
	// don't need to iterate through this list
	ConstantSet* pSet = NULL;
	for(uint32 i=0; i<ModelResource::Mesh::MAX_UV_STAGES; i++)
	{
		if(m_uVMapper[i].IsValid())
		{
			m_uReqOverrides |= m_uVMapper[i].Update() ? OVERRIDE_MATERIAL : 0;
		}
	}

	// FIXME: These can be removed from here by moving events such as set scale to being GPU exclusive
	if (m_uReqOverrides != m_uOverrides)
	{
		if (m_uReqOverrides & OVERRIDE_MATERIAL)
		{
			m_descriptorSet.SetConstantSetAtBinding(SHADER_CONSTANT_MATERIAL, m_pOverridesConstants[0], 0, SHADER_FLAG_VERTEX);
		}
		if (m_uReqOverrides & OVERRIDE_MATERIAL_1)
		{
			m_descriptorSet.SetConstantSetAtBinding(SHADER_CONSTANT_MATERIAL_1, m_pOverridesConstants[1], 0, SHADER_FLAG_PIXEL);
		}
		
		m_descriptorSet.UpdateDescriptors(pDevice);
		m_uOverrides |= m_uReqOverrides;
	}

	if (m_uOverrides)
	{
		// FIXME: Use the customFX runtime
		m_pOverridesConstants[0]->UpdateData(pDevice);
		if(m_pOverridesConstants[1])
			m_pOverridesConstants[1]->UpdateData(pDevice);
	}


}

bool Model::RenderMesh::SetScale(float fScale, CustomEffectRuntime& customFX)
{
	ASSERT(fScale<=1.0f);	// This is a nasty fudge
	customFX.SetVariable("vScaling0", fScale, 0);
	customFX.SetVariable("fScale", fScale, 0);
	return true;
}




}

