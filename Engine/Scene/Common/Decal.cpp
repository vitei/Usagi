/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Graphics/StandardVertDecl.h"
#include "Engine/Scene/SceneConstantSets.h"
#include "Engine/Resource/ResourceMgr.h"
#include "Engine/Graphics/Device/GFXDevice.h"
#include "Engine/Graphics/Device/GFXContext.h"
#include "Decal.h"

namespace usg {

struct TextureProjectionConstants
{
	Matrix4x4 ProjectorMatrix;
};

struct FadeConstants
{
	float	fFade;
};

const ShaderConstantDecl g_fadeCBDecl[] =
{
	SHADER_CONSTANT_ELEMENT(FadeConstants, fFade,	CT_FLOAT, 1),
	SHADER_CONSTANT_END()
};


static const ShaderConstantDecl g_textureProjectionConstants[] =
{
	SHADER_CONSTANT_ELEMENT( TextureProjectionConstants, ProjectorMatrix, CT_MATRIX_44, 1 ),
	SHADER_CONSTANT_END()
};

static const DescriptorDeclaration g_descriptorDecl[] =
{
	DESCRIPTOR_ELEMENT(0,							 DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, SHADER_FLAG_PIXEL),
	DESCRIPTOR_ELEMENT(SHADER_CONSTANT_MATERIAL,	 DESCRIPTOR_TYPE_CONSTANT_BUFFER, 1, SHADER_FLAG_VERTEX),
	DESCRIPTOR_ELEMENT(SHADER_CONSTANT_MATERIAL_1,	 DESCRIPTOR_TYPE_CONSTANT_BUFFER, 1, SHADER_FLAG_PIXEL),
	DESCRIPTOR_END()
};


Decal::Decal()
{
	m_pRenderGroup = NULL;
	m_pTransformNode = NULL;
	m_pVertexBuffer = NULL;
	m_indicesNum = 0;
}

Decal::~Decal()
{

}

void Decal::Init(GFXDevice* pDevice, Scene* pScene, TextureHndl pTexture, uint32 uMaxTriangles, float fDepthBias)
{
	const uint32 indicesMax = uMaxTriangles * 3;
	m_uMaxTriangles = uMaxTriangles;

	// Initialise the memory for this decal
	m_indexBuffer.Init( pDevice, (uint16*)NULL, indicesMax, false );


	// Render just before the sky
	SetPriority(127);
	SetLayer(RenderNode::LAYER_OPAQUE);
	

	PipelineStateDecl pipeline;
	pipeline.ePrimType = PT_TRIANGLES;
	pipeline.inputBindings[0].Init(GetVertexDeclaration(VT_POSITION));
	pipeline.uInputBindingCount = 1;

	DescriptorSetLayoutHndl matDescriptors = pDevice->GetDescriptorSetLayout(g_descriptorDecl);
	pipeline.layout.descriptorSets[0] = pDevice->GetDescriptorSetLayout(SceneConsts::g_globalDescriptorDecl);
	pipeline.layout.descriptorSets[1] = matDescriptors;
	pipeline.layout.uDescriptorSetCount = 2;

	DepthStencilStateDecl& depthDecl = pipeline.depthState;
	depthDecl.bDepthWrite = false;
	depthDecl.bDepthEnable = true;
	depthDecl.eDepthFunc = DEPTH_TEST_LEQUAL;
	depthDecl.bStencilEnable = true;
	depthDecl.SetOperation(STENCIL_OP_REPLACE, STENCIL_OP_KEEP, STENCIL_OP_KEEP);
	depthDecl.eStencilTest = STENCIL_TEST_ALWAYS;

	AlphaStateDecl& alphaDecl = pipeline.alphaState;
	alphaDecl.bBlendEnable = true;
	alphaDecl.blendEq = BLEND_EQUATION_ADD;
	alphaDecl.srcBlend = BLEND_FUNC_SRC_ALPHA;
	alphaDecl.dstBlend = BLEND_FUNC_ONE_MINUS_SRC_ALPHA;

	alphaDecl.blendEqAlpha = BLEND_EQUATION_ADD;
	alphaDecl.srcBlendAlpha = BLEND_FUNC_ZERO;
	alphaDecl.dstBlendAlpha = BLEND_FUNC_ONE;

	RasterizerStateDecl& rasterizerDecl = pipeline.rasterizerState;
	rasterizerDecl.bUseDepthBias = true;
	rasterizerDecl.fDepthBias = fDepthBias;
	
	pipeline.pEffect = ResourceMgr::Inst()->GetEffect(pDevice, "TextureProjection");

	m_material.Init( pDevice, pDevice->GetPipelineState(pScene->GetRenderPasses(0).GetRenderPass(*this), pipeline), pDevice->GetDescriptorSetLayout(g_descriptorDecl));

	m_projConsts.Init( pDevice, g_textureProjectionConstants);
	m_pixelConstants.Init(pDevice, g_fadeCBDecl);
	FadeConstants* pColors = m_pixelConstants.Lock<FadeConstants>();
	pColors->fFade = 1.0f;
	m_pixelConstants.Unlock();
	m_pixelConstants.UpdateData(pDevice);

	m_material.SetConstantSet(SHADER_CONSTANT_MATERIAL, &m_projConsts);
	m_material.SetConstantSet(SHADER_CONSTANT_MATERIAL_1, &m_pixelConstants);

	uint32 texIndex = 0;
	SamplerDecl sampDecl( SF_LINEAR, SC_CLAMP );
	sampDecl.eClampU = SC_CLAMP;
	sampDecl.eClampV = SC_CLAMP;
	sampDecl.bProjection = true;
	m_sampler = pDevice->GetSampler(sampDecl);
	SetTexture(pDevice, pTexture);
	//m_material.UpdateDescriptors(pDevice);

}


void Decal::AddToScene(GFXDevice* pDevice, Scene* pScene)
{
	if(!m_pTransformNode)
	{
		m_pTransformNode = pScene->CreateTransformNode();
		m_pRenderGroup = pScene->CreateRenderGroup( m_pTransformNode );

		RenderNode* pNode = (RenderNode*)this;
		m_pRenderGroup->AddRenderNodes(pDevice, &pNode, 1, 0 );
	}

}

void Decal::RemoveFromScene(Scene* pScene)
{
	if (m_pTransformNode)
	{
		m_pRenderGroup->RemoveRenderNode(this);

		if (m_pRenderGroup->IsEmpty())
		{
			pScene->DeleteRenderGroup(m_pRenderGroup);
			pScene->DeleteTransformNode(m_pTransformNode);
			m_pTransformNode = NULL;
			m_pRenderGroup = NULL;
		}
	}
}

void Decal::SetTexture(usg::GFXDevice* pDevice, TextureHndl pTexture)
{
	m_material.SetTexture(0, pTexture, m_sampler);
	m_material.UpdateDescriptors(pDevice);
}


void Decal::SetMatrix(const Matrix4x4& mProj, const Matrix4x4 &mView)
{
	TextureProjectionConstants* p = m_projConsts.Lock<TextureProjectionConstants>();
	Matrix4x4 projScaleTrans;
	projScaleTrans.LoadIdentity();
	projScaleTrans.Scale( 0.5f, -0.5f, 1.0f, 1.0f );
	projScaleTrans.SetTranslation( 0.5f, 0.5f, 0.0f );

	p->ProjectorMatrix = mView * mProj * projScaleTrans;
	m_projConsts.Unlock();
}

void Decal::SetContents( GFXDevice* pDevice, const Sphere* pBounds, const VertexBuffer* pBuffer, const uint16* pIndices, uint32 uIndices )
{
	// Initialize values
	m_pVertexBuffer = pBuffer;

	m_indicesNum = uIndices;
	m_indexBuffer.SetContents( pDevice, (void*)pIndices, uIndices );

	Matrix4x4 matTmp;
	matTmp.LoadIdentity();
	m_pTransformNode->SetMatrix( matTmp );

	m_pTransformNode->SetBoundingSphere( *pBounds );
	m_pTransformNode->SetMatrix( Matrix4x4::Identity() );
}

bool Decal::Draw( GFXContext* pContext, PostFXSys* pPostFXSys )
{
	if(m_indicesNum)
	{
		const RenderGroup* pParent = GetParent();

		m_material.Apply(pContext);
		pContext->SetVertexBuffer(m_pVertexBuffer);

		// "indexBuffer" allocated its buffer by maximum numbers of indices for replacement.
		// Then, must draw with the member variable "indicesNum" which contains actual numbers of indices.
		pContext->DrawIndexedEx(&m_indexBuffer, 0, m_indicesNum);
	}


	return true;
}


void Decal::RenderPassChanged(GFXDevice* pDevice, uint32 uContextId, const RenderPassHndl &renderPass)
{
	
}

void Decal::SetOpacity( float opacity )
{
	FadeConstants* pColors = m_pixelConstants.Lock<FadeConstants>();
	pColors->fFade = opacity;
	m_pixelConstants.Unlock();
}

void Decal::UpdateBuffers(GFXDevice* pDevice)
{
	m_pixelConstants.UpdateData(pDevice);
	m_projConsts.UpdateData(pDevice);
}

}