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

	ModelInstanceRenderer::ModelInstanceRenderer()
	{

	}

	ModelInstanceRenderer::~ModelInstanceRenderer()
	{

	}

	void ModelInstanceRenderer::Init(GFXDevice* pDevice, Scene* pScene, const uint64 uInstanceId, const ModelResource::Mesh* pMesh, bool bDepth)
	{
		ASSERT(pMesh->primitive.eSkinningMode == exchange::SkinningType_NO_SKINNING);

		m_mesh.Init(pDevice, pScene, pMesh, nullptr, bDepth, true);
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


	void ModelInstanceRenderer::FinishGroup()
	{
		if(m_uActiveDrawId != USG_INVALID_ID)
		{
			DrawGroup group;
			group.uStartIndex = m_uStartIndex;
			group.uCount = (uint32)m_instanceData.size() - m_uStartIndex;

			m_groups[m_uActiveDrawId] = group;

			m_uStartIndex = (uint32)m_instanceData.size();
		}
	}

	void ModelInstanceRenderer::RenderNodes(RenderNode** ppNodes, uint32 uCount, uint32 uDrawId)
	{
		if (uDrawId != m_uActiveDrawId)
		{
			FinishGroup();
		}
		ASSERT(m_groups.find(uDrawId) == m_groups.end());

		for(uint32 i=0; i<uCount; i++)
		{
			Model::RenderMesh* pMesh = (Model::RenderMesh*)(ppNodes[i]);
			m_instanceData.push_back( pMesh->GetInstanceTransform() );		
		}

	}

	void ModelInstanceRenderer::PreDraw(GFXDevice* pDevice)
	{
		FinishGroup();

		if (m_instanceData.size() > 0)
		{
			if (m_instanceData.size() > m_instanceBuffer.GetCount())
			{
				m_instanceBuffer.Cleanup(pDevice);
				m_instanceBuffer.Init(pDevice, nullptr, sizeof(Matrix4x3), (uint32)Math::Roundup(m_instanceData.size(), 32), "ModelInstance");
			}
			m_instanceBuffer.SetContents(pDevice, m_instanceData.data(), (uint32)m_instanceData.size());
		}
	}


	void ModelInstanceRenderer::DrawFinished()
	{
		m_uStartIndex = 0;
		m_instanceData.empty();
		m_groups.empty();
	}

}


