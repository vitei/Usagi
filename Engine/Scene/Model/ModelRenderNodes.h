/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Simple model from binary
*****************************************************************************/
#ifndef _USG_GRAPHICS_SCENE_MODEL_RENDER_NODES_H_
#define _USG_GRAPHICS_SCENE_MODEL_RENDER_NODES_H_
#include "Engine/Common/Common.h"
#include "Engine/Maths/Matrix4x4.h"
#include "Engine/Scene/TransformNode.h"
#include "Engine/Resource/ModelResource.h"
#include "Engine/Resource/ModelResourceMesh.h"
#include "Engine/Scene/Model/Skeleton.h"
#include "Engine/Scene/Model/UVMapper.h"
#include "Engine/Scene/Model/Model.h"
 

namespace usg {

	class RenderNodeEx : public RenderNode
	{
	public:
		RenderNodeEx()
		{
		
			m_pIndexBuffer = NULL;
			SetPriority(128);	// Set the priority to middle level so we can easily tweak ordering
			m_uVertexBuffers = 0;
			m_bEnableFog = true;

		}
		virtual ~RenderNodeEx() {}
		virtual void CleanUp(GFXDevice* pDevice)
		{
			m_descriptorSet.CleanUp(pDevice);
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

	protected:
		struct VertexBufferSet
		{
			const VertexBuffer* pBuffer;
			uint32 uIndex;
		};

		bool					m_bEnableFog;
		DescriptorSet			m_descriptorSet;
		PipelineStateHndl		m_pipelineState;
		PipelineStateHndl		m_deferredPipelineState;
		PipelineStateHndl		m_omniDepthPipelineState;
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

		virtual void Init(GFXDevice* pDevice, Scene* pScene, const ModelResource::Mesh* pMesh, const Model* pModel, bool bDepth);
		virtual void CleanUp(GFXDevice* pDevice);
		UVMapper* GetUVMapper(uint32 uIndex) { return &m_uVMapper[uIndex]; }
		void SetOverrideConstant(uint32 uIndex, ConstantSet* pSet) { m_pOverridesConstants[uIndex] = pSet; }
		void RequestOverride(uint8 uIndex) { m_uReqOverrides |= (1 << uIndex); }
		void ResetOverrides() { m_uReqOverrides = 0; m_uOverrides = 0; }

		// Update the animations and UV co-ordinates
		virtual void VisibilityUpdate(GFXDevice* pDevice, const Vector4f& vTransformOffset);
		virtual bool Draw(GFXContext* pContext, RenderContext& renderContext);
		bool SetScale(float fScale, CustomEffectRuntime& customFX);
		uint8 GetLod() { return m_uLod; }
		void SetRenderMaskWithShadowCheck(uint32 uMask);

	protected:
		enum 
		{
			OVERRIDE_MATERIAL = (1 << 0),
			OVERRIDE_MATERIAL_1 = (1<<1),
			OVERRIDE_COUNT = 2
		};

		UVMapper					m_uVMapper[ModelResource::Mesh::MAX_UV_STAGES];
		ConstantSet*				m_pOverridesConstants[OVERRIDE_COUNT];
		const char*					m_pszName;
		uint8						m_uOverrides;
		uint8						m_uReqOverrides;
		bool						m_bCanHaveShadow;

		uint8						m_uLod;
	};


	struct Model::MaterialInfo
	{
		CustomEffectRuntime	customFX;
	};

}

#endif	// #ifndef _USG_GRAPHICS_SCENE_MODEL_RENDER_NODES_H_
