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



static RenderNode::Layer layerMapping[] = 
{
	RenderNode::LAYER_OPAQUE,
	RenderNode::LAYER_TRANSLUCENT,
	RenderNode::LAYER_SUBTRACTIVE,
	RenderNode::LAYER_ADDITIVE
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

	if( rasterizer.blendMode == exchange::Rasterizer_BlendMode_BLEND_MODE_COLOR )
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

ModelResource::ModelResource()
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

	//szFileName = "Models\\t1ply_m016_devilcatSwitch\\DevilCat.vmdn";

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


void ModelResource::CleanUp(GFXDevice* pDevice)
{
	for (uint32 i = 0; i < m_uMeshCount; i++)
	{
		m_meshArray[i].CleanUp(pDevice);
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
		decl[uIndex].uBinding = SHADER_CONSTANT_INSTANCE;
		uIndex++;
	}
	else
	{
		// Still enabling this separately
		decl[uIndex].eDescriptorType = DESCRIPTOR_TYPE_CONSTANT_BUFFER;
		decl[uIndex].shaderType = SHADER_FLAG_VERTEX;
		decl[uIndex].uCount = 1;
		decl[uIndex].uBinding = SHADER_CONSTANT_BONES;
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

	U8String name = "CustomFX/";
	name += pMaterial->customEffectName;
	name += ".cfx";
	m_meshArray[m_uMeshCount].effectRuntime.Init(pDevice, name.CStr());


	CustomEffectRuntime& fxRunTime = m_meshArray[m_uMeshCount].effectRuntime;

	PipelineStateDecl pipelineState;
	pipelineState.ePrimType = PT_TRIANGLES;

	ASSERT(uCount <= maxElements);
	memsize uVertexSize;
	PipelineStateDecl::InputBinding* bindings = pipelineState.inputBindings;
	VertexElement* pElement = m_meshArray[m_uMeshCount].vertexElements;
	uint32 elementOffset = GetModelDeclUVReusse(pShape, fxRunTime, pMaterial, pElement, uVertexSize);
	bindings[0].Init(pElement);
	bindings[0].uVertexSize = (uint32)uVertexSize;
	pElement += elementOffset;
	pipelineState.uInputBindingCount = 1;


	DescriptorSetLayoutHndl matDescriptors = GetDeclarationLayout(pDevice, pMaterial, pShape->skinningType != usg::exchange::SkinningType_NO_SKINNING);
	pipelineState.layout.descriptorSets[0] = pDevice->GetDescriptorSetLayout(SceneConsts::g_globalDescriptorDecl);
	pipelineState.layout.descriptorSets[1] = matDescriptors;
	pipelineState.layout.uDescriptorSetCount = 2;

	m_meshArray[m_uMeshCount].defaultPipelineDescLayout = matDescriptors;

	GPULocation eGPULocation = bFastMem ? GPU_LOCATION_FASTMEM : GPU_LOCATION_STANDARD;
	// Seem to be running short on VRAM and it's not impacting performance any
	GPULocation eVertGPULocation = GPU_LOCATION_STANDARD;//bFastMem ? GPU_LOCATION_FASTMEM : GPU_LOCATION_STANDARD;
	
	// set vertex stream
	uint32 vertexStreamOffset = calcIndexStreamSizeSum( pShape );
	m_meshArray[m_uMeshCount].vertexBuffer[0].Init(pDevice, pIndexStream + vertexStreamOffset, (uint32)uVertexSize, pShape->vertexNum, pMaterial->customEffectName, GPU_USAGE_STATIC, eVertGPULocation );
	
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


	EffectHndl pEffect;
	EffectHndl pDeferredEffect;

	U8String effectPath = fxRunTime.GetResource()->GetEffectName();
	U8String deferredEffectPath = fxRunTime.GetResource()->GetDeferredEffectName();

	
	if (pShape->singleAttributes_count != 0)
	{
		// Single attributes
		uint32 uSlot = 0;
		for (size_t i = 0; i < pShape->singleAttributes_count; ++i)
		{
			if (!GetSingleAttributeDeclNamed(fxRunTime, pShape->singleAttributes[i].usageHint, pShape->singleAttributes[i].columns, pElement))
				continue;
		
			bindings[uSlot + 1].Init(pElement, (uint32)uSlot + 1, VERTEX_INPUT_RATE_INSTANCE, (uint32)(-1));
			pipelineState.uInputBindingCount++;
			pElement += 2;

			const Vector4f& value = pShape->singleAttributes[i].value;
			m_meshArray[m_uMeshCount].vertexBuffer[1 + uSlot].Init(pDevice, (void*)&value, sizeof(float)*pShape->singleAttributes[i].columns, 1, pMaterial->customEffectName, GPU_USAGE_CONST_REG);
			uSlot++;
		}
	}

	pEffect = ResourceMgr::Inst()->GetEffect(pDevice, effectPath.CStr());
	pDeferredEffect = ResourceMgr::Inst()->GetEffect(pDevice, deferredEffectPath.CStr());


	DepthStencilStateDecl& depthDecl = pipelineState.depthState;
	depthDecl.bDepthWrite = ( pMaterial->rasterizer.attribute & 1 << exchange::Rasterizer_Attribute_DEPTH_TEST_WRITE ? true : false );
	depthDecl.bDepthEnable = ( pMaterial->rasterizer.attribute & 1 << exchange::Rasterizer_Attribute_DEPTH_TEST_ENABLE ? true : false );
	depthDecl.eDepthFunc = static_cast<DepthTest>( pMaterial->rasterizer.depthTestFunc );
	SetupDepthStencilStateDecl( depthDecl, pMaterial->rasterizer.stencilTest );
	depthDecl.SetMask(0x0, STENCIL_MASK_GEOMETRY, STENCIL_GEOMETRY);
	depthDecl.SetOperation(STENCIL_OP_REPLACE, STENCIL_OP_KEEP, STENCIL_OP_KEEP);
	depthDecl.bStencilEnable = true;
	m_meshArray[m_uMeshCount].bEnableFog = pMaterial->attribute.isFogEnable != 0;
	m_meshArray[m_uMeshCount].uFogIndex = pMaterial->attribute.fogIndex;

	AlphaStateDecl& alphaDecl = pipelineState.alphaState;
	SetupAlphaStateDecl( alphaDecl, pMaterial->rasterizer );
	m_meshArray[m_uMeshCount].layer = layerMapping[pMaterial->attribute.translucencyKind];
	m_meshArray[m_uMeshCount].priority = pInitialMesh[meshIndex].renderPriority;
	m_meshArray[m_uMeshCount].bCanFade = (pMaterial->rasterizer.colorMask & usg::RT_MASK_RGB)!=0;

	RasterizerStateDecl& rasterizerDecl = pipelineState.rasterizerState;
	rasterizerDecl.eCullFace = static_cast<CullFace>( pMaterial->rasterizer.cullFace );
	rasterizerDecl.bUseDepthBias = ( pMaterial->rasterizer.isPolygonOffsetEnable ? true : false );
	rasterizerDecl.fDepthBias = static_cast<float>( pMaterial->rasterizer.polygonOffsetUnit );

	pipelineState.pEffect = pEffect;
	m_meshArray[m_uMeshCount].matName = pMaterial->materialName;


	m_meshArray[m_uMeshCount].pipelines.defaultPipeline = pipelineState;

	pipelineState.pEffect = pDeferredEffect;

	m_meshArray[m_uMeshCount].pipelines.deferredPipeline = pipelineState;

	
	for (uint32 i = 0; i < pMaterial->constants_count; i++)
	{
		// Override the default values if we have them
		void* pData = (void*)(((uint8*)pMaterial->constantData) + pMaterial->constants[i].uOffset);
		fxRunTime.SetSetData(i, pData, pMaterial->constants[i].uSize);
	}
	// The w is scaling of the vertex color, if 0 the vertex color is ignored
	Vector4f vTmpVec(1.f, (float)GetBoneIndexCount(pShape), HasAttribute(pShape->streamInfo, exchange::VertexAttribute_TANGENT, pShape->streamInfo_count) ? 1.0f : 0.0f, GetStreamScaling(pShape->streamInfo, pShape->streamInfo_count, usg::exchange::VertexAttribute_BONE_WEIGHT));
	fxRunTime.SetVariable("vScaling0", &vTmpVec);
	fxRunTime.SetVariable("iBoneCount", GetBoneIndexCount(pShape));
	fxRunTime.SetVariable("bAlwaysTrue", true);

	// TODO: Load these from file
	fxRunTime.SetVariable("vScaling1", Vector4f(1.f, 1.f, 1.f, pShape->vertexAlphaScale));

	// Texture Coordinator
	m_meshArray[m_uMeshCount].uUVCount = pMaterial->textureCoordinators_count;
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

				// FIXME: HACK HACK HACK!!!! HACK FOR TESTING!!!
				if (strcmp(texture.textureHint, "reflection0") == 0)
				{
					pathName = "textures/sky_fine01";
				}
				m_meshArray[m_uMeshCount].pTextures[uIndex] = ResourceMgr::Inst()->GetTextureAbsolutePath(pDevice, pathName.CStr(), eGPULocation);

				m_meshArray[m_uMeshCount].samplers[uIndex] = pDevice->GetSampler(sampDecl);
			}
		}
	}

	CreateDepthPassMaterial(pDevice, meshIndex, pShape, pMaterial, effectPath);
}


void ModelResource::CreateDepthPassMaterial(GFXDevice* pDevice, uint32 uMeshIndex, exchange::Shape* pShape, exchange::Material* pMaterial, const U8String& effectName)
{
	PipelineStateDecl pipelineState = m_meshArray[m_uMeshCount].pipelines.defaultPipeline;

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

	CustomEffectRuntime& fxRunTime = m_meshArray[m_uMeshCount].effectRuntime;
	U8String effectPath = fxRunTime.GetResource()->GetDepthEffectName();

	pipelineState.pEffect = ResourceMgr::Inst()->GetEffect(pDevice, effectPath.CStr());

	alphaDecl.bBlendEnable = false;
	uint32 uRenderMask = alphaDecl.uColorMask[0];
	for (uint32 i = 0; i < MAX_COLOR_TARGETS; i++)
	{
		alphaDecl.uColorMask[i] = usg::RT_MASK_NONE;
	}

	m_meshArray[m_uMeshCount].pipelines.depthPassPipeline = pipelineState;

	U8String omniDepthName = fxRunTime.GetResource()->GetOmniDepthEffectName();
	pipelineState.pEffect = ResourceMgr::Inst()->GetEffect(pDevice, omniDepthName.CStr());

	globalDescriptors = pDevice->GetDescriptorSetLayout(SceneConsts::g_omniShadowGlobalDescriptorDecl);
	pipelineState.layout.descriptorSets[0] = globalDescriptors;


	m_meshArray[m_uMeshCount].pipelines.omniDepthPassPipeline = pipelineState;

	pipelineState.pEffect = ResourceMgr::Inst()->GetEffect(pDevice, effectName.CStr());
	pipelineState = m_meshArray[m_uMeshCount].pipelines.defaultPipeline;
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

	m_meshArray[m_uMeshCount].pipelines.translucentStateCmp = pipelineState;
}

uint32 ModelResource::GetInstanceDecl()
{
	VertexDeclaration instDecl;
	instDecl.InitDecl(g_instanceElements, 1);
	uint32 uDeclId = VertexDeclaration::GetDeclId(instDecl);
	return uDeclId;
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

uint32 ModelResource::GetModelDeclUVReusse(const exchange::Shape* pShape, const CustomEffectRuntime& runTime, const exchange::Material* pMaterial, VertexElement elements[], memsize& offset)
{
	const exchange::VertexStreamInfo* pInfo = pShape->streamInfo;

	uint32 uCount = 0;
	const uint32 mask = 0xf;
	int texUV = 0;
	offset = 0;
	uint32 uUVIndex = runTime.GetResource()->GetAttribBinding("uv0");
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
				if (texCo.sourceCoordinate == texUV)
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
			uint32 uAttribId = runTime.GetResource()->GetAttribBinding(pInfo->usageHint);
			ASSERT(uAttribId != USG_INVALID_ID);
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


bool ModelResource::GetSingleAttributeDeclNamed(const CustomEffectRuntime& runTime, const char* szName, uint32 uCount, VertexElement element[2])
{
	uint32 uBinding = runTime.GetResource()->GetAttribBinding(szName);
	if (uBinding != USG_INVALID_ID)
	{
		element[0].bNormalised = false;
		element[0].bIntegerReg = false;
		element[0].eType = VE_FLOAT;
		element[0].uAttribId = uBinding;
		element[0].uCount = uCount;
		element[0].uOffset = 0;
		element[1] = VERTEX_ELEMENT_CAP;
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
