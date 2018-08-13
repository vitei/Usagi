/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Simple model from binary
*****************************************************************************/
#ifndef _USG_GRAPHICS_SCENE_MODEL_RESOURCE_MESH_H_
#define _USG_GRAPHICS_SCENE_MODEL_RESOURCE_MESH_H_
#include "Engine/Common/Common.h"
#include "Engine/Maths/Matrix4x4.h"
#include "Engine/Resource/SkeletonResource.h"
#include "Engine/Resource/ResourceBase.h"
#include "Engine/Resource/ResourceDecl.h"
#include "Engine/Scene/Model/Model.pb.h"

#include "Engine/Graphics/Materials/Material.pb.h"
#include "Engine/Scene/Model/Shape.pb.h"
#include "Engine/Scene/Common/CustomEffectRuntime.h"
#include "Engine/Resource/ModelResource.h"
#include "Engine/Core/stl/vector.h"
  
namespace usg{

	class GFXContext;

	struct ModelResource::Mesh
	{
		enum
		{
			VERTEX_BUFFER_NUM = 1 + exchange::Shape::singleAttributes_max_count,
			MAX_UV_STAGES = 7
		};

		struct Primitive
		{
			IndexBuffer				indexBuffer;
			usg::exchange::SkinningType eSkinningMode;
			uint32					uRootIndex;
		} primitive;
		VertexBuffer			vertexBuffer[VERTEX_BUFFER_NUM];
		CustomEffectRuntime		effectRuntime;

		struct PipelineGroup
		{
			PipelineStateDecl		defaultPipeline;
			PipelineStateDecl		deferredPipeline;
			PipelineStateDecl		depthPassPipeline;
			PipelineStateDecl		omniDepthPassPipeline;
			PipelineStateDecl		translucentStateCmp;
		} pipelines;

		VertexElement vertexElements[MAX_VERTEX_ATTRIBUTES+1+(2*exchange::Shape::singleAttributes_max_count)];	// +1 for cap



		void CleanUp(GFXDevice* pDevice)
		{
			for (auto& it : vertexBuffer)
			{
				it.CleanUp(pDevice);
			}
			primitive.indexBuffer.CleanUp(pDevice);
			effectRuntime.CleanUp(pDevice);
		}


		DescriptorSetLayoutHndl	defaultPipelineDescLayout;
		U8String				name;
		U8String				matName;

		TextureHndl				pTextures[MAX_UV_STAGES];
		SamplerHndl				samplers[MAX_UV_STAGES];
		TextureCoordInfo		uvMapping[MAX_UV_STAGES];
		uint32					uUVCount;
		uint32					uFogIndex;
		bool					bEnableFog;
		bool					bCanFade;

		uint32					singleAttributesNum;
		Vector4f				singleAttributeValues[exchange::Shape::singleAttributes_max_count];

		RenderNode::Layer		layer;
		uint8					priority;
		uint8					uLodIndex;
	};

}

#endif	// #ifndef _USG_GRAPHICS_SCENE_MODEL_H_
