/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Maths/MathUtil.h"
#include "Engine/Resource/ResourceMgr.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/SceneConstantSets.h"
#include "Engine/Core/String/String_Util.h"
#include "Engine/Particles/ParticleMgr.h"
#include "Engine/Graphics/Device/GFXContext.h"
#include "RibbonTrail.h"

namespace usg
{

//usg::FastPool<RibbonTrail>* RibbonTrail::m_spEffectPool = NULL;

struct RibbonMetaData
{
	usg::Color	cStartColor;
	usg::Color	cEndColor;
	float		fLineWidth;  
	float		fInvLinePersist;
	float		fElapsedTime;
};

static const usg::VertexElement g_ribbonVertexElements[] =
{
	VERTEX_DATA_ELEMENT_NAME(0, RibbonTrail::RibbonTrailVertex, vPos, usg::VE_FLOAT, 3, false),
	VERTEX_DATA_ELEMENT_NAME(1, RibbonTrail::RibbonTrailVertex, fEmissionTime, usg::VE_FLOAT, 1, false),
	VERTEX_DATA_ELEMENT_NAME(2, RibbonTrail::RibbonTrailVertex, fEmissionLength, usg::VE_FLOAT, 1, false),
	VERTEX_DATA_END()
};

static const usg::ShaderConstantDecl g_ribbonEffectDecl[] =
{
	SHADER_CONSTANT_ELEMENT(RibbonMetaData, cStartColor,		usg::CT_VECTOR_4, 1),
	SHADER_CONSTANT_ELEMENT(RibbonMetaData, cEndColor,			usg::CT_VECTOR_4, 1),
	SHADER_CONSTANT_ELEMENT(RibbonMetaData, fLineWidth,			usg::CT_FLOAT, 1),
	SHADER_CONSTANT_ELEMENT(RibbonMetaData, fInvLinePersist,	usg::CT_FLOAT, 1),
	SHADER_CONSTANT_ELEMENT(RibbonMetaData, fElapsedTime,		usg::CT_FLOAT, 1),
	SHADER_CONSTANT_END()
};

static const DescriptorDeclaration g_descriptorDecl[] =
{
	DESCRIPTOR_ELEMENT(0,						 DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, SHADER_FLAG_PIXEL),
	DESCRIPTOR_ELEMENT(1,						 DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, SHADER_FLAG_PIXEL),
	DESCRIPTOR_ELEMENT(SHADER_CONSTANT_MATERIAL, DESCRIPTOR_TYPE_CONSTANT_BUFFER, 1, SHADER_FLAG_VS_GS),
	DESCRIPTOR_END()
};


RibbonTrail::RibbonTrail()
{
	m_uNextIndex = 0;
	m_uSetVerts = 0;
	m_uDeclLength = 0;
	m_pOwner = NULL;
	m_fTimeAccum = 0.0f;
	m_fScale = 1.0f;
	m_pCpuData = NULL;

	// Effect layer information
	SetLayer(LAYER_TRANSLUCENT);
	SetPriority(11);

}

RibbonTrail::~RibbonTrail()
{
	if(m_pCpuData)
	{
		vdelete m_pCpuData;
		m_pCpuData = NULL;
	}
	
}

void RibbonTrail::Alloc(usg::GFXDevice* pDevice, ParticleMgr* pMgr, const particles::RibbonData* pDecl, bool bDynamicResize)
{
	uint32 uMaxLength = bDynamicResize ? 1000 : GetRequiredVerts(pDecl->fLifeTime);
	const char* szTexName = "ribbon/round_quarter";
	const char* szPatternName = "ribbon/trail";

	usg::VertexDeclaration decl;
	decl.InitDecl(g_ribbonVertexElements);

	uint32 uVertexSize = sizeof(RibbonTrailVertex);
	m_uMaxLength = uMaxLength;

	m_pOwner = pMgr;

	// +2 for the cap vertices
	m_pCpuData = (RibbonTrailVertex*)usg::mem::Alloc(usg::MEMTYPE_STANDARD, usg::ALLOC_PARTICLES, uVertexSize*(uMaxLength+2));

	usg::ResourceMgr* pRes = usg::ResourceMgr::Inst();

	PipelineStateDecl pipeline;
	pipeline.ePrimType = usg::PT_LINES_ADJ;

	pipeline.inputBindings[0].Init(g_ribbonVertexElements);
	pipeline.uInputBindingCount = 1;

	DescriptorSetLayoutHndl matDescriptors = pDevice->GetDescriptorSetLayout(g_descriptorDecl);
	pipeline.layout.descriptorSets[0] = pDevice->GetDescriptorSetLayout(SceneConsts::g_globalDescriptorDecl);
	pipeline.layout.descriptorSets[1] = matDescriptors;
	pipeline.layout.uDescriptorSetCount = 2;

	usg::AlphaStateDecl& alphaDecl = pipeline.alphaState;
	alphaDecl.SetColor0Only();
	alphaDecl.uColorMask[0]		= usg::RT_MASK_ALL;
	alphaDecl.bBlendEnable		= true;
	alphaDecl.blendEq			= usg::BLEND_EQUATION_ADD;
	alphaDecl.srcBlend			= usg::BLEND_FUNC_ONE;
	alphaDecl.dstBlend			= usg::BLEND_FUNC_ONE_MINUS_SRC_ALPHA;
	
	
	
	usg::DepthStencilStateDecl& depthDecl = pipeline.depthState;
	depthDecl.bDepthWrite		= false;
	depthDecl.bDepthEnable		= true;
	depthDecl.eDepthFunc 		= usg::DEPTH_TEST_LESS;
	depthDecl.bStencilEnable	= false;
	depthDecl.eStencilTest		= usg::STENCIL_TEST_ALWAYS;
	
	usg::RasterizerStateDecl& rasDecl = pipeline.rasterizerState;
	rasDecl.eCullFace	= usg::CULL_FACE_NONE;

	pipeline.pEffect = pRes->GetEffect(pDevice, "Particles.RibbonTrail");

	m_material.Init(pDevice, pDevice->GetPipelineState(pMgr->GetScene()->GetRenderPasses(0).GetRenderPass(*this), pipeline), pDevice->GetDescriptorSetLayout(g_descriptorDecl));
	m_constantSet.Init(pDevice, g_ribbonEffectDecl);

	m_material.SetConstantSet(SHADER_CONSTANT_MATERIAL, &m_constantSet);

	usg::SamplerDecl samp(usg::SAMP_FILTER_LINEAR, usg::SAMP_WRAP_MIRROR);	
	m_material.SetTexture(0, usg::ResourceMgr::Inst()->GetTexture(pDevice, szTexName), pDevice->GetSampler(samp) );
	m_material.SetTexture(1, usg::ResourceMgr::Inst()->GetTexture(pDevice, szPatternName), pDevice->GetSampler(samp));
	m_material.UpdateDescriptors(pDevice);	// We've bound all our resources

	SetDeclaration(pDevice, pDecl);

	// Vertex buffer and index buffers
	uint16 uIndices4 = (uint16)uMaxLength*4;
	uint16* pIndices4;
	usg::ScratchObj<uint16>  scratchIndices4(pIndices4, uIndices4);

	uint16 uIndex = 0;
	for(uint16 i=0; i<(uint16)uMaxLength; i++)
	{
		pIndices4[uIndex++] = i;
		pIndices4[uIndex++] = i+1;
		pIndices4[uIndex++] = i+2;
		pIndices4[uIndex++] = i+3;
	}

	m_indices.Init(pDevice, pIndices4, uIndices4, true );
	// +2 because we have a cap vertex at each end
	m_vertices.Init(pDevice, NULL, uVertexSize, uMaxLength+2, "RibbonTrail", usg::GPU_USAGE_DYNAMIC);

	m_capVertex.vPos.Assign(0.0f, 0.0f, 0.0f);
	m_capVertex.fEmissionTime = -1.0f;
	m_capVertex.fEmissionLength = -1.0f;
}

void RibbonTrail::Cleanup(usg::GFXDevice* pDevice) 
{
	m_material.Cleanup(pDevice);
	m_constantSet.Cleanup(pDevice);
	m_vertices.Cleanup(pDevice);
	m_indices.Cleanup(pDevice);
}


void RibbonTrail::SetScale(float fScale)
{
	m_fScale = m_fBaseLineWidth * fScale;
}

void RibbonTrail::SetDeclaration(GFXDevice* pDevice, const particles::RibbonData* pDecl)
{
	RibbonMetaData* pConsts = m_constantSet.Lock<RibbonMetaData>();
	pConsts->cStartColor = pDecl->cStartColor;
	pConsts->cEndColor = pDecl->cEndColor;
	pConsts->fInvLinePersist = 1.f/pDecl->fLifeTime;
	pConsts->fLineWidth = pDecl->fLineWidth;
	m_constantSet.Unlock();

	m_fBaseLineWidth = pDecl->fLineWidth;
	m_fScale = m_fBaseLineWidth;	// Assume no scale override

	m_uDeclLength = GetRequiredVerts(pDecl->fLifeTime);
	ASSERT(m_uDeclLength<=m_uMaxLength);

	usg::SamplerDecl samp(usg::SAMP_FILTER_LINEAR, usg::SAMP_WRAP_MIRROR);
	usg::U8String name = "ribbon/";
	usg::U8String srcName = pDecl->textureName;
	if (str::StartsWithToken(pDecl->textureName, "ribbon/"))
	{
		srcName.RemovePath();	// FIXME: Hack for old particle data
	}
	name += srcName;
	m_material.SetTexture(1, usg::ResourceMgr::Inst()->GetTexture(pDevice, name.CStr()), pDevice->GetSampler(samp) );
}

void RibbonTrail::Init(usg::GFXDevice* pDevice, const usg::ParticleEffect* pEffect)
{
	m_uNextIndex = 0;
	m_uSetVerts = 0;
	m_fEffectTime = 0.0f;
	m_fCountdown = 1.0f;
	m_fLenAccum = 0.0f;

	Inherited::Init(pDevice, pEffect);

}


bool RibbonTrail::Update(float fElapsed)
{
	if(fElapsed <= 0.0f && m_fEffectTime > 0.0f)
		return false;

	bool bFirst = m_fEffectTime == 0.0f;

	m_fTimeAccum += fElapsed;
	m_fEffectTime += fElapsed;

	RibbonMetaData* pConsts = m_constantSet.Lock<RibbonMetaData>();
	pConsts->fElapsedTime = m_fEffectTime;
	pConsts->fLineWidth = m_fScale;
	m_constantSet.Unlock();

	if(!CanEmit())
	{
		m_fCountdown -= fElapsed;
		return true;
	}

	if (!bFirst && m_fTimeAccum < GetEmissionInterval())
	{
		// High framerate early out
		return true;
	}

	m_fTimeAccum = 0.0f;

	float fLength  = 0.0f;
	usg::Vector3f vPos = GetParent()->GetPosition(1.0f).v3();
	if(!bFirst)
	{
		fLength = vPos.GetDistanceFrom(m_vPrevPos);
		// Discard points which are basically at the same location
		if (usg::Math::IsEqual(fLength, 0.0f))
		{
			return true;
		}
	}
	m_fLenAccum += fLength;
	m_vPrevPos = vPos;

	m_pCpuData[m_uNextIndex].fEmissionTime = m_fEffectTime;
	m_pCpuData[m_uNextIndex].vPos = vPos;
	m_pCpuData[m_uNextIndex].fEmissionLength = m_fLenAccum;

	m_uNextIndex = (m_uNextIndex + 1) % m_uDeclLength;
	m_uSetVerts = usg::Math::Min(m_uSetVerts + 1, m_uDeclLength);


	return true;
}

void RibbonTrail::UpdateBuffers(GFXDevice* pDevice)
{
	usg::VertexBuffer::Lock lock;
	m_vertices.LockData(pDevice, m_uSetVerts + 2, lock);
	RibbonTrailVertex* pDst = (RibbonTrailVertex*)lock.GetData();


	// Copy the memory in such a way that the destination buffer doesn't need indices
	*pDst = m_capVertex;
	pDst++;
	if (m_uSetVerts < m_uDeclLength)
	{
		usg::MemCpy(pDst, m_pCpuData, m_uSetVerts*sizeof(RibbonTrailVertex));
		pDst += m_uSetVerts;
		*pDst = m_capVertex;
	}
	else
	{
		uint32 uFirstIndex = m_uNextIndex;

		//if(m_uNextIndex<(m_uSetVerts-1))
		{
			uint32 uFirstBlockCnt = m_uDeclLength - uFirstIndex;
			usg::MemCpy(pDst, m_pCpuData + uFirstIndex, uFirstBlockCnt*sizeof(RibbonTrailVertex));
			pDst += uFirstBlockCnt;
		}

		if (uFirstIndex > 0)
		{
			usg::MemCpy(pDst, m_pCpuData, uFirstIndex*sizeof(RibbonTrailVertex));
			pDst += uFirstIndex;
		}

		*pDst = m_capVertex;
	}

	m_vertices.Unlock(lock);

	m_constantSet.UpdateData(pDevice);
	m_material.UpdateDescriptors(pDevice);
}

bool RibbonTrail::Draw(GFXContext* pContext, RenderContext& renderContext)
{
	if(m_uSetVerts > 1)
	{
		m_material.Apply(pContext);
		pContext->SetVertexBuffer(&m_vertices, 0);
		pContext->DrawIndexedEx(&m_indices, 0, (m_uSetVerts-1)*4);

		return true;
	}


	return false;
}


bool RibbonTrail::ActiveParticles()
{
	return m_fCountdown > 0.0f;
}

void RibbonTrail::FreeFromPool()
{
	if(m_pOwner)
	{
		m_pOwner->FreeRibbon(this);
	}
}

}