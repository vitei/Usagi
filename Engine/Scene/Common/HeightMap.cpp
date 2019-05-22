/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Core/Utility.h"
#include "Engine/Core/File/File.h"
#include "Engine/Core/String/U8String.h"
#include "Engine/Graphics/GFX.h"
#include "Engine/Graphics/MeshUtility.h"
#include "Engine/Graphics/Device/GFXDevice.h"
#include "Engine/Graphics/StandardVertDecl.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Memory/Mem.h"
#include "Engine/Memory/ScratchObj.h"
#include "Engine/Resource/ResourceMgr.h"
#include "Engine/Physics/CollisionQuadTree.h"
#include "Engine/Scene/SceneConstantSets.h"
#include "HeightMap.h"

namespace usg{

static const usg::DescriptorDeclaration g_descriptorDecl[] =
{
	DESCRIPTOR_ELEMENT(0,	usg::DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, usg::SHADER_FLAG_PIXEL),
	DESCRIPTOR_ELEMENT(1,	usg::DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, usg::SHADER_FLAG_PIXEL),
	DESCRIPTOR_ELEMENT(2,	usg::DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, usg::SHADER_FLAG_PIXEL),
	DESCRIPTOR_END()
};


static const char* g_texNames[] =
{
	"terrain_low",
	"terrain_top",
	"terrain_detail"
};

HeightMap::HeightMap(void)
{
	m_pRenderGroup = NULL;
	m_fScale = 0.0f;
	m_fOffset = 0.0f;
}

HeightMap::~HeightMap(void)
{
}


bool HeightMap::Load(GFXDevice* pDevice, Scene* pScene, const char* szFileName, uint32 uWidth, uint32 uHeight, uint32 uBpp, float scale, float offset, CollisionQuadTree& pCollisionQuadTree)
{
	ASSERT(uBpp==8);	// Only implemented this so far

	// TODO: Replace with temp stack data
	uint8* heightData = NULL;
	ScratchObj<uint8> scratchHeight(heightData, uWidth*uHeight, FILE_READ_ALIGN);

	U8String fileAndPath = U8String("Terrain/") + szFileName;
	File heightFile(fileAndPath.CStr(), FILE_ACCESS_READ);
	
	if(!heightFile.IsOpen())
	{
		ASSERT(false);
		return false;
	}

	ASSERT(heightFile.GetSize() >= uWidth*uHeight*(uBpp/8));
	heightFile.Read(uWidth*uHeight*(uBpp/8), heightData);

	uint32 noVerts = uWidth*uHeight;
	uint32 noIndices = (uWidth-1)*(uHeight-1)*6;
	PositionNormalUVVertex* pVerts = NULL;
	uint16* pIndices = NULL;
	ScratchObj<PositionNormalUVVertex> scratchVerts(pVerts, noVerts, 4);
	ScratchObj<uint16> scratchIndices(pIndices, noIndices, 4);
	
    
    
	// TODO: Keep record of vertices, throw into texture, collision etc
	// TODO: Evaluate performance of triangle strips against triangles
	uint32 count=0;
	uint32 uIndex = 0;
	for(uint32 x=0; x<uWidth; x++)
	{
		for(uint32 z=0; z<uHeight; z++)
		{
			pVerts[uIndex].x = ((float)x-(uWidth/2))*5.f;
			pVerts[uIndex].z = ((float)z-(uHeight/2))*5.f;
			pVerts[uIndex].y = (((float)heightData[count])*0.196f)-25.0f;

			//pVerts[uIndex].x = -1.0f + (((float)x)/uWidth)*2;
			//pVerts[uIndex].z = 0.0f;
			//pVerts[uIndex].y = -1.0f + (((float)z)/uWidth)*2;

			// Set tex repeat in shader if needs be rather than here
			pVerts[uIndex].nx = 0.0f;
			pVerts[uIndex].ny = 1.0f;
			pVerts[uIndex].nz = 0.0f;

			pVerts[uIndex].u = ((float)x/uWidth)*10.0f;
			pVerts[uIndex].v = ((float)z/uHeight)*10.0f;
			count++;
			uIndex++;
		}
	}

	uint16* pCurrIndice = pIndices;
	//uint32 uIndexCount = 0;
	for(uint32 x=0; x<uWidth-1; x++)
	{
		for(uint32 z=0; z<uHeight-1; z++)
		{
			*pCurrIndice++ = (x*(uWidth))+(z+1);		// LT
			*pCurrIndice++ = ((x+1)*(uWidth))+(z);		// RB
			*pCurrIndice++ = ((x+1)*(uWidth))+(z+1);	// RT
			
			*pCurrIndice++ = (x*(uWidth))+(z+1);		// LT
			*pCurrIndice++ = (x*(uWidth))+(z);			// LB			
			*pCurrIndice++ = ((x+1)*(uWidth))+(z);		// RB
				
		}
	}

	MeshUtl::CalculateNormals(noVerts, noIndices, pVerts, pIndices);

	m_mesh.SetName("Heightmap");
	m_mesh.GetVertexBuffer().Init(pDevice, pVerts, sizeof(PositionNormalUVVertex), uIndex, "Heightmap");
	m_mesh.GetIndexBuffer().Init(pDevice, pIndices, noIndices, PT_TRIANGLES);

	PipelineStateDecl pipeline;
	DepthStencilStateDecl& depthDecl = pipeline.depthState;
	depthDecl.bDepthWrite	= true;
	depthDecl.bDepthEnable	= true;
	depthDecl.eDepthFunc	= DEPTH_TEST_LEQUAL;
	depthDecl.bStencilEnable= true;
	depthDecl.SetOperation(STENCIL_OP_REPLACE, STENCIL_OP_KEEP, STENCIL_OP_KEEP);
	depthDecl.eStencilTest	= STENCIL_TEST_ALWAYS;
	depthDecl.SetMask(0x0, STENCIL_MASK_GEOMETRY, STENCIL_GEOMETRY);    

	pipeline.pEffect = ResourceMgr::Inst()->GetEffect(pDevice, "Heightmap");

	pipeline.inputBindings[0].Init(GetVertexDeclaration(VT_POSITION_NORMAL_UV));
	pipeline.uInputBindingCount = 1;

	DescriptorSetLayoutHndl matDescriptors = pDevice->GetDescriptorSetLayout(g_descriptorDecl);
	pipeline.layout.descriptorSets[0] = pDevice->GetDescriptorSetLayout(SceneConsts::g_globalDescriptorDecl);
	pipeline.layout.descriptorSets[1] = matDescriptors;
	pipeline.layout.uDescriptorSetCount = 2;

	m_mesh.SetPipeline(pDevice->GetPipelineState(pScene->GetRenderPasses(0).GetRenderPass(RenderLayer::LAYER_OPAQUE, 0), pipeline));
	m_mesh.GetDescriptorSet().Init(pDevice, matDescriptors);

    
	SamplerDecl sampDecl(SF_LINEAR, SC_WRAP);
	for(int i=0; i<TERRAIN_TEX_COUNT; i++)
	{
		m_mesh.GetDescriptorSet().SetImageSamplerPair(i, ResourceMgr::Inst()->GetTexture(pDevice, g_texNames[i]), pDevice->GetSampler(sampDecl) );
	}

	Matrix4x4 matTmp;
	matTmp.LoadIdentity();
	m_pTransformNode = pScene->CreateTransformNode();
	m_pTransformNode->SetMatrix( matTmp );
	m_pTransformNode->SetBoundingSphere( Sphere(128.0f*5.0f) );
	m_pRenderGroup = pScene->CreateRenderGroup(m_pTransformNode);

	RenderNode* pNode = &m_mesh;
	m_pRenderGroup->AddRenderNodes( pDevice, &pNode, 1, 0 );
	pNode->SetLayer(RenderLayer::LAYER_OPAQUE);
	pNode->SetPriority(0);

	m_mesh.GetDescriptorSet().UpdateDescriptors(pDevice);

    ASSERT(noIndices % 3 == 0);
    uint32 uTriangles = noIndices / 3;
   // pCollisionQuadTree.Init(pVerts, pIndices, uTriangles, noVerts);

	return true;
}

}
