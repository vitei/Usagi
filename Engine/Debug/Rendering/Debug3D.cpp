/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Resource/ResourceMgr.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/SceneConstantSets.h"
#include "Engine/Maths/MathUtil.h"
#include "Engine/Graphics/Device/GFXContext.h"
#include "Engine/Graphics/Device/GFXDevice.h"
#include "Engine/Graphics/StandardVertDecl.h"
#include "Engine/Debug/Rendering/Debug3D.h"
#include "Engine/Scene/ViewContext.h"

namespace usg 
{

Debug3D* Debug3D::m_psRenderer = NULL;

static const VertexElement g_instanceVertex[] =
{
	VERTEX_DATA_ELEMENT_NAME(1, Debug3D::SphereData, vPos, VE_FLOAT, 4, false),
	VERTEX_DATA_ELEMENT_NAME(2, Debug3D::SphereData, vColor, VE_FLOAT, 4, false),
	VERTEX_DATA_END()
};


Debug3D::Debug3D()
{
	m_uSpheres = 0;
	m_uCubes = 0;
	SetLayer(RenderNode::LAYER_TRANSLUCENT);
	SetPriority(0);
}

Debug3D::~Debug3D()
{
	ASSERT(m_psRenderer == this);
	m_psRenderer = NULL;
}

void Debug3D::Init(GFXDevice* pDevice, Scene* pScene)
{
	int i, iC = MAX_SPHERES;
	
	MakeSphere(pDevice);
	

	uint16* pIndices = NULL;
	ScratchObj<uint16> cubeIndices(pIndices, MAX_CUBES, 4);

	iC = MAX_CUBES;
	for(i = 0; i < iC; i++)
	{
		pIndices[i] = i;
	}

	m_cubeVB.Init(pDevice, NULL, sizeof(CubeRender::Cube), MAX_CUBES, "Cube", GPU_USAGE_DYNAMIC);
	m_cubeIB.Init(pDevice, pIndices, MAX_CUBES);



	m_pRenderGroup = pScene->CreateRenderGroup(NULL);

	RenderNode* pNode = this;
	m_pRenderGroup->AddRenderNodes( pDevice, &pNode, 1, 0 );

	m_psRenderer = this;
}

void Debug3D::InitContextData(GFXDevice* pDevice, ViewContext* pContext)
{
	RenderPassHndl rp = pContext->GetRenderPasses().GetRenderPass(*this);


	// Don't write to the depth, but do read from it
	PipelineStateDecl pipelineState;
	//pipelineState.renderPass = pScene->GetRenderPass(0);
	DepthStencilStateDecl& depthDecl = pipelineState.depthState;
	depthDecl.bDepthWrite = false;
	depthDecl.bDepthEnable = true;
	depthDecl.eDepthFunc = DEPTH_TEST_LESS;

	AlphaStateDecl& alphaDecl = pipelineState.alphaState;
	// Make these spheres transparent
	alphaDecl.bBlendEnable = true;
	alphaDecl.srcBlend = BLEND_FUNC_SRC_ALPHA;
	alphaDecl.dstBlend = BLEND_FUNC_ONE_MINUS_SRC_ALPHA;
	alphaDecl.blendEq = BLEND_EQUATION_ADD;

	pipelineState.pEffect = ResourceMgr::Inst()->GetEffect(pDevice, "Debug.Sphere");
	pipelineState.inputBindings[0].Init(GetVertexDeclaration(VT_POSITION));
	pipelineState.inputBindings[1].Init(g_instanceVertex, 1, usg::VERTEX_INPUT_RATE_INSTANCE, 1);
	pipelineState.uInputBindingCount = 2;

	pipelineState.layout.descriptorSets[0] = pDevice->GetDescriptorSetLayout(SceneConsts::g_globalDescriptorDecl);
	pipelineState.layout.uDescriptorSetCount = 1;

	m_spherePipeline = pDevice->GetPipelineState(rp, pipelineState);

	pipelineState.inputBindings[0].Init(CubeRender::VertexElements);
	pipelineState.uInputBindingCount = 1;
	// Initialize cubes

	pipelineState.pEffect = ResourceMgr::Inst()->GetEffect(pDevice, "Debug.CubesOriented");
	pipelineState.ePrimType = PT_POINTS;
	// FIXME: Issue on Vulkan with this effect
	//m_cubePipeline = pDevice->GetPipelineState(rp, pipelineState);
}

void Debug3D::CleanUp(GFXDevice* pDevice)
{
	m_sphereIB.CleanUp(pDevice);
	m_sphereVB.CleanUp(pDevice);
	m_cubeIB.CleanUp(pDevice);
	m_cubeVB.CleanUp(pDevice);
	m_transforms.CleanUp(pDevice);
}

Debug3D* Debug3D::GetRenderer()
{
	return m_psRenderer;
}

void Debug3D::Clear()
{
	m_uSpheres = 0;
	m_uCubes = 0;
}

void Debug3D::AddSphere(const Vector3f &vPos, float fRadius, const Color& color)
{
	if(m_uSpheres >= MAX_SPHERES)
	{
		//ASSERT(false);
		return;
	}
	m_spheres[m_uSpheres].vPos = Vector4f(vPos, fRadius);
	color.FillV4(m_spheres[m_uSpheres].vColor);

	m_uSpheres++;
}

void Debug3D::AddCube(const Matrix4x4& mat, const Color& color)
{
	if(m_uCubes >= MAX_CUBES)
	{
		return;
	}

	CubeRender::Cube& cube = m_cubes[m_uCubes++];
	
	cube.mat = mat;

	cube.r = color.r();
	cube.g = color.g();
	cube.b = color.b();
	cube.a = color.a();

}

void Debug3D::AddLine(const Vector3f& vStart, const Vector3f& vEnd, const Color& color, float fWidth)
{
	if (vStart.GetSquaredDistanceFrom(vEnd) < 0.0001f)
	{
		return;
	}
	Matrix4x4 mat;
	mat.LoadIdentity();
	mat.MakeScale(vEnd.GetDistanceFrom(vStart)*0.5f, fWidth, fWidth);
	Quaternionf qRot;
	qRot.MakeVectorRotation(Vector3f(1, 0, 0), (vEnd - vStart).GetNormalised());
	Matrix4x4 mRot;
	mRot = qRot;
	mat *= mRot;
	mat.SetPos(vStart + 0.5f*(vEnd-vStart));
	AddCube(mat, color);
}

void Debug3D::UpdateBuffers(GFXDevice* pDevice)
{
	if(m_uCubes > 0)
	{
		m_cubeVB.SetContents(pDevice, m_cubes, m_uCubes);
	}

	if (m_uSpheres > 0)
	{
		m_transforms.SetContents(pDevice, m_spheres, m_uSpheres);
	}
}

bool Debug3D::Draw(GFXContext* pContext, RenderContext& renderContext)
{
	if(m_uSpheres != 0 && m_spherePipeline.IsValid())
	{
		pContext->SetPipelineState(m_spherePipeline);
		pContext->SetVertexBuffer(&m_sphereVB, 0);
		pContext->SetVertexBuffer(&m_transforms, 1);
		pContext->DrawIndexedEx(&m_sphereIB, 0, m_sphereIB.GetIndexCount(), m_uSpheres);
	}

	if(m_uCubes != 0 && m_cubePipeline.IsValid())
	{
		pContext->SetPipelineState(m_cubePipeline);
		pContext->SetVertexBuffer(&m_cubeVB);
		pContext->DrawIndexedEx(&m_cubeIB, 0, m_uCubes);
	}

	return true;
}

static inline void sincosf( float angle, float* psin, float* pcos )
{
    *psin = sinf( angle );
    *pcos = cosf( angle );
}

void Debug3D::MakeSphere(GFXDevice* pDevice)
{
	const float fRadius = 1.0f;	// Will be scaled in the vertexShader;
	const uint32 uSlices = 16;
	const uint32 uStacks = 16;

	uint32 uIndices		= (2 * ( uStacks - 1 ) * uSlices)*3;
	uint32 uVertices	= ( uStacks - 1 ) * uSlices + 2;

	PositionVertex* pVertices = NULL;
	ScratchObj<PositionVertex> scratchVertices(pVertices, uVertices, 4);
	uint16* puIndices = NULL;
	ScratchObj<uint16> scratchIndices(puIndices, uIndices, 4);

    float fSinI[uSlices];
	float fCosI[uSlices];
    float fSinJ[uStacks];
	float fCosJ[uStacks];

    for(uint32 i = 0; i < uSlices; i++)
	{
		sincosf(2.0f * Math::pi * i / uSlices, fSinI + i, fCosI + i);
	}

    for(uint32 j = 0; j < uStacks; j++)
	{
        sincosf(Math::pi * j / uStacks, fSinJ + j, fCosJ + j);
	}

    // Generate vertices
    PositionVertex* pVertex = pVertices;

    // +Z pole
    pVertex->x = 0.0f;
	pVertex->y = 0.0f;
	pVertex->z = fRadius;
    pVertex++;

    // Stacks
    for(uint32 j = 1; j < uStacks; j++)
    {
        for(uint32 i = 0; i < uSlices; i++)
        {
            Vector3f norm(fSinI[i]* fSinJ[j], fCosI[i]* fSinJ[j], fCosJ[j]);
			norm.Normalise();	// Shouldn't be necessary, but ensure accuracy
			Vector3f pos = norm*fRadius;

			pVertex->x = pos.x;
			pVertex->y = pos.y;
			pVertex->z = pos.z;

            pVertex++;
        }
    }

    // Z- pole
    pVertex->x = 0.0f;
	pVertex->y =0.0f;
	pVertex->z = -fRadius;
    pVertex++;

    // Generate indices
    uint16* puFace = puIndices;
    uint16 uRowA, uRowB;

    // Z+ pole
    uRowA = 0;
    uRowB = 1;

    for(uint32 i = 0; i < uSlices - 1; i++)
    {
        puFace[2] = (uint16)(uRowA);
        puFace[1] = (uint16)(uRowB + i + 1);
        puFace[0] = (uint16)(uRowB + i);
        puFace += 3;
    }

    puFace[2] = (uint16)(uRowA);
    puFace[1] = (uint16)(uRowB);
    puFace[0] = (uint16)(uRowB + (uSlices-1));
    puFace += 3;

    // Interior stacks
    for(uint32 j = 1; j < uStacks - 1; j++)
    {
        uRowA = 1 + (j - 1) * uSlices;
        uRowB = uRowA + uSlices;

        for(uint32 i = 0; i < uSlices - 1; i++)
        {
            puFace[2] = (uint16)(uRowA + i);
            puFace[1] = (uint16)(uRowA + i + 1);
            puFace[0] = (uint16)(uRowB + i);
            puFace += 3;

            puFace[2] = (uint16)(uRowA + i + 1);
            puFace[1] = (uint16)(uRowB + i + 1);
            puFace[0] = (uint16)(uRowB + i);
            puFace += 3;
        }

        puFace[2] = (uint16)(uRowA + uSlices - 1);
        puFace[1] = (uint16)(uRowA);
        puFace[0] = (uint16)(uRowB + uSlices - 1);
        puFace += 3;

        puFace[2] = (uint16)(uRowA);
        puFace[1] = (uint16)(uRowB);
        puFace[0] = (uint16)(uRowB + uSlices - 1);
        puFace += 3;
    }

    // Z- pole
    uRowA = 1 + (uStacks - 2) * uSlices;
    uRowB = uRowA + uSlices;

    for(uint32 i = 0; i < uSlices - 1; i++)
    {
        puFace[2] = (uint16)(uRowA + i);
        puFace[1] = (uint16)(uRowA + i + 1);
        puFace[0] = (uint16)(uRowB);
        puFace += 3;
    }

    puFace[2] = (uint16)(uRowA + uSlices - 1);
    puFace[1] = (uint16)(uRowA);
    puFace[0] = (uint16)(uRowB);

	m_sphereVB.Init(pDevice, pVertices, sizeof(PositionVertex), uVertices, "DebugSphere");
	m_transforms.Init(pDevice, NULL, sizeof(SphereData), MAX_SPHERES, "DebugSphereTrans", usg::GPU_USAGE_DYNAMIC);
	m_sphereIB.Init(pDevice, puIndices, uIndices);
}

}