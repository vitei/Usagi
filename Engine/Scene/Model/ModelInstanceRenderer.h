/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Rendering of multiple models with a single draw call
*****************************************************************************/
#pragma once
#include "Engine/Maths/Matrix4x4.h"
#include "Engine/Core/stl/list.h"
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

	class ModelInstanceRenderer;

	class ModelInstanceSet : public RenderNode
	{
	public:
		ModelInstanceSet() {}
		virtual ~ModelInstanceSet() {}

		void Init(ModelInstanceRenderer* pOwner, uint32 uStartIndex, uint32 uCount);
		virtual bool Draw(GFXContext* pContext, RenderContext& renderContext);

	private:
		ModelInstanceRenderer*	m_pOwner;
		uint32					m_uStartIndex = 0;
		uint32					m_uCount = 0;
	};

	class ModelInstanceRenderer : public InstancedRenderer
	{
	public:
		ModelInstanceRenderer();
		virtual ~ModelInstanceRenderer();

		void Init(GFXDevice* pDevice, Scene* pScene, const uint64 uInstanceId, const ModelResource::Mesh* pMesh, bool bDepth);
		virtual void Cleanup(GFXDevice* pDevice);
		virtual uint64 GetInstanceId();
		virtual void Draw(GFXContext* pContext, RenderNode::RenderContext& renderContext, uint32 uDrawId);
		virtual void AddNode(RenderNode* ppNodes) override;
		RenderNode* EndBatch() override;
		virtual void PreDraw(GFXDevice* pDevice) override;

		virtual void DrawFinished();

		Model::InstanceDrawer& GetDrawer() { return m_mesh; }
		VertexBuffer& GetInstanceBuffer() { return m_instanceBuffer; }

	protected:

		vector<Matrix4x3>				m_instanceData;
		usg::FastPool<ModelInstanceSet>	m_groups;
		VertexBuffer					m_instanceBuffer;
		Model::InstanceDrawer			m_mesh;
		uint64							m_uInstanceId = 0;
		memsize							m_uStartIndex = 0;
		uint32							m_uGroups = 0;

	};

}



