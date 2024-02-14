/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Rendering of multiple models with a single draw call
*****************************************************************************/
#pragma once
#include "Engine/Maths/Matrix4x4.h"
#include "Engine/Core/stl/vector.h"
#include "Engine/Core/stl/hash_map.h"
#include "Engine/Scene/TransformNode.h"
#include "Engine/Graphics/Primitives/VertexBuffer.h"
#include "Engine/Resource/ModelResource.h"
#include "Engine/Scene/RenderNode.h"
#include "Engine/Resource/ModelResourceMesh.h"
#include "Engine/Scene/Model/Skeleton.h"
#include "Engine/Scene/Model/UVMapper.h"
#include "Engine/Scene/Model/Model.h"
#include "Engine/Scene/InstancedRenderer.h"
#include "ModelRenderNodes.h"
 

namespace usg {

	class ModelInstanceRenderer : public InstancedRenderer
	{
	public:
		ModelInstanceRenderer();
		virtual ~ModelInstanceRenderer();

		void Init(GFXDevice* pDevice, Scene* pScene, const uint64 uInstanceId, const ModelResource::Mesh* pMesh, bool bDepth);
		
		virtual uint64 GetInstanceId();
		virtual void Draw(GFXContext* pContext, RenderNode::RenderContext& renderContext, uint32 uDrawId);
		virtual void RenderNodes(RenderNode** ppNodes, uint32 uCount, uint32 uDrawId);
		virtual void PreDraw(GFXDevice* pDevice) override;

		virtual void DrawFinished();

	protected:
		void FinishGroup();

		struct DrawGroup
		{
			uint32 uStartIndex;
			uint32 uCount;
		};

		vector<Matrix4x3>				m_instanceData;
		hash_map<uint32, DrawGroup>		m_groups;
		VertexBuffer					m_instanceBuffer;
		Model::RenderMesh				m_mesh;
		uint64							m_uInstanceId = 0;
		uint32							m_uStartIndex = 0;
		uint32							m_uActiveDrawId = USG_INVALID_ID;

	};

}



