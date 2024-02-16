/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Rendering of multiple models with a single draw call
*****************************************************************************/
#pragma once
#include "Engine/Common/Common.h"
#include "Engine/Graphics/Device/GFXContext.h"
#include "Engine/Scene/Model/Shape.pb.h"
#include "ModelInstanceRenderer.h"
 

namespace usg {


	void ModelInstanceSet::Init(ModelInstanceRenderer* pOwner, uint32 uStartIndex, uint32 uCount)
	{
		m_pOwner = pOwner;
		m_uStartIndex = uStartIndex;
		m_uCount = uCount;
	}
	
	bool ModelInstanceSet::Draw(GFXContext* pContext, RenderContext& renderContext)
	{
		m_pOwner->GetDrawer().InstanceDraw(pContext, renderContext, m_pOwner->GetInstanceBuffer(), m_uStartIndex, m_uCount);
		return true;
	}

	ModelInstanceRenderer::ModelInstanceRenderer() : 
		m_groups(30, true, false)
	{

	}

	ModelInstanceRenderer::~ModelInstanceRenderer()
	{

	}

	void ModelInstanceRenderer::Init(GFXDevice* pDevice, Scene* pScene, const uint64 uInstanceId, const ModelResource::Mesh* pMesh, bool bDepth)
	{
		ASSERT(pMesh->primitive.eSkinningMode == exchange::SkinningType_NO_SKINNING);

		m_mesh.Init(pDevice, pScene, pMesh, nullptr, bDepth);
	}
		
	uint64 ModelInstanceRenderer::GetInstanceId()
	{
		return m_uInstanceId;
	}

	void ModelInstanceRenderer::Draw(GFXContext* pContext, RenderNode::RenderContext& renderContext, uint32 uDrawId)
	{		
		pContext->SetVertexBuffer(&m_instanceBuffer, m_mesh.GetVertexBufferCount());
		m_mesh.Draw(pContext, renderContext);
	}


	RenderNode* ModelInstanceRenderer::EndBatch()
	{
		memsize uCount = m_instanceData.size() - m_uStartIndex;
		if(uCount > 0)
		{
			usg::ModelInstanceSet* pSet = m_groups.Alloc();
			pSet->Init(this, (uint32)m_uStartIndex, (uint32)uCount);

			m_uStartIndex = m_instanceData.size();

			return pSet;
		}
		return nullptr;
	}

	void ModelInstanceRenderer::AddNode(RenderNode* pNode)
	{
		Model::InstanceMesh* pMesh = (Model::InstanceMesh*)(pNode);
		m_instanceData.push_back( pMesh->GetInstanceTransform() );		
	
	}

	void ModelInstanceRenderer::PreDraw(GFXDevice* pDevice)
	{
		if (m_instanceData.size() > 0)
		{
			if (m_instanceData.size() > m_instanceBuffer.GetCount())
			{
				m_instanceBuffer.Cleanup(pDevice);
				m_instanceBuffer.Init(pDevice, nullptr, sizeof(Matrix4x3), (uint32)Math::Roundup(m_instanceData.size(), 32), "ModelInstance", GPU_USAGE_DYNAMIC);
			}
			m_instanceBuffer.SetContents(pDevice, m_instanceData.data(), (uint32)m_instanceData.size());
		}
	}


	void ModelInstanceRenderer::DrawFinished()
	{
		m_uStartIndex = 0;
		m_instanceData.clear();
		m_groups.Clear();
	}

	void ModelInstanceRenderer::Cleanup(GFXDevice* pDevice)
	{
		m_instanceBuffer.Cleanup(pDevice);
	}

}


