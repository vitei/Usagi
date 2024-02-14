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
#include "Engine/Scene/Model/ModelInstanceRenderer.h"
 

namespace usg {


void Model::RenderMesh::RenderPassChanged(GFXDevice* pDevice, uint32 uContextId, const RenderPassHndl &renderPass, const SceneRenderPasses& passes)
{
	if (passes.IsRenderPassDeferred(*this))
	{
		m_pipelineState = pDevice->GetPipelineState(renderPass, GetPipelineState(ModelResource::Mesh::RS_DEFERRED));
		SetVertexBuffer(1, &m_pMeshResource->renderSets[ModelResource::Mesh::RS_DEFERRED].singleVerts);
	}
	else if (passes.IsRenderPassTranslucent(*this))
	{
		m_pipelineState = pDevice->GetPipelineState(renderPass, GetPipelineState(ModelResource::Mesh::RS_TRANSPARENT));
		SetVertexBuffer(1, &m_pMeshResource->renderSets[ModelResource::Mesh::RS_TRANSPARENT].singleVerts);
	}
	else
	{
		m_pipelineState = pDevice->GetPipelineState(renderPass, GetPipelineState(ModelResource::Mesh::RS_DEFAULT));
		SetVertexBuffer(1, &m_pMeshResource->renderSets[ModelResource::Mesh::RS_DEFAULT].singleVerts);
	}
}
	
Model::RenderMesh::RenderMesh() : RenderNodeEx()
{
	m_uOverrides = 0;
	m_uReqOverrides = 0;
	m_bCanHaveShadow = false;
	m_pMeshResource = nullptr;
	m_pBone = nullptr;
	m_bInstanced = false;
	m_bDepth = false;
	m_uInstanceId = USG_INVALID_ID64;
	for (uint32 i = 0; i < OVERRIDE_COUNT; i++)
	{
		m_pOverridesConstants[i] = NULL;
	}
	m_blendColor = usg::Color::White;
}


Model::RenderMesh::~RenderMesh()
{

}

void Model::RenderMesh::Init(GFXDevice* pDevice, Scene* pScene, const ModelResource::Mesh* pMesh, const Model* pModel, bool bDepth, bool bInstanced)
{
	if (bInstanced && pMesh->primitive.eSkinningMode != exchange::SkinningType_NO_SKINNING)
	{
		bInstanced = false;

	}
	m_bInstanced = bInstanced;
	m_bDepth = bDepth;
	m_pMeshResource = pMesh;
	const char* pszName = pModel->GetResource()->GetName().c_str();
	SetVertexBuffer(0, &pMesh->vertexBuffer);
	SetIndexBuffer(&pMesh->primitive.indexBuffer);

	m_bInstanced = bInstanced;
	if (bInstanced)
	{
		usg::string matName = pMesh->matName;
		if (bDepth)
		{
			matName += "_depth";
		}
		uint64 uModelCRC = utl::CRC32(pModel->GetName().c_str());
		uint64 uMeshCRC = utl::CRC32(matName.c_str());

		m_uInstanceId = uModelCRC << uint64(32) | uMeshCRC;

		m_pBone = pModel->GetSkeleton().GetBone(pMesh->primitive.uRootIndex);
	}

	SetAnimated(pMesh->primitive.eSkinningMode != exchange::SkinningType_NO_SKINNING);

	//	SetMaterial(&pMesh->material);
	m_name = pszName;
	str::RemovePath(m_name);
	str::TruncateExtension(m_name);
	m_name += " (";
	m_name += pMesh->matName;
	m_name += ")";
	m_uLod = pMesh->uLodIndex;
	m_bCanHaveShadow = pMesh->layer < RenderLayer::LAYER_TRANSLUCENT;

	if (bDepth)
	{
		SetLayer(usg::RenderLayer::LAYER_TRANSLUCENT);
		SetPriority(0);
	}
	else
	{
		SetLayer(pMesh->layer);
		SetPriority(pMesh->priority);
	}

	// FIXME: We shouldn't be creating this mesh at all
	if (bDepth && !pMesh->renderSets[ModelResource::Mesh::RS_DEPTH].pipeline.pEffect)
	{
		return;
	}

	RenderPassHndl renderPass = pScene->GetRenderPasses(0).GetRenderPass(*this);

	if (bDepth)
	{
		// FIXME: This is only valid for shadow render passes, need another pipeline for scene pre depth passes
		renderPass = pScene->GetShadowRenderPass();

		m_pipelineState = pDevice->GetPipelineState(renderPass, GetPipelineState(ModelResource::Mesh::RS_DEPTH));
		SetVertexBuffer(1, &pMesh->renderSets[ModelResource::Mesh::RS_DEPTH].singleVerts);
	}
	else
	{
		if (pScene->GetRenderPasses(0).IsRenderPassDeferred(*this))
		{
			m_pipelineState = pDevice->GetPipelineState(renderPass, GetPipelineState(ModelResource::Mesh::RS_DEFERRED));
			SetVertexBuffer(1, &pMesh->renderSets[ModelResource::Mesh::RS_DEFERRED].singleVerts);
		}
		else if (pScene->GetRenderPasses(0).IsRenderPassTranslucent(*this))
		{
			m_pipelineState = pDevice->GetPipelineState(renderPass, GetPipelineState(ModelResource::Mesh::RS_TRANSPARENT));
			SetVertexBuffer(1, &pMesh->renderSets[ModelResource::Mesh::RS_TRANSPARENT].singleVerts);
		}
		else
		{
			m_pipelineState = pDevice->GetPipelineState(renderPass, GetPipelineState(ModelResource::Mesh::RS_DEFAULT));
			SetVertexBuffer(1, &pMesh->renderSets[ModelResource::Mesh::RS_DEFAULT].singleVerts);
		}
	}

	// FIXME: Single verts for omni depth!

	const PipelineStateDecl& omniDecl = GetPipelineState(ModelResource::Mesh::RS_OMNI_DEPTH);
	if (omniDecl.pEffect)
	{
		m_omniDepthPipelineState = pDevice->GetPipelineState(pScene->GetShadowRenderPass(), omniDecl);
	}

	m_descriptorSet.Init(pDevice, pMesh->defaultPipelineDescLayout); 

	// FIXME: Get the index from the custom fx declaration

	uint32 uFirstValid = 0;
	for (uint32 i = 0; i < ModelResource::Mesh::RS_COUNT; i++)
	{
		if (pMesh->renderSets[i].pipeline.pEffect)
		{
			uFirstValid = i;
			break;
		}
	}

	m_descriptorSet.SetConstantSetAtBinding(SHADER_CONSTANT_MATERIAL_1, pMesh->renderSets[uFirstValid].effectRuntime.GetConstantSet(1), 0, SHADER_FLAG_PIXEL);
	m_descriptorSet.SetConstantSetAtBinding(SHADER_CONSTANT_MATERIAL, pMesh->renderSets[uFirstValid].effectRuntime.GetConstantSet(0), 0, SHADER_FLAG_VERTEX);

	switch (pMesh->primitive.eSkinningMode)
	{
	case usg::exchange::SkinningType_NO_SKINNING:
		m_descriptorSet.SetConstantSetAtBinding(SHADER_CONSTANT_CUSTOM_0, pModel->GetSkeleton().GetBone(pMesh->primitive.uRootIndex)->GetConstantSet(), 0, SHADER_FLAG_VERTEX);
		break;
	case usg::exchange::SkinningType_RIGID_SKINNING:
		m_descriptorSet.SetConstantSetAtBinding(SHADER_CONSTANT_CUSTOM_2, &pModel->GetRigidBones(), 0, SHADER_FLAG_VERTEX);
		break;
	case usg::exchange::SkinningType_SMOOTH_SKINNING:
		m_descriptorSet.SetConstantSetAtBinding(SHADER_CONSTANT_CUSTOM_2, &pModel->GetSkinnedBones(), 0, SHADER_FLAG_VERTEX);
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

	SetMaterialCmpVal(m_pipelineState, pMesh->pTextures[0].get());
	m_descriptorSet.UpdateDescriptors(pDevice);
}



InstancedRenderer* Model::RenderMesh::CreateInstanceRenderer(GFXDevice* pDevice, Scene* pScene)
{
	ModelInstanceRenderer* pInstance = vnew(ALLOC_OBJECT)ModelInstanceRenderer;
	pInstance->Init(pDevice, pScene, m_uInstanceId, m_pMeshResource, m_bDepth);

	return pInstance;
}


const PipelineStateDecl& Model::RenderMesh::GetPipelineState(ModelResource::Mesh::ERenderState eRenderState)
{
	if (m_bInstanced)
	{
		return m_pMeshResource->renderSets[eRenderState].instancedPipeline;
	}
	else
	{
		return m_pMeshResource->renderSets[eRenderState].pipeline;
	}
}

void Model::RenderMesh::Cleanup(GFXDevice* pDevice)
{
	for (uint32 i = 0; i < OVERRIDE_COUNT; i++)
	{
		if (m_pOverridesConstants[i])
		{
			m_pOverridesConstants[i]->Cleanup(pDevice);
		}
	}
	Inherited::Cleanup(pDevice);
}


usg::Matrix4x4 Model::RenderMesh::GetInstanceTransform() const
{
	if (m_pBone)
	{
		return m_pBone->GetWorldMatrix();
	}
	ASSERT(false);
	return usg::Matrix4x4::Identity();
}

void Model::RenderMesh::SetRenderMaskWithShadowCheck(uint32 uMask)
{
	if (!m_bCanHaveShadow)
	{
		uMask &= ~RenderMask::RENDER_MASK_SHADOW_CAST;
	}
	SetRenderMaskIncShadow(uMask);
}

bool Model::RenderMesh::Draw(GFXContext* pContext, RenderContext& renderContext)
{
	pContext->BeginGPUTag(m_name.c_str(), Color::Blue);

	// FIXME: Cleaner system?
	switch (renderContext.eRenderPass)
	{
	case RenderNode::RENDER_PASS_DEFERRED:
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
		
		m_uOverrides |= m_uReqOverrides;
	}

	if (m_uOverrides)
	{
		// FIXME: Use the customFX runtime
		m_pOverridesConstants[0]->UpdateData(pDevice);
		if(m_pOverridesConstants[1])
			m_pOverridesConstants[1]->UpdateData(pDevice);
	}
	m_descriptorSet.UpdateDescriptors(pDevice);

}

bool Model::RenderMesh::SetScale(float fScale, CustomEffectRuntime& customFX)
{
	ASSERT(fScale<=1.0f);	// This is a nasty fudge
	customFX.SetVariable("vScaling0", fScale, 0);
	customFX.SetVariable("fScale", fScale, 0);
	return true;
}




}

