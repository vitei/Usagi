/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Core/File/File.h"
#include "Engine/Memory/Mem.h"
#include "Engine/Graphics/Device/GFXDevice.h"
#include "Engine/Resource/ResourceMgr.h"
#include "Engine/Graphics/Materials/Material.pb.h"
#include "Engine/Resource/ModelResourceMesh.h"
#include "Engine/Scene/Model/Shape.pb.h"
#include "Engine/Scene/Model/Mesh.pb.h"
#include "Engine/Resource/CustomEffectResource.h"
#include "Engine/Scene/SceneConstantSets.h"
#include "Engine/Scene/Common/CustomEffectRuntime.h"
#include "Engine/Scene/RenderGroup.h"
#include "Engine/Scene/Model/Skeleton.pb.h"
#include "Engine/Graphics/Effects/Effect.h"
#include "Engine/Scene/ViewContext.h"
#include "ModelResource.h"

namespace usg{

static uint32 g_uAttribId[usg::exchange::_VertexAttribute_count] =
{
	0,
	0,
	1,
	6,
	2,	
	5,		
	3,
	4,
	10
};

// Instance's vertex declaration
static const VertexElement g_instanceElements[]  =
{
	{
		9,	// szInditifier
		0,				// uOffset
		VE_FLOAT,		// eType;
		12,				// uCount;
		false			// bNormalised
	},
	VERTEX_DATA_END()
};



static RenderLayer layerMapping[] = 
{
	RenderLayer::LAYER_OPAQUE,
	RenderLayer::LAYER_TRANSLUCENT,
	RenderLayer::LAYER_SUBTRACTIVE,
	RenderLayer::LAYER_ADDITIVE
};



uint32 calcIndexStreamSizeSum( const exchange::Shape* pShape )
{
	uint32 offset = 0;
	const exchange::Primitive& prim = pShape->primitive;
	if( prim.indexStream.indexNum == 0 )
	{
		return 0;
	}
	
	offset += prim.indexStream.sizeAligned + prim.adjacencyStream.sizeAligned;
	
	return offset;
}

exchange::Shape* GetShape( usg::exchange::ModelHeader* pHeader, uint32 index )
{
	uint8* pShapePtr = reinterpret_cast<uint8*>(pHeader) + pHeader->shapeOffset;
	for( unsigned int i = 0; i < index; ++i ) 
	{
		exchange::Shape* pTempShape = reinterpret_cast<exchange::Shape*>( pShapePtr );
		pShapePtr += pTempShape->streamOffset;
		pShapePtr += calcIndexStreamSizeSum( pTempShape );
		pShapePtr += pTempShape->vertexStreamSizeAligned;
	}

	return reinterpret_cast<exchange::Shape*>( pShapePtr );
}

void SetupAlphaStateDecl( AlphaStateDecl& decl, const exchange::Rasterizer& rasterizer )
{
	if( rasterizer.attribute & ( 1 << exchange::Rasterizer_Attribute_ALPHA_TEST_ENABLE ) )
	{
		decl.eAlphaTest = static_cast<AlphaTest>( rasterizer.alphaState.alphaTestFunc );
		decl.uAlphaRef = static_cast<uint8>( rasterizer.alphaState.alphaTestReference * UCHAR_MAX );
	}

	if( rasterizer.blendEnabled )
	{
		decl.bBlendEnable = true;
		decl.srcBlend = static_cast<BlendFunc>( rasterizer.alphaState.rgbSrcFunc );
		decl.dstBlend = static_cast<BlendFunc>( rasterizer.alphaState.rgbDestFunc );
		decl.blendEq  = static_cast<BlendEquation>( rasterizer.alphaState.rgbOp );
		decl.srcBlendAlpha = static_cast<BlendFunc>( rasterizer.alphaState.alphaSrcFunc );
		decl.dstBlendAlpha = static_cast<BlendFunc>( rasterizer.alphaState.alphaDestFunc );
		decl.blendEqAlpha  = static_cast<BlendEquation>( rasterizer.alphaState.alphaOp );
	}

	for( int i = 0; i < MAX_COLOR_TARGETS; ++i )
	{
		decl.uColorMask[i] = rasterizer.colorMask;
	}
}

void SetupDepthStencilStateDecl( DepthStencilStateDecl& decl, const exchange::StencilTest& stencilTest )
{
	if( stencilTest.isEnable )
	{
		decl.bStencilEnable = true;
		decl.eStencilTest = static_cast<StencilTest>( stencilTest.func );

		decl.SetMask(stencilTest.mask, stencilTest.mask, stencilTest.ref);
		decl.SetOperation((StencilOp)stencilTest.passOperation, (StencilOp)stencilTest.failOperation, (StencilOp)stencilTest.zFailOperation);
	}
}

SamplerFilter chooseMagFilter( exchange::Texture_Filter filter )
{
	return ( filter == exchange::Texture_Filter_nearest ? SF_POINT : SF_LINEAR );
}

SamplerFilter chooseMinFilter( exchange::Texture_Filter filter )
{
	if( filter == exchange::Texture_Filter_nearest ||
		filter == exchange::Texture_Filter_nearest_mipmap_nearest ||
		filter == exchange::Texture_Filter_nearest_mipmap_linear )
	{
		return SF_POINT;
	}
	else
	{
		return SF_LINEAR;
	}
}

MipFilter chooseMipFilter( exchange::Texture_Filter filter )
{
	if( filter == exchange::Texture_Filter_nearest ||
		filter == exchange::Texture_Filter_nearest_mipmap_nearest ||
		filter == exchange::Texture_Filter_linear_mipmap_nearest )
	{
		return MF_POINT;
	}
	else
	{
		return MF_LINEAR;
	}
}

ModelResource::ModelResource() :
	ResourceBase(StaticResType)
{
	m_bInstance = false;
	m_meshArray = NULL;
}

ModelResource::~ModelResource()
{
	if(m_meshArray)
	{
		vdelete[] m_meshArray;
	}
}


bool ModelResource::Load( GFXDevice* pDevice, const char* szFileName, bool bInstance, bool bFastMem )
{
	m_bNeedsRootNode = false;
	m_uBoneNodes = 0;

	m_bInstance = false;// bInstance;
	U8String path = szFileName;
	path.TruncateToPath();

	ResourceMgr::Inst()->LoadPackage(pDevice, path.CStr(), "resources.pak");

	File modelFile(szFileName, FILE_ACCESS_READ );
	
	if( !modelFile.IsOpen() )
	{
		// We couldn't find this file.
		ASSERT( 0 );
		return false;
	}

	uint8* p = NULL;
	ScratchObj<uint8> scratchModel( p, (uint32)modelFile.GetSize(), FILE_READ_ALIGN );
	modelFile.Read( modelFile.GetSize(), p );

	// Set up the indices
	usg::exchange::ModelHeader* pHeader = reinterpret_cast<usg::exchange::ModelHeader*>(p);
	if (pHeader->rigidBoneIndices_count)
	{
		m_rigidBoneIndices.resize(pHeader->rigidBoneIndices_count);
		for (uint32 i = 0; i < pHeader->rigidBoneIndices_count; i++)
		{
			m_rigidBoneIndices[i] = pHeader->rigidBoneIndices[i];
		}
	}

	if (pHeader->skinnedBoneIndices_count)
	{
		m_smoothBoneIndices.resize(pHeader->skinnedBoneIndices_count);
		for (uint32 i = 0; i < pHeader->skinnedBoneIndices_count; i++)
		{
			m_smoothBoneIndices[i] = pHeader->skinnedBoneIndices[i];
		}
	}

	m_name = szFileName;
	SetupHash( m_name.CStr() );
	SetupMeshes( path, pDevice, p, bFastMem );
	SetupSkeleton( p );

	SetReady(true);
	return true;
}


void ModelResource::Cleanup(GFXDevice* pDevice)
{
	for (uint32 i = 0; i < m_uMeshCount; i++)
	{
		m_meshArray[i].Cleanup(pDevice);
	}
}


void ModelResource::SetupMeshes( const U8String & modelDir, GFXDevice* pDevice, uint8* p, bool bFastMem )
{
	usg::exchange::ModelHeader* pHeader = reinterpret_cast<usg::exchange::ModelHeader*>( p );

	ASSERT(m_meshArray == NULL);
	m_meshArray = vnew(usg::ALLOC_GEOMETRY_DATA) Mesh[pHeader->meshNum];

	const uint32 uMaxLodLevel = 0;
	m_uMeshCount = 0;

	bool bUseLods = ResourceMgr::Inst()->AreLodsEnabled();

	// Looking forward to removing this LOD stuff
	for (uint32 n = 0; n < pHeader->meshNum; ++n)
	{
		uint8* pT = reinterpret_cast<uint8*>(pHeader);

		exchange::Material* pInitialMaterial = reinterpret_cast<exchange::Material*>(pT + pHeader->materialOffset);
		exchange::Mesh* pInitialMesh = reinterpret_cast<exchange::Mesh*>(pT + pHeader->meshOffset);

		if (!bUseLods)
		{
			const char* szLod = strstr(pInitialMesh[n].name, "LOD");
			if (szLod)
			{
				if ((szLod[3] - '0') > 0)
					continue;
			}
		}

		SetupMesh(modelDir, pDevice, pHeader, n, bFastMem);

		m_uMeshCount++;
	}


	// bounding sphere
	Vector3f vPos( pHeader->boundingSphere.center.x, pHeader->boundingSphere.center.y, pHeader->boundingSphere.center.z );
	m_bounds.SetPos(vPos);
	m_bounds.SetRadius( pHeader->boundingSphere.radius );

}

DescriptorSetLayoutHndl GetDeclarationLayout(GFXDevice* pDevice, const exchange::Material* pMaterial, bool bAnimated)
{
	uint32 uIndex = 0;
	DescriptorDeclaration decl[exchange::Material_Constant_TEXTURE_NUM + 6];

	for (int i = 0; i < exchange::Material_Constant_TEXTURE_NUM; ++i)
	{
		if (pMaterial->textures[i].textureName[0] != '\0')
		{
			decl[uIndex].eDescriptorType = DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			decl[uIndex].shaderType = SHADER_FLAG_PIXEL;
			decl[uIndex].uCount = 1;
			decl[uIndex].uBinding = i;
			uIndex++;
		}
	}

	if(!bAnimated)
	{
		decl[uIndex].eDescriptorType = DESCRIPTOR_TYPE_CONSTANT_BUFFER_DYNAMIC;
		decl[uIndex].shaderType = SHADER_FLAG_VERTEX;
		decl[uIndex].uCount = 1;
		decl[uIndex].uBinding = SHADER_CONSTANT_CUSTOM_0;
		uIndex++;
	}
	else
	{
		// Still enabling this separately
		decl[uIndex].eDescriptorType = DESCRIPTOR_TYPE_CONSTANT_BUFFER;
		decl[uIndex].shaderType = SHADER_FLAG_VERTEX;
		decl[uIndex].uCount = 1;
		decl[uIndex].uBinding = SHADER_CONSTANT_CUSTOM_2;
		uIndex++;
	}

	// Vertex material constants
	decl[uIndex].eDescriptorType = DESCRIPTOR_TYPE_CONSTANT_BUFFER;
	decl[uIndex].shaderType = SHADER_FLAG_VERTEX;
	decl[uIndex].uCount = 1;
	decl[uIndex].uBinding = SHADER_CONSTANT_MATERIAL;
	uIndex++;

	// Fragment lighting
	decl[uIndex].eDescriptorType = DESCRIPTOR_TYPE_CONSTANT_BUFFER;
	decl[uIndex].shaderType = SHADER_FLAG_PIXEL;
	decl[uIndex].uCount = 1;
	decl[uIndex].uBinding = SHADER_CONSTANT_MATERIAL_1;
	uIndex++;

	decl[uIndex] = DESCRIPTOR_CAP;
	return pDevice->GetDescriptorSetLayout(decl);
}


memsize ModelResource::InitInputBindings(usg::GFXDevice* pDevice, const exchange::Shape* pShape, const exchange::Material* pMaterial, const CustomEffectResHndl& customFXDecl, 
											int RenderState, PipelineStateDecl& pipelineState)
{
	memsize uVertexSize = 0;
	PipelineStateDecl::InputBinding* bindings = pipelineState.inputBindings;
	VertexElement* pElement = m_meshArray[m_uMeshCount].vertexElements[RenderState];
	uint32 elementOffset = GetModelDeclUVReusse(pShape, customFXDecl, pMaterial, pElement, uVertexSize);
	bindings[0].Init(pElement);
	bindings[0].uVertexSize = (uint32)uVertexSize;
	pElement += elementOffset;
	pipelineState.uInputBindingCount = 1;

	// Missing attributes
	{
		uint32 uMaxSize = 0;
		for(uint32 i=0; i<customFXDecl->GetAttribCount(); i++)
		{
			uMaxSize += g_uConstantSize[customFXDecl->GetAttribute(i)->eConstantType] * customFXDecl->GetAttribute(i)->uCount;
		}
		usg::ScratchRaw singleAttribScratch(uMaxSize, 4);
		uint8* pSingleAttribData = (uint8*)singleAttribScratch.GetRawData();
		usg::VertexElement* pStaticElements = pElement;
		uint32 uDataSize = 0;

		if (pShape->singleAttributes_count != 0)
		{
			// Single attributes
			for (size_t i = 0; i < pShape->singleAttributes_count; ++i)
			{
				if (!GetSingleAttributeDeclNamed(customFXDecl, pShape->singleAttributes[i].usageHint, pShape->singleAttributes[i].columns, pElement))
					continue;

				pElement++;

				uint32 uElementSize = sizeof(float) * pShape->singleAttributes[i].columns;
				memcpy(pSingleAttribData, &pShape->singleAttributes[i].value, uElementSize);
				pSingleAttribData += uElementSize;
				uDataSize += uElementSize;
			}
		}

		// Effect defaults
		for (uint32 i = 0; i < customFXDecl->GetAttribCount(); i++)
		{
			// Set up the vertex buffer for attributes without any vertex streams
			const CustomEffectDecl::Attribute* attrib = customFXDecl->GetAttribute(i);
			for (uint32 j = 0; j < attrib->uCount; j++)
			{
				uint32 attribIdx = attrib->uIndex + j;
				bool bFound = false;
				// If we already have it we don't want another copy
				for (const VertexElement* pCmpElement = m_meshArray[m_uMeshCount].vertexElements[RenderState]; pCmpElement < pElement; pCmpElement++)
				{
					uint32 uAttribEnd = attrib->uIndex + attrib->uCount;
					if (pCmpElement->uAttribId >= (attrib->uIndex + j) && pCmpElement->uAttribId < uAttribEnd)
					{
						bFound = true;
						break;
					}
				}

				if (bFound)
				{
					continue;
				}

				GetSingleAttributeDeclDefault(attrib, uDataSize, pElement);
				pElement->uAttribId += j;
				pElement++;

				// We didn't have that attribute from the model, so hook it up
				uint32 uElementSize = g_uConstantSize[attrib->eConstantType];
				memcpy(pSingleAttribData, attrib->defaultData, uElementSize);
				pSingleAttribData += uElementSize;
				uDataSize += uElementSize;
			}
		}

		*pElement = VERTEX_ELEMENT_CAP;	// Cap off the declaration
		bindings[1].Init(pStaticElements, (uint32)1, VERTEX_INPUT_RATE_INSTANCE, (uint32)(-1));
		m_meshArray[m_uMeshCount].renderSets[RenderState].singleVerts.Init(pDevice, singleAttribScratch.GetRawData(), uDataSize, 1, pMaterial->renderPasses[RenderState].effectName, GPU_USAGE_CONST_REG);
		pipelineState.uInputBindingCount++;
	}

	return uVertexSize;
}


void ModelResource::SetupMesh( const U8String & modelDir, GFXDevice* pDevice, usg::exchange::ModelHeader* pHeader, uint32 meshIndex, bool bFastMem )
{
	uint8* pT = reinterpret_cast<uint8*>( pHeader );

	exchange::Material* pInitialMaterial = reinterpret_cast<exchange::Material*>( pT + pHeader->materialOffset );
	exchange::Mesh* pInitialMesh = reinterpret_cast<exchange::Mesh*>( pT + pHeader->meshOffset );

	unsigned int shapeIndex = pInitialMesh[meshIndex].shapeRefIndex;
	unsigned int materialIndex = pInitialMesh[meshIndex].materialRefIndex;

	// referenced shape
	exchange::Shape* pShape = GetShape( pHeader, shapeIndex );
	uint8* pIndexStream = reinterpret_cast<uint8*>(pShape) + pShape->streamOffset;
	
	// referenced material
	exchange::Material* pMaterial = &pInitialMaterial[materialIndex];

	uint32 uCount = pShape->streamInfo_count;
	const int maxElements = 15;
	// FIXME: Remove hardocding

	EffectHndl effects[usg::exchange::_Material_RenderPass_count];

	for(uint32 i=0; i<usg::exchange::_Material_RenderPass_count; i++)
	{
		effects[i] = usg::ResourceMgr::Inst()->GetEffect(pDevice, pMaterial->renderPasses[i].effectName);
		m_meshArray[m_uMeshCount].renderSets[i].effectRuntime.Init(pDevice, effects[i]->GetCustomEffect());
	}

	PipelineStateDecl pipelineState;
	pipelineState.ePrimType = PT_TRIANGLES;


	bool bAnimated = pShape->skinningType != usg::exchange::SkinningType_NO_SKINNING;

	ASSERT(uCount <= maxElements);

	memsize uVertexSize = InitInputBindings(pDevice, pShape, pMaterial, effects[usg::exchange::Material_RenderPass_DEFAULT]->GetCustomEffect(), Mesh::RS_DEFAULT, pipelineState);

	DescriptorSetLayoutHndl matDescriptors = GetDeclarationLayout(pDevice, pMaterial, bAnimated);
	pipelineState.layout.descriptorSets[0] = pDevice->GetDescriptorSetLayout(SceneConsts::g_globalDescriptorDecl);
	pipelineState.layout.descriptorSets[1] = matDescriptors;
	pipelineState.layout.uDescriptorSetCount = 2;

	m_meshArray[m_uMeshCount].defaultPipelineDescLayout = matDescriptors;

	GPULocation eGPULocation = bFastMem ? GPU_LOCATION_FASTMEM : GPU_LOCATION_STANDARD;
	// Seem to be running short on VRAM and it's not impacting performance any
	GPULocation eVertGPULocation = GPU_LOCATION_STANDARD;//bFastMem ? GPU_LOCATION_FASTMEM : GPU_LOCATION_STANDARD;
	
	// set vertex stream
	uint32 vertexStreamOffset = calcIndexStreamSizeSum( pShape );
	m_meshArray[m_uMeshCount].vertexBuffer.Init(pDevice, pIndexStream + vertexStreamOffset, (uint32)uVertexSize, pShape->vertexNum, "Mesh", GPU_USAGE_STATIC, eVertGPULocation );
	
	// TODO: Implement skinning
	m_meshArray[m_uMeshCount].name = pInitialMesh[meshIndex].name;
	m_meshArray[m_uMeshCount].uLodIndex = 0;
	const char* szLod = strstr(pInitialMesh[meshIndex].name, "LOD");
	if( szLod )
	{
		m_meshArray[m_uMeshCount].uLodIndex  = szLod[3] - '0';
		ASSERT(m_meshArray[m_uMeshCount].uLodIndex < RenderGroup::MAX_LOD_GROUPS);
	}

	const exchange::Primitive& prim = pShape->primitive;

	m_meshArray[m_uMeshCount].primitive.uRootIndex = pShape->primitive.rootBone;
	m_meshArray[m_uMeshCount].primitive.eSkinningMode = (usg::exchange::SkinningType)pShape->skinningType;
	
	if( (m_meshArray[m_uMeshCount].primitive.eSkinningMode != usg::exchange::SkinningType_NO_SKINNING) || m_meshArray[m_uMeshCount].primitive.uRootIndex == 0 )
	{
		m_bNeedsRootNode = true;
	}
	else
	{
		m_uBoneNodes++;
	}

	m_meshArray[m_uMeshCount].primitive.indexBuffer.InitSize(pDevice, pIndexStream, prim.indexStream.formatSize, prim.indexStream.indexNum, true, eVertGPULocation);
	pIndexStream += prim.indexStream.sizeAligned + prim.adjacencyStream.sizeAligned;
	


	DepthStencilStateDecl& depthDecl = pipelineState.depthState;
	depthDecl.bDepthWrite = ( pMaterial->rasterizer.attribute & 1 << exchange::Rasterizer_Attribute_DEPTH_TEST_WRITE ? true : false );
	depthDecl.bDepthEnable = ( pMaterial->rasterizer.attribute & 1 << exchange::Rasterizer_Attribute_DEPTH_TEST_ENABLE ? true : false );
	depthDecl.eDepthFunc = static_cast<DepthTest>( pMaterial->rasterizer.depthTestFunc );
	SetupDepthStencilStateDecl( depthDecl, pMaterial->rasterizer.stencilTest );
	depthDecl.SetMask(0x0, STENCIL_MASK_GEOMETRY, STENCIL_GEOMETRY);
	depthDecl.SetOperation(STENCIL_OP_REPLACE, STENCIL_OP_KEEP, STENCIL_OP_KEEP);
	depthDecl.bStencilEnable = true;

	AlphaStateDecl& alphaDecl = pipelineState.alphaState;
	SetupAlphaStateDecl( alphaDecl, pMaterial->rasterizer );
	m_meshArray[m_uMeshCount].layer = layerMapping[pMaterial->attribute.translucencyKind];
	m_meshArray[m_uMeshCount].priority = pInitialMesh[meshIndex].renderPriority;
	m_meshArray[m_uMeshCount].bCanFade = (pMaterial->rasterizer.colorMask & usg::RT_MASK_RGB)!=0;

	RasterizerStateDecl& rasterizerDecl = pipelineState.rasterizerState;
	rasterizerDecl.eCullFace = static_cast<CullFace>( pMaterial->rasterizer.cullFace );
	rasterizerDecl.bUseDepthBias = ( pMaterial->rasterizer.isPolygonOffsetEnable ? true : false );
	rasterizerDecl.fDepthBias = static_cast<float>( pMaterial->rasterizer.polygonOffsetUnit );

	pipelineState.pEffect = effects[usg::exchange::Material_RenderPass_DEFAULT];
	m_meshArray[m_uMeshCount].matName = pMaterial->materialName;

	pipelineState.alphaState.uColorTargets = 1;
	m_meshArray[m_uMeshCount].renderSets[Mesh::RS_DEFAULT].pipeline = pipelineState;

	pipelineState.pEffect = effects[usg::exchange::Material_RenderPass_DEFERRED];
	pipelineState.alphaState.uColorTargets = 5;
	InitInputBindings(pDevice, pShape, pMaterial, pipelineState.pEffect->GetCustomEffect(), Mesh::RS_DEFERRED, pipelineState);
	m_meshArray[m_uMeshCount].renderSets[Mesh::RS_DEFERRED].pipeline = pipelineState;

	pipelineState.alphaState.uColorTargets = 1;
	pipelineState.pEffect = effects[usg::exchange::Material_RenderPass_TRANSPARENT];
	if (m_meshArray[m_uMeshCount].layer < RenderLayer::LAYER_TRANSLUCENT)
	{
		// FIXME: Default blend settings for now, just here to handle our issues with default imported blender materials
		alphaDecl.bBlendEnable = true;
		alphaDecl.blendEq = usg::BLEND_EQUATION_ADD;
		alphaDecl.srcBlend = usg::BLEND_FUNC_SRC_ALPHA;
		alphaDecl.dstBlend = usg::BLEND_FUNC_ONE_MINUS_SRC_ALPHA;

		alphaDecl.blendEqAlpha = usg::BLEND_EQUATION_ADD;
		alphaDecl.srcBlendAlpha = usg::BLEND_FUNC_ZERO;
		alphaDecl.dstBlendAlpha = usg::BLEND_FUNC_ONE;
	}
	pipelineState.depthState.bDepthWrite = false;
	InitInputBindings(pDevice, pShape, pMaterial, pipelineState.pEffect->GetCustomEffect(), Mesh::RS_TRANSPARENT, pipelineState);
	m_meshArray[m_uMeshCount].renderSets[Mesh::RS_TRANSPARENT].pipeline = pipelineState;

	
	// FIXME: FXRunTime per type
	CustomEffectRuntime& fxRunTime = m_meshArray[m_uMeshCount].renderSets[0].effectRuntime;

	for (uint32 i = 0; i < pMaterial->renderPasses[0].constants_count; i++)
	{
		// Override the default values if we have them
		void* pData = (void*)(((uint8*)pMaterial->constantData) + pMaterial->renderPasses[0].constants[i].uOffset);
		fxRunTime.SetSetData(i, pData, pMaterial->renderPasses[0].constants[i].uSize);
	}
	// The w is scaling of the vertex color, if 0 the vertex color is ignored
	fxRunTime.SetVariable("iBoneCount", GetBoneIndexCount(pShape));

	// Texture Coordinator
	m_meshArray[m_uMeshCount].uUVCount = usg::Math::Min(pMaterial->textureCoordinators_count, fxRunTime.GetVariableCount("mTexMatrix"));
	uint32 uTexMatCount = fxRunTime.GetVariableCount("mTexMatrix");
	for( size_t i = 0; i < uTexMatCount; ++i )
	{
		Matrix4x3 mTex = Matrix4x4::Identity();
		const usg::exchange::TextureCoordinator& texCo = pMaterial->textureCoordinators[i];

		m_meshArray[m_uMeshCount].uvMapping[i] = texCo;
		
		// Matrix
		Matrix4x4 mTexMat = Matrix4x4::Identity();
		mTexMat.Scale(texCo.scale.x, texCo.scale.y, 1.0f, 1.0f);
		mTexMat.SetTranslation( texCo.translate.x, texCo.translate.y, 0.0f );
		Matrix4x3& matrix = mTex;
		matrix = mTexMat;

		fxRunTime.SetVariable("mTexMatrix", &mTex, 1, (uint32)i);
	}

	fxRunTime.GPUUpdate(pDevice);
	//mat.UnlockBuffer(0);

	for (uint32 i = 0; i < ModelResource::Mesh::MAX_UV_STAGES; i++)
	{
		m_meshArray[m_uMeshCount].pTextures[i] = NULL;
	}

	SamplerDecl sampDecl( SF_LINEAR, SC_WRAP );
	for( int i = 0; i < exchange::Material_Constant_TEXTURE_NUM; ++i )
	{
		if( pMaterial->textures[i].textureName[0] != '\0' )
		{
			uint32 uIndex = USG_INVALID_ID;
			uIndex = i;

			if( uIndex != USG_INVALID_ID )
			{
				U8String pathName = m_name;
				pathName.TruncateToPath();
				//pathName = U8String("models/") + pathName; 
				::usg::exchange::Texture& texture = pMaterial->textures[i];
				const char* pTextureName = &texture.textureName[0];
				pathName += pTextureName;

				sampDecl.eClampU = static_cast<SamplerClamp>(texture.wrapS);
				sampDecl.eClampV = static_cast<SamplerClamp>(texture.wrapT);
				sampDecl.eFilterMag = chooseMagFilter((exchange::Texture_Filter)texture.magFilter);
				sampDecl.eFilterMin = chooseMinFilter((exchange::Texture_Filter)texture.minFilter);
				sampDecl.eMipFilter = chooseMipFilter((exchange::Texture_Filter)texture.mipFilter);
				sampDecl.eAnisoLevel = (SamplerDecl::Anisotropy)texture.anisoLevel;
				sampDecl.LodBias = texture.lodBias;
				sampDecl.LodMinLevel = texture.lodMinLevel;


				// First check the models local textures
				m_meshArray[m_uMeshCount].pTextures[uIndex] = ResourceMgr::Inst()->GetTextureAbsolutePath(pDevice, pathName.CStr(), false, eGPULocation);
				if (m_meshArray[m_uMeshCount].pTextures[uIndex].get() == nullptr)
				{
					// Fallback to absolute path, passing in true for replace missing texture just in case
					m_meshArray[m_uMeshCount].pTextures[uIndex] = ResourceMgr::Inst()->GetTextureAbsolutePath(pDevice, texture.textureName, true, eGPULocation);
				}

				m_meshArray[m_uMeshCount].samplers[uIndex] = pDevice->GetSampler(sampDecl);
			}
		}
	}

	CreateDepthPassMaterial(pDevice, meshIndex, pShape, pMaterial);
}


void ModelResource::CreateDepthPassMaterial(GFXDevice* pDevice, uint32 uMeshIndex, exchange::Shape* pShape, exchange::Material* pMaterial)
{
	bool bAnimated = pShape->skinningType != usg::exchange::SkinningType_NO_SKINNING;

	PipelineStateDecl pipelineState = m_meshArray[m_uMeshCount].renderSets[Mesh::RS_DEFAULT].pipeline;

	usg::AlphaStateDecl& alphaDecl = pipelineState.alphaState;
	usg::RasterizerStateDecl& rasDecl = pipelineState.rasterizerState;
	usg::DepthStencilStateDecl& depthPassP = pipelineState.depthState;
	depthPassP.bDepthEnable = true;
	depthPassP.eDepthFunc = usg::DEPTH_TEST_LEQUAL;
	depthPassP.bStencilEnable = false;
	depthPassP.bDepthWrite = true;
	depthPassP.eStencilTest = usg::STENCIL_TEST_ALWAYS;

	usg::DescriptorSetLayoutHndl globalDescriptors = pDevice->GetDescriptorSetLayout(SceneConsts::g_shadowGlobalDescriptorDecl);
	pipelineState.layout.descriptorSets[0] = globalDescriptors;

	// FIXME: Render set per type
	U8String effectPath = pMaterial->renderPasses[usg::exchange::Material_RenderPass_DEPTH].effectName;

	pipelineState.pEffect = ResourceMgr::Inst()->GetEffect(pDevice, effectPath.CStr());

	alphaDecl.bBlendEnable = false;
	uint32 uRenderMask = alphaDecl.uColorMask[0];
	for (uint32 i = 0; i < MAX_COLOR_TARGETS; i++)
	{
		alphaDecl.uColorMask[i] = usg::RT_MASK_NONE;
	}

	InitInputBindings(pDevice, pShape, pMaterial, pipelineState.pEffect->GetCustomEffect(), Mesh::RS_DEPTH, pipelineState);
	m_meshArray[m_uMeshCount].renderSets[Mesh::RS_DEPTH].pipeline = pipelineState;

	U8String omniDepthName = pMaterial->renderPasses[usg::exchange::Material_RenderPass_OMNI_DEPTH].effectName;

	pipelineState.pEffect = ResourceMgr::Inst()->GetEffect(pDevice, omniDepthName.CStr());

	globalDescriptors = pDevice->GetDescriptorSetLayout(SceneConsts::g_omniShadowGlobalDescriptorDecl);
	pipelineState.layout.descriptorSets[0] = globalDescriptors;


	InitInputBindings(pDevice, pShape, pMaterial, pipelineState.pEffect->GetCustomEffect(), Mesh::RS_OMNI_DEPTH, pipelineState);
	m_meshArray[m_uMeshCount].renderSets[Mesh::RS_OMNI_DEPTH].pipeline = pipelineState;
	  

	// FIXME: Still using default effect for depth pass
	pipelineState.pEffect = ResourceMgr::Inst()->GetEffect(pDevice, pMaterial->renderPasses[usg::exchange::Material_RenderPass_DEFAULT].effectName);
	pipelineState = m_meshArray[m_uMeshCount].renderSets[Mesh::RS_DEFAULT].pipeline;
	alphaDecl.bBlendEnable = true;
	alphaDecl.uColorMask[0] = uRenderMask;
	alphaDecl.eAlphaTest = usg::ALPHA_TEST_ALWAYS;
	alphaDecl.blendEq = usg::BLEND_EQUATION_ADD;
	alphaDecl.srcBlend = usg::BLEND_FUNC_CONSTANT_COLOR;
	alphaDecl.dstBlend = usg::BLEND_FUNC_ONE_MINUS_CONSTANT_COLOR;

	// Initialize the depth stencil states needed to test against this stencil write
	depthPassP.bDepthEnable = true;
	depthPassP.bStencilEnable = false;
	depthPassP.bDepthWrite = false;

	depthPassP.eStencilTest = usg::STENCIL_TEST_ALWAYS;
	depthPassP.eDepthFunc = usg::DEPTH_TEST_EQUAL;
}

float ModelResource::GetStreamScaling(const usg::exchange::VertexStreamInfo* pInfo, uint32 uCount, usg::exchange::VertexAttribute eType)
{
	for (uint32 i = 0; i < uCount; i++)
	{
		if (pInfo[i].attribute == eType)
		{
			return pInfo[i].scaling;
		}
	}

	return 1.0f;
}

bool ModelResource::HasAttribute(const exchange::VertexStreamInfo* pInfo, exchange::VertexAttribute attrib, uint32 uCount)
{
	for (uint32 i = 0; i < uCount; i++)
	{
		if (pInfo[i].attribute == attrib)
		{
			return true;
		}
	}
	
	return false;
}

uint32 ModelResource::GetBoneIndexCount(const usg::exchange::Shape* pShape)
{
	const exchange::VertexStreamInfo* pInfo = pShape->streamInfo;
	const uint32 uCount = pShape->streamInfo_count;

	if (pShape->skinningType == usg::exchange::SkinningType_NO_SKINNING)
		return 0;

	if (pShape->skinningType == usg::exchange::SkinningType_RIGID_SKINNING)
		return 1;

	for (uint32 i = 0; i < uCount; i++)
	{
		if (pInfo[i].attribute == exchange::VertexAttribute_BONE_WEIGHT)
		{
			ASSERT(pInfo[i].columns > 1);
			return pInfo[i].columns;
		}
	}

	ASSERT(false);
	return 0;
}

static memsize AlignSize(memsize uSize, memsize uAlign)
{
	memsize uMask = uAlign - 1;
	memsize uMisAlignment = (uSize & uMask);
	memsize uAdjustment = uAlign - uMisAlignment;
	if (uMisAlignment == 0)
		return uSize;

	return uSize + uAdjustment;
}

uint32 ModelResource::GetModelDeclUVReusse(const exchange::Shape* pShape, const CustomEffectResHndl& customFXDecl, const exchange::Material* pMaterial, VertexElement elements[], memsize& offset)
{
	const exchange::VertexStreamInfo* pInfo = pShape->streamInfo;

	uint32 uCount = 0;
	const uint32 mask = 0xf;
	int texUV = 0;
	offset = 0;
	uint32 uUVIndex = customFXDecl->GetAttribBinding("uv0");
	for (uint32 i = 0; i < pShape->streamInfo_count; i++)
	{
		VertexElement element;

		// accumulate offset
		element.uCount = pInfo->columns;
		ASSERT(element.uCount <= 16);
		element.eType = (VertexElementType)pInfo->elementType;	// TODO: This should be defined in the struct
	//	offset = AlignSize(offset, VertexDeclaration::GetByteCount(element.eType));
		element.uOffset = offset;
		element.bNormalised = false;
		element.bIntegerReg = pInfo->attribute == exchange::VertexAttribute_BONE_INDEX;

		// Copy all the references we need
		if (pInfo->attribute == exchange::VertexAttribute_UV && uUVIndex != USG_INVALID_ID)
		{
			for (size_t uTexDst = 0; uTexDst < pMaterial->textureCoordinators_count; ++uTexDst)
			{
				const usg::exchange::TextureCoordinator& texCo = pMaterial->textureCoordinators[uTexDst];
				// FIXME: Remove hardcoding by adding support for arrays on attributes!
				if (texCo.sourceCoordinate == texUV && uTexDst < 4)
				{
					element.uAttribId = uUVIndex + (uint32)uTexDst;
					memcpy(&elements[uCount], &element, sizeof(VertexElement));
					uCount++;
				}
			};
			texUV++;
		}
		else
		{
			uint32 uAttribId = customFXDecl->GetAttribBinding(pInfo->usageHint);
			if (uAttribId != USG_INVALID_ID)
			{
				// Only bind it if we use it
				element.uAttribId = uAttribId;
				memcpy(&elements[uCount], &element, sizeof(VertexElement));
				uCount++;
			}
		}

		// FIXME: This should be encoded into the declaration to account for padding
		offset += VertexDeclaration::GetByteCount(element.eType) * element.uCount;

		pInfo++;
	}

	// Match the declaration alignment
	offset = AlignSize(offset, 4);

	elements[uCount] = VERTEX_ELEMENT_CAP;

	return uCount+1;
}



void ModelResource::GetSingleAttributeDecl( exchange::VertexAttribute attr, uint32 uCount, VertexElement element[2] )
{
	ASSERT_MSG( attr != exchange::VertexAttribute_NONE, "Wrong attribute!" );
	element[0].bNormalised = false;
	element[0].bIntegerReg = false;
	element[0].eType = VE_FLOAT;
	element[0].uAttribId = g_uAttribId[attr];
	element[0].uCount = uCount;
	element[0].uOffset = 0;
	element[1] = VERTEX_ELEMENT_CAP;
} 

bool ModelResource::GetSingleAttributeDeclDefault(const CustomEffectDecl::Attribute* pAttrib, uint32 uOffset, VertexElement* pElement)
{
	pElement->bNormalised = false;
	pElement->bIntegerReg = false;
	pElement->uAttribId = pAttrib->uIndex;

	switch (pAttrib->eConstantType)
	{
	case CT_VECTOR_4:
		pElement->eType = VE_FLOAT;
		pElement->uCount = 4;
		break;
	case CT_VECTOR_3:
		pElement->eType = VE_FLOAT;
		pElement->uCount = 3;
		break;
	case CT_VECTOR_2:
		pElement->eType = VE_FLOAT;
		pElement->uCount = 2;
		break;
	case CT_FLOAT:
		pElement->eType = VE_FLOAT;
		pElement->uCount = 1;
		break;
	case CT_INT:
		pElement->bIntegerReg = true;
		pElement->eType = VE_INT;
		pElement->uCount = 1;
		break;
	case CT_VECTOR4I:
		pElement->bIntegerReg = true;
		pElement->eType = VE_INT;
		pElement->uCount = 4;
		break;
	case CT_VECTOR2I:
		pElement->bIntegerReg = true;
		pElement->eType = VE_INT;
		pElement->uCount = 2;
		break;
	case CT_VECTOR4U:
		pElement->bIntegerReg = true;
		pElement->eType = VE_UINT;
		pElement->uCount = 4;
		break;		
	default:
		ASSERT(false);
		return false;
	}
	pElement->uOffset = uOffset;
	return true;
}

bool ModelResource::GetSingleAttributeDeclNamed(const CustomEffectResHndl& fxRes, const char* szName, uint32 uCount, VertexElement* pElement)
{
	uint32 uBinding = fxRes->GetAttribBinding(szName);
	if (uBinding != USG_INVALID_ID)
	{
		pElement->bNormalised = false;
		pElement->bIntegerReg = false;
		pElement->eType = VE_FLOAT;
		pElement->uAttribId = uBinding;
		pElement->uCount = uCount;
		pElement->uOffset = 0;
		return true;
	}
	return false;
}


void ModelResource::SetupSkeleton( uint8* p )
{
	usg::exchange::ModelHeader* pHeader = reinterpret_cast<usg::exchange::ModelHeader*>( p );
	if( pHeader->skeletonOffset == 0 ) {
		// doesn't have a Skeleton.
		return;
	}

	uint8* skeletonAddr = p + pHeader->skeletonOffset;
	usg::exchange::Skeleton* pSkeleton = reinterpret_cast<usg::exchange::Skeleton*>( skeletonAddr );
	usg::exchange::Bone* pFirstBone = reinterpret_cast<usg::exchange::Bone*>( skeletonAddr + sizeof( usg::exchange::Skeleton ) );

	m_defaultSkeleton.Init(pSkeleton, pFirstBone);
}

const ModelResource::Mesh* ModelResource::GetMesh(uint32 uMesh) const
{
	ASSERT(uMesh < m_uMeshCount); 
	return &m_meshArray[uMesh];
}


}
