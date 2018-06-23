/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/common.h"
#include "Engine/Maths/MathUtil.h"
#include "Engine/Graphics/Textures/Texture.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/SceneConstantSets.h"
#include "Engine/Scene/RenderGroup.h"
#include "Engine/Graphics/GFX.h"
#include "Engine/Graphics/StandardVertDecl.h"
#include "Engine/Graphics/Device/GFXDevice.h"
#include "Engine/Resource/ResourceMgr.h"
#include "Engine/Graphics/Device/GFXContext.h"
#include "Engine/PostFX/PostFXSys.h"
#include "SkyFog.h"

namespace usg {

static const usg::DescriptorDeclaration g_descriptorDecl[] =
{
	DESCRIPTOR_ELEMENT(0,	usg::DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, usg::SHADER_FLAG_PIXEL),
	DESCRIPTOR_END()
};

static const usg::DescriptorDeclaration g_descriptorDeclFade[] =
{
	DESCRIPTOR_ELEMENT(0,	usg::DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, usg::SHADER_FLAG_PIXEL),
	DESCRIPTOR_ELEMENT(5,	usg::DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, usg::SHADER_FLAG_PIXEL),
	DESCRIPTOR_END()
};

SkyFog::SkyFog(void)
{
	m_bValid = false;
	SetLayer(RenderNode::LAYER_SKY);
}

SkyFog::~SkyFog(void)
{

}


void SkyFog::Init(GFXDevice* pDevice, PostFXSys* pSys, RenderTarget* pDst)
{
	m_bUseDepthTex = true;
	m_pDestTarget = pDst;

	MakeSphere(pDevice, 1.0f);
	//MakeCube(pDevice);

	// TODO: Move the depth stencil stuff out of the materials and into the layers?
	PipelineStateDecl pipeline;
	RenderPassHndl renderPassHndl = pDst->GetRenderPass();
	pipeline.inputBindings[0].Init(GetVertexDeclaration(VT_POSITION));
	pipeline.uInputBindingCount = 1;

	DescriptorSetLayoutHndl matDescriptors = pDevice->GetDescriptorSetLayout(g_descriptorDecl);
	DescriptorSetLayoutHndl matDescriptorsFade = pDevice->GetDescriptorSetLayout(g_descriptorDeclFade);
	pipeline.layout.descriptorSets[0] = pDevice->GetDescriptorSetLayout(SceneConsts::g_globalDescriptorDecl);
	pipeline.layout.descriptorSets[1] = matDescriptorsFade;
	pipeline.layout.uDescriptorSetCount = 2;

	DepthStencilStateDecl& depthDecl = pipeline.depthState;
	depthDecl.bDepthWrite	= false;
	depthDecl.bDepthEnable	= true;
	depthDecl.eDepthFunc	= DEPTH_TEST_ALWAYS;//DEPTH_TEST_LEQUAL;
	depthDecl.bStencilEnable= true;
	if(m_bUseDepthTex)
	{
		depthDecl.eStencilTest	= STENCIL_TEST_EQUAL;
	}
	else
	{
		depthDecl.eStencilTest = STENCIL_TEST_ALWAYS;
	}
	
	depthDecl.SetMask(STENCIL_MASK_GEOMETRY, 0, STENCIL_GEOMETRY);

	AlphaStateDecl& alphaDecl = pipeline.alphaState;

	alphaDecl.bBlendEnable = true;
	if(m_bUseDepthTex)
	{
		alphaDecl.srcBlend = BLEND_FUNC_SRC_ALPHA;
		alphaDecl.dstBlend = BLEND_FUNC_ONE_MINUS_SRC_ALPHA;
		alphaDecl.blendEq = BLEND_EQUATION_ADD;
	}
	else
	{
		alphaDecl.dstBlend = BLEND_FUNC_DST_ALPHA;
		alphaDecl.srcBlend = BLEND_FUNC_ONE_MINUS_DST_ALPHA;
		alphaDecl.blendEq = BLEND_EQUATION_ADD;
	}

	//alphaDecl.SetColor0Only();
	alphaDecl.bBlendEnable = true;
	pipeline.pEffect = ResourceMgr::Inst()->GetEffect(pDevice, "FogSphere");

	Material &mat = m_materialFade;
	mat.Init(pDevice, pDevice->GetPipelineState(renderPassHndl, pipeline), matDescriptorsFade);

	SamplerDecl sampDecl(SF_LINEAR, SC_CLAMP);	
	sampDecl.eMipFilter = MF_POINT;
	SamplerHndl linear = pDevice->GetSampler(sampDecl);
	sampDecl.SetFilter(SF_POINT);
	sampDecl.SetClamp(SC_WRAP);
	SamplerHndl point = pDevice->GetSampler(sampDecl);

	Material &mat2 = m_materialNoFade;
	pipeline.pEffect = ResourceMgr::Inst()->GetEffect(pDevice, "FogSphereFar");
	pipeline.layout.descriptorSets[1] = matDescriptors;
	depthDecl.eStencilTest = STENCIL_TEST_NOTEQUAL;
	depthDecl.SetMask(STENCIL_GEOMETRY, 0, STENCIL_GEOMETRY);
	alphaDecl.bBlendEnable = false;
	mat2.Init(pDevice, pDevice->GetPipelineState(renderPassHndl, pipeline), matDescriptors);
	
	// TODO: Set the transform nodes bounding volume (should always pass)
	SamplerDecl depthSamp(SF_POINT, SC_WRAP);
	SamplerDecl colorSamp(SF_LINEAR, SC_WRAP);
	colorSamp.SetClamp(SC_CLAMP);
	m_samplerHndl = pDevice->GetSampler(depthSamp);
	m_linearSampl = pDevice->GetSampler(colorSamp);
}

void SkyFog::CleanUp(GFXDevice* pDevice)
{
	m_materialFade.Cleanup(pDevice);
	m_materialNoFade.Cleanup(pDevice);
	m_vertexBuffer.CleanUp(pDevice);
	m_indexBuffer.CleanUp(pDevice);
	m_bValid = false;
}

void SkyFog::SetDestTarget(GFXDevice* pDevice, RenderTarget* pDst)
{
	m_pDestTarget = pDst;
}

void SkyFog::Resize(GFXDevice* pDevice, uint32 uWidth, uint32 uHeight)
{
	if (m_bValid)
	{
		m_materialFade.UpdateDescriptors(pDevice);
		m_materialNoFade.UpdateDescriptors(pDevice);
	}
}

void SkyFog::SetTexture(GFXDevice* pDevice, const TextureHndl& tex, const TextureHndl& linDepth)
{
	m_materialFade.SetTexture(0, tex, m_linearSampl);
	m_materialFade.SetTexture(5, linDepth, m_samplerHndl);
	m_materialFade.UpdateDescriptors(pDevice);
	m_materialNoFade.SetTexture(0, tex, m_linearSampl);
	m_materialNoFade.UpdateDescriptors(pDevice);
	m_bValid = true;
}

void SkyFog::MakeCube(GFXDevice* pDevice)
{
	PositionVertex verts[8] =
	{
		// Top
		{ -1.0f,  1.0f, -1.0f }, // 0 - BL
		{  1.0f,  1.0f, -1.0f }, // 1 - BR
		{  1.0f,  1.0f,  1.0f }, // 2 - FR
		{ -1.0f,  1.0f,  1.0f }, // 3 - FL
		// Bottom
		{ -1.0f, -1.0f, -1.0f }, // 4 - BL
		{  1.0f, -1.0f, -1.0f }, // 5 - BR
		{  1.0f, -1.0f,  1.0f }, // 6 - FR
		{ -1.0f, -1.0f,  1.0f }, // 7 - FL
	};

	uint16 iIndices[36] = 
	{
		3, 1, 0, 3, 2, 1,    // Top
		4, 3, 0, 4, 7, 3,    // Left
		7, 2, 3, 7, 6, 2,    // Front
		6, 1, 2, 6, 5, 1,    // Right
		5, 0, 1, 5, 4, 0,    // Back
		4, 6, 7, 4, 5, 6     // Bottom
	};

	m_vertexBuffer.Init(pDevice, verts, sizeof(PositionVertex), 8, "Sky");
	m_indexBuffer.Init(pDevice, iIndices, 36, PT_TRIANGLES);
}

void SkyFog::MakeSphere(GFXDevice* pDevice, float fScale)
{
	const float fRadius = fScale;	
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
		Math::SinCos(2.0f * Math::pi * i / uSlices, fSinI[i], fCosI[i]);
	}
	
    for(uint32 j = 0; j < uStacks; j++)
	{
		Math::SinCos(Math::pi * j / uStacks, fSinJ[j], fCosJ[j]);
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

	for (uint32 i = 0; i < uSlices - 1; i++)
	{
		puFace[0] = (uint16)(uRowA);
		puFace[1] = (uint16)(uRowB + i + 1);
		puFace[2] = (uint16)(uRowB + i);
		puFace += 3;
	}

	puFace[0] = (uint16)(uRowA);
	puFace[1] = (uint16)(uRowB);
	puFace[2] = (uint16)(uRowB + (uSlices - 1));
	puFace += 3;

	// Interior stacks
	for (uint32 j = 1; j < uStacks - 1; j++)
	{
		uRowA = 1 + (j - 1) * uSlices;
		uRowB = uRowA + uSlices;

		for (uint32 i = 0; i < uSlices - 1; i++)
		{
			puFace[0] = (uint16)(uRowA + i);
			puFace[1] = (uint16)(uRowA + i + 1);
			puFace[2] = (uint16)(uRowB + i);
			puFace += 3;

			puFace[0] = (uint16)(uRowA + i + 1);
			puFace[1] = (uint16)(uRowB + i + 1);
			puFace[2] = (uint16)(uRowB + i);
			puFace += 3;
		}

		puFace[0] = (uint16)(uRowA + uSlices - 1);
		puFace[1] = (uint16)(uRowA);
		puFace[2] = (uint16)(uRowB + uSlices - 1);
		puFace += 3;

		puFace[0] = (uint16)(uRowA);
		puFace[1] = (uint16)(uRowB);
		puFace[2] = (uint16)(uRowB + uSlices - 1);
		puFace += 3;
	}

	// Z- pole
	uRowA = 1 + (uStacks - 2) * uSlices;
	uRowB = uRowA + uSlices;

	for (uint32 i = 0; i < uSlices - 1; i++)
	{
		puFace[0] = (uint16)(uRowA + i);
		puFace[1] = (uint16)(uRowA + i + 1);
		puFace[2] = (uint16)(uRowB);
		puFace += 3;
	}

	puFace[0] = (uint16)(uRowA + uSlices - 1);
	puFace[1] = (uint16)(uRowA);
	puFace[2] = (uint16)(uRowB);

	m_vertexBuffer.Init(pDevice, pVertices, sizeof(PositionVertex), uVertices, "Sky");
	m_indexBuffer.Init(pDevice, puIndices, uIndices, PT_TRIANGLES);
}


bool SkyFog::Draw(GFXContext* pContext, RenderContext& renderContext)
{
	pContext->BeginGPUTag("Sky");

	// Setting the destination target now handled outside
	//pContext->SetRenderTarget(m_pDestTarget);
	m_materialFade.Apply(pContext);
	pContext->SetVertexBuffer(&m_vertexBuffer);
	pContext->DrawIndexed(&m_indexBuffer);

	m_materialNoFade.Apply(pContext);
	pContext->SetVertexBuffer(&m_vertexBuffer);
	pContext->DrawIndexed(&m_indexBuffer);

	pContext->EndGPUTag();

	
	return true;
}

}
