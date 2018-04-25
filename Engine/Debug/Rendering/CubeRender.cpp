/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Core/Utility.h"
#include "Engine/Core/File/File.h"
#include "Engine/Core/String/U8String.h"
#include "Engine/Graphics/GFX.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Memory/Mem.h"
#include "Engine/Memory/ScratchObj.h"
#include "Engine/Resource/ResourceMgr.h"
#include "Engine/Graphics/Device/GFXDevice.h"
#include "Engine/Memory/Mem.h"
#include "Engine/Maths/MathUtil.h"
#include "Engine/Scene/SceneConstantSets.h"
#include "CubeRender.h"

namespace usg
{

const VertexElement CubeRender::VertexElements[]=
{
    {
		// Matrix 4x3
        0,	// szInditifier
        0,			// uOffset
        VE_FLOAT,	//eType;
        12,			//uCount;
        false		// bNormalised
    },
    {
        3,		// szInditifier
        48,				// uOffset
        VE_FLOAT,		//eType;
        4,				//uCount;
        false			// bNormalised
    },
	VERTEX_DATA_END() 
};


static bool g_doneInit = false;


CubeRender::CubeRender(void)
{
	m_pScene = NULL;
    m_pRenderGroup = NULL;
    m_uNumCubes = 0;
    m_pVerts = NULL;
    m_mesh.SetMaxCount(0);
}

CubeRender::~CubeRender(void)
{
	Remove();
	if(m_pVerts)
	{
		vdelete[] m_pVerts;
		m_pVerts = NULL;
	}
}

void CubeRender::FreeAllocation()
{
	m_uNumCubes = 0;
}

void CubeRender::Remove()
{
	if (m_pScene && m_pRenderGroup)
	{
		m_pScene->DeleteRenderGroup(m_pRenderGroup);
		m_pRenderGroup = NULL;
        m_uNumCubes = 0;
        m_mesh.SetMaxCount(m_uNumCubes);
	}
}

void CubeRender::Create(bool bHideInside)
{
	if (m_pScene && !m_pRenderGroup)
	{
		m_pRenderGroup = m_pScene->CreateRenderGroup(NULL);

		RenderNode* pNode = &m_mesh;
		m_pRenderGroup->AddRenderNodes(&pNode, 1, 0);
	}
}

void CubeRender::Clear()
{
    m_uNumCubes = 0;
    m_mesh.SetMaxCount(m_uNumCubes);
}

void CubeRender::Flush(GFXDevice* pDevice)
{
    m_mesh.GetVertexBuffer().SetContents(pDevice, m_pVerts, m_uNumCubes);
    m_mesh.SetMaxCount(m_uNumCubes);
}

void CubeRender::AddCube(const Matrix4x4& mat, const Color& clr)
{
    if (m_uNumCubes < m_uMaxCubes)
    {
        m_pVerts[m_uNumCubes].mat = mat;

        m_pVerts[m_uNumCubes].r = clr.r();
        m_pVerts[m_uNumCubes].g = clr.g();
        m_pVerts[m_uNumCubes].b = clr.b();
        m_pVerts[m_uNumCubes].a = clr.a();

        m_uNumCubes++;
    }
}


bool CubeRender::Init(GFXDevice* pDevice, Scene* pScene, uint32 uMaxCubes, bool bHideInside)
{
	m_pScene = pScene;
	m_pRenderGroup = NULL;
   
    
    m_uMaxCubes = uMaxCubes;
    
	uint16* pIndices = NULL;
	ScratchObj<uint16> scratchIndices(pIndices, uMaxCubes, 4);

	ASSERT(m_pVerts == NULL);
    m_pVerts = vnew(ALLOC_GEOMETRY_DATA) Cube[uMaxCubes];
    
    
    for(uint32 i=0; i<uMaxCubes; i++)
	{
		pIndices[i] = i;
	}
    
	m_mesh.GetVertexBuffer().Init(pDevice, m_pVerts, sizeof(Cube), uMaxCubes, "Cube", GPU_USAGE_DYNAMIC);
	m_mesh.GetIndexBuffer().Init(pDevice, pIndices, uMaxCubes, PT_POINTS); 
        
 
	PipelineStateDecl pipelineDecl;
	pipelineDecl.renderPass = pScene->GetRenderPass(0);
	pipelineDecl.inputBindings[0].Init(VertexElements);
	pipelineDecl.uInputBindingCount = 1;

	pipelineDecl.layout.descriptorSets[0] = pDevice->GetDescriptorSetLayout(SceneConsts::g_globalDescriptorDecl);
	pipelineDecl.layout.uDescriptorSetCount = 1;

	DepthStencilStateDecl& depthDecl = pipelineDecl.depthState;
	depthDecl.bDepthWrite	= true;
	depthDecl.bDepthEnable	= true;
	depthDecl.eDepthFunc	= DEPTH_TEST_LEQUAL;
	depthDecl.bStencilEnable= true;
	depthDecl.SetOperation(STENCIL_OP_REPLACE, STENCIL_OP_REPLACE, STENCIL_OP_REPLACE);
	depthDecl.eStencilTest	= STENCIL_TEST_ALWAYS;
	depthDecl.SetMask(0x0, STENCIL_MASK_GEOMETRY, STENCIL_GEOMETRY);
    
	AlphaStateDecl& alphaDecl = pipelineDecl.alphaState;
    
	pipelineDecl.pEffect = ResourceMgr::Inst()->GetEffect(pDevice, "CubesOriented");
	m_mesh.SetPipeline(pDevice->GetPipelineState(pipelineDecl));
    
	RenderNode* pNode = &m_mesh;
	pNode->SetLayer(RenderNode::LAYER_OPAQUE);
	pNode->SetPriority(0);
	
	
	if (bHideInside)
		pNode->SetRenderMask(RenderNode::RENDER_MASK_OUTSIDE);
	else
		pNode->SetRenderMask(RenderNode::RENDER_MASK_INSIDE | RenderNode::RENDER_MASK_OUTSIDE);
	
    Clear();
    
	return true;
}

}	//	namespace usg