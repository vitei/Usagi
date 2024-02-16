/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Simple model from binary
*****************************************************************************/
#ifndef _USG_GRAPHICS_SCENE_MODEL_RENDER_NODES_H_
#define _USG_GRAPHICS_SCENE_MODEL_RENDER_NODES_H_

#include "Engine/Maths/Matrix4x4.h"
#include "Engine/Scene/TransformNode.h"
#include "Engine/Resource/ModelResource.h"
#include "Engine/Resource/ModelResourceMesh.h"
#include "Engine/Scene/Model/Skeleton.h"
#include "Engine/Scene/Model/UVMapper.h"
#include "Engine/Scene/Model/Model.h"
 

namespace usg {

	class Model::ModelNodeBase : public RenderNode
	{
	public:
		virtual void Init(GFXDevice* pDevice, Scene* pScene, const ModelResource::Mesh* pMesh, const Model* pModel, bool bDepth) { }
		virtual void Cleanup(GFXDevice* pDevice) { }
		virtual class Model::RenderMesh* AsRenderMesh() { return nullptr; };
		virtual class Model::InstanceMesh* AsInstanceMesh() { return nullptr; };
		uint8 GetLod() { return m_uLod; }

		void SetRenderMaskWithShadowCheck(uint32 uMask)
		{
			if (!m_bCanHaveShadow)
			{
				uMask &= ~RenderMask::RENDER_MASK_SHADOW_CAST;
			}
			SetRenderMaskIncShadow(uMask);
		}

	protected:
		uint8 m_uLod = false;
		bool m_bCanHaveShadow = false;
	};

	class RenderNodeEx : public Model::ModelNodeBase
	{
	public:
		RenderNodeEx()
		{
		
			m_pIndexBuffer = NULL;
			SetPriority(128);	// Set the priority to middle level so we can easily tweak ordering
			m_uVertexBuffers = 0;

		}
		virtual ~RenderNodeEx() {}
		virtual void Cleanup(GFXDevice* pDevice)
		{
			m_descriptorSet.Cleanup(pDevice);
		}


		void SetVertexBuffer( uint32 index, const VertexBuffer* pBuffer )
		{
			if(!pBuffer || pBuffer->GetCount() == 0)
				return;
			
			for(uint32 i=0; i<m_uVertexBuffers; i++)
			{
				if(m_vertexBuffer[i].uIndex == index)
				{
					m_vertexBuffer[i].pBuffer = pBuffer;
					return;
				}
			}

			m_vertexBuffer[m_uVertexBuffers].pBuffer = pBuffer;
			m_vertexBuffer[m_uVertexBuffers].uIndex = index;
			m_uVertexBuffers++;
		}
		void SetIndexBuffer(const IndexBuffer* pBuffer )
		{
			if(pBuffer->GetIndexCount() != 0)
			{
				m_pIndexBuffer = pBuffer;
			}
		}

		DescriptorSet& GetDescriptorSet() { return m_descriptorSet;  }
		void SetBlendColor(usg::Color color) { m_blendColor = color; }
		// So far no good reason to override deferred as all our overrides have been for transparency
		void SetPipelineState(const PipelineStateHndl& pipeline) { m_pipelineState = pipeline; }

		uint32 GetVertexBufferCount() const { return m_uVertexBuffers; }

	protected:
		struct VertexBufferSet
		{
			const VertexBuffer* pBuffer;
			uint32 uIndex;
		};

		DescriptorSet			m_descriptorSet;
		PipelineStateHndl		m_pipelineState;
		PipelineStateHndl		m_omniDepthPipelineState;

		PipelineStateDecl		defaultPipeline;
		PipelineStateDecl		deferredPipeline;
		PipelineStateDecl		transparentPipeline;
		PipelineStateDecl		depthPassPipeline;
		PipelineStateDecl		omniDepthPassPipeline;

		usg::Color				m_blendColor;
		const IndexBuffer*		m_pIndexBuffer;
		VertexBufferSet			m_vertexBuffer[ModelResource::Mesh::VERTEX_BUFFER_NUM];
		uint32					m_uVertexBuffers;
	};


	// Not using a mesh directly as we will want to support
	// texture and material overrides
	class Model::RenderMesh : public RenderNodeEx
	{
		typedef RenderNodeEx Inherited;
	public:
		RenderMesh();
		virtual ~RenderMesh();

		// An instanced version should only be created by the model manager
		virtual void Init(GFXDevice* pDevice, Scene* pScene, const ModelResource::Mesh* pMesh, const Model* pModel, bool bDepth) override;
		virtual void Cleanup(GFXDevice* pDevice);


		UVMapper* GetUVMapper(uint32 uIndex) { return &m_uVMapper[uIndex]; }
		void SetOverrideConstant(uint32 uIndex, ConstantSet* pSet) { m_pOverridesConstants[uIndex] = pSet; }
		void RequestOverride(uint8 uIndex) { m_uReqOverrides |= (1 << uIndex); }
		void ResetOverrides() { m_uReqOverrides = 0; m_uOverrides = 0; }

		// Update the animations and UV co-ordinates
		virtual void VisibilityUpdate(GFXDevice* pDevice, const Vector4f& vTransformOffset);
		virtual bool Draw(GFXContext* pContext, RenderContext& renderContext);


		bool SetScale(float fScale, CustomEffectRuntime& customFX);

		virtual void RenderPassChanged(GFXDevice* pDevice, uint32 uContextId, const RenderPassHndl &renderPass, const SceneRenderPasses& passes) override;

		virtual class Model::RenderMesh* AsRenderMesh() { return this; };

	protected:
		virtual const PipelineStateDecl& GetPipelineState(ModelResource::Mesh::ERenderState eRenderState);
		virtual DescriptorSetLayoutHndl GetDescriptorHndl();

		enum 
		{
			OVERRIDE_MATERIAL = (1 << 0),
			OVERRIDE_MATERIAL_1 = (1<<1),
			OVERRIDE_COUNT = 2
		};

		const ModelResource::Mesh*	m_pMeshResource;
		UVMapper					m_uVMapper[ModelResource::Mesh::MAX_UV_STAGES];
		ConstantSet*				m_pOverridesConstants[OVERRIDE_COUNT];
		usg::string					m_name;
		uint8						m_uOverrides;
		uint8						m_uReqOverrides;
		bool						m_bInstanced;
		bool						m_bDepth;
	};


	class Model::InstanceDrawer : public Model::RenderMesh
	{
	public:
		bool InstanceDraw(GFXContext* pContext, RenderContext& renderContext, usg::VertexBuffer& instanceBuffer, uint32 uOffset, uint32 uCount);
	protected:
		virtual const PipelineStateDecl& GetPipelineState(ModelResource::Mesh::ERenderState eRenderState);
		virtual DescriptorSetLayoutHndl GetDescriptorHndl();

	};

	class Model::InstanceMesh : public Model::ModelNodeBase
	{
	public:
		virtual void Init(GFXDevice* pDevice, Scene* pScene, const ModelResource::Mesh* pMesh, const Model* pModel, bool bDepth);
		virtual uint64 GetInstanceId() const override { return m_uInstanceId; }
		virtual InstancedRenderer* CreateInstanceRenderer(GFXDevice* pDevice, Scene* pScene) override;
		virtual usg::Matrix4x4 GetInstanceTransform() const;
		
		virtual class Model::InstanceMesh* AsInstanceMesh() { return this; };
	private:
		
		const ModelResource::Mesh*	m_pMeshResource = nullptr;
		const Bone*					m_pBone = nullptr;
		bool						m_bDepth = false;
		uint64						m_uInstanceId = USG_INVALID_ID64;

	};


	struct Model::MaterialInfo
	{
		CustomEffectRuntime	customFX;
	};

}

#endif	// #ifndef _USG_GRAPHICS_SCENE_MODEL_RENDER_NODES_H_
