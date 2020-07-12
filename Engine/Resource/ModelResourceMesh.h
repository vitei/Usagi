/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Simple model from binary
*****************************************************************************/
#ifndef _USG_GRAPHICS_SCENE_MODEL_RESOURCE_MESH_H_
#define _USG_GRAPHICS_SCENE_MODEL_RESOURCE_MESH_H_

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
			RS_DEFAULT = 0,
			RS_DEFERRED,
			RS_TRANSPARENT,
			RS_DEPTH,
			RS_OMNI_DEPTH,
			RS_COUNT,
			VERTEX_BUFFER_NUM = 1 + exchange::Shape::singleAttributes_max_count,
			MAX_UV_STAGES = 7,
			MAX_VERT_ELEMENTS = MAX_VERTEX_ATTRIBUTES + 1 + (2 * exchange::Shape::singleAttributes_max_count)
		};

		struct Primitive
		{
			IndexBuffer				indexBuffer;
			usg::exchange::SkinningType eSkinningMode;
			uint32					uRootIndex;
		} primitive;
		VertexBuffer			vertexBuffer;
		CustomEffectRuntime		effectRuntime;

		struct RenderSet
		{
			PipelineStateDecl		pipeline;
			VertexBuffer			singleVerts;
		} renderSets[RS_COUNT];


		VertexElement vertexElements[RS_COUNT][MAX_VERT_ELEMENTS];	// +1 for cap



		void CleanUp(GFXDevice* pDevice)
		{
			vertexBuffer.CleanUp(pDevice);
			for (auto& it : renderSets)
			{
				it.singleVerts.CleanUp(pDevice);
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

		RenderLayer				layer;
		uint8					priority;
		uint8					uLodIndex;
	};

}

#endif	// #ifndef _USG_GRAPHICS_SCENE_MODEL_H_
