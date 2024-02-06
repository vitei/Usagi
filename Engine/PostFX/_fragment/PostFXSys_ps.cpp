/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Resource/ResourceMgr.h"
#include "Engine/Maths/MathUtil.h"
#include "Bloom.h"
#include "FXAA.h"
#include "SMAA.h"
#include "FilmGrain.h"
#include "ASSAO.h"
#include "LinearDepth.h"
#include "SetSceneTarget.h"
#include "DeferredShading.h"
#include "SkyFog.h"
#include "Engine/Scene/SceneConstantSets.h"
#include "Engine/PostFX/PostFXSys.h"
#include "Engine/Graphics/StandardVertDecl.h"
#include "Engine/Graphics/Device/GFXDevice.h"
#include "Engine/Graphics/Device/GFXContext.h"
#include API_HEADER(Engine/Graphics/Device, GFXDevice_ps.h)
#include "Engine/Core/stl/vector.h"

namespace usg {

inline int CompareNodes(const void* a, const void* b) 
{
	const PostEffect* arg1 = *(const PostEffect**)(a);
	const PostEffect* arg2 = *(const PostEffect**)(b);
	if (arg1->GetLayer() > arg2->GetLayer()) return 1;
	if (arg1->GetLayer() == arg2->GetLayer())
	{
		if( arg1->GetPriority() > arg2->GetPriority())
			return 1;
		if (arg1->GetPriority() < arg2->GetPriority())
			return -1;

		return 0;
	}
	return -1;
}

#define GAUSS_SAMPLE_COUNT			13
#define DOWNSCALE44_SAMPLE_COUNT	4

struct GaussConstants
{
	Vector4f	vOffsets[GAUSS_SAMPLE_COUNT];
};

struct Downscale44Constants
{
	Vector4f	vOffsets[DOWNSCALE44_SAMPLE_COUNT];
};

static const ShaderConstantDecl g_gaussConstantDef[] = 
{
	SHADER_CONSTANT_ELEMENT( GaussConstants, vOffsets,	CT_VECTOR_4,	GAUSS_SAMPLE_COUNT ),
	SHADER_CONSTANT_END()
};

static const ShaderConstantDecl g_downscale44ConstantDef[] = 
{
	SHADER_CONSTANT_ELEMENT( Downscale44Constants, vOffsets,	CT_VECTOR_4,	DOWNSCALE44_SAMPLE_COUNT ),
	SHADER_CONSTANT_END()
};

static const DescriptorDeclaration g_multiSampleDescriptor[] =
{
	DESCRIPTOR_ELEMENT(0,						 DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, SHADER_FLAG_PIXEL),
	DESCRIPTOR_ELEMENT(SHADER_CONSTANT_MATERIAL, DESCRIPTOR_TYPE_CONSTANT_BUFFER, 1, SHADER_FLAG_ALL),
	DESCRIPTOR_END()
};

static const DescriptorDeclaration g_singleSampleDescriptor[] =
{
	DESCRIPTOR_ELEMENT(0,						 DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, SHADER_FLAG_PIXEL),
	DESCRIPTOR_END()
};

PostFXSys_ps::PostFXSys_ps()
	: m_pDefaultEffects{nullptr}
{
	m_uDefaultEffects = 0;
	m_pFXAA = nullptr;
	m_pSMAA = nullptr;
	m_pFilmGrain = nullptr;
	m_pSSAO = nullptr;
	m_pBloom = nullptr;
	m_pSkyFog = nullptr;
	m_pDeferredShading = nullptr;
	m_pSetNoDepthTarget = nullptr;
	m_pSetLinDepthTarget = nullptr;
	m_fPixelScale = 1.0f;
	m_bHDROut = false;
}

PostFXSys_ps::~PostFXSys_ps()
{
	for (uint32 i = 0; i < m_uDefaultEffects; i++)
	{
		vdelete m_pDefaultEffects[i];
	}
}

void PostFXSys_ps::Update(Scene* pScene, float fElapsed)
{
	for (uint32 i = 0; i < m_uDefaultEffects; i++)
	{
		m_pDefaultEffects[i]->Update(pScene,fElapsed);
	}
}

void PostFXSys_ps::UpdateGPU(GFXDevice* pDevice)
{
	// If effects get out of hand we might want to use callbacks, but this is fine for now
	for (auto cfx : m_customEffects)
	{
		auto itr = eastl::find(m_activeEffects.begin(), m_activeEffects.end(), cfx);
		bool bEnabled = itr != m_activeEffects.end();
		if (bEnabled != cfx->GetEnabled())
		{
			EnableEffectsInt(pDevice, m_pParent->GetEnabledEffectFlags());
			break;
		}
	}

	for (uint32 i = 0; i < m_uDefaultEffects; i++)
	{
		if(m_pDefaultEffects[i]->GetEnabled())
		{
			m_pDefaultEffects[i]->UpdateBuffer(pDevice);
		}
	}
}

void PostFXSys_ps::Init(PostFXSys* pParent, ResourceMgr* pResMgr, GFXDevice* pDevice, uint32 uInitFlags, uint32 uWidth, uint32 uHeight)
{
	m_pParent = pParent;

	uint32 uFinalTransferFlags = (uInitFlags & PostFXSys::EFFECT_OFFSCREEN_TARGET) != 0 ? TU_FLAG_SHADER_READ : TU_FLAG_TRANSFER_SRC;
	bool bDeferred = (uInitFlags&PostFXSys::EFFECT_DEFERRED_SHADING)!=0;
	SampleCount eSamples = SAMPLE_COUNT_1_BIT;			

	m_colorBuffer[BUFFER_HDR_0].Init(pDevice, uWidth, uHeight, ColorFormat::RGB_HDR, eSamples, uFinalTransferFlags | TU_FLAGS_OFFSCREEN_COLOR, 0);
	m_colorBuffer[BUFFER_HDR_1].Init(pDevice, uWidth, uHeight, ColorFormat::RGB_HDR, eSamples, uFinalTransferFlags | TU_FLAGS_OFFSCREEN_COLOR, 0);
	m_colorBuffer[BUFFER_LIN_DEPTH].Init(pDevice, uWidth, uHeight, ColorFormat::R_16F, eSamples, TU_FLAGS_OFFSCREEN_COLOR, 1);

	// We don't have enough memory for 1080p if we put this in fast memory too
	m_colorBuffer[BUFFER_LDR_0].Init(pDevice, uWidth, uHeight, ColorFormat::RGBA_8888, SAMPLE_COUNT_1_BIT, uFinalTransferFlags |TU_FLAGS_OFFSCREEN_COLOR);
	m_colorBuffer[BUFFER_LDR_1].Init(pDevice, uWidth, uHeight, ColorFormat::RGBA_8888, SAMPLE_COUNT_1_BIT, uFinalTransferFlags |TU_FLAGS_OFFSCREEN_COLOR);


	m_depthStencil.Init(pDevice, uWidth, uHeight, DepthFormat::DEPTH_24_S8, eSamples, TU_FLAGS_DEPTH_BUFFER | ((uInitFlags&PostFXSys::EFFECT_SMAA) ? TU_FLAG_SHADER_READ : 0));


	if(bDeferred)
	{
		m_colorBuffer[BUFFER_DIFFUSE].Init(pDevice, uWidth, uHeight, ColorFormat::RGBA_8888, SAMPLE_COUNT_1_BIT, TU_FLAGS_OFFSCREEN_COLOR, 0); // 4th component is specular power
		m_colorBuffer[BUFFER_NORMAL].Init(pDevice, uWidth, uHeight, ColorFormat::NORMAL, SAMPLE_COUNT_1_BIT, TU_FLAGS_OFFSCREEN_COLOR, 2);
		m_colorBuffer[BUFFER_EMISSIVE].Init(pDevice, uWidth, uHeight, ColorFormat::RGBA_8888, SAMPLE_COUNT_1_BIT, TU_FLAGS_OFFSCREEN_COLOR, 3);
		m_colorBuffer[BUFFER_SPECULAR].Init(pDevice, uWidth, uHeight, ColorFormat::RGBA_5551, SAMPLE_COUNT_1_BIT, TU_FLAGS_OFFSCREEN_COLOR, 4);
	}
	


	SamplerDecl pointDecl(SAMP_FILTER_POINT, SAMP_WRAP_CLAMP);
	SamplerDecl linearDecl(SAMP_FILTER_LINEAR, SAMP_WRAP_CLAMP);

#if 0
	PipelineStateDecl pipelineDecl;
	pipelineDecl.inputBindings[0].Init(GetVertexDeclaration(VT_POSITION));
	pipelineDecl.uInputBindingCount = 1;

	DescriptorSetLayoutHndl multiDesc = pDevice->GetDescriptorSetLayout(g_multiSampleDescriptor);
	DescriptorSetLayoutHndl singleDesc = pDevice->GetDescriptorSetLayout(g_singleSampleDescriptor);
	pipelineDecl.layout.descriptorSets[0] = multiDesc;
	pipelineDecl.layout.uDescriptorSetCount = 1;

	pipelineDecl.pEffect = pResMgr->GetEffect(pDevice, "PostProcess.Downscale2x2");
	m_downscale4x4Effect = pDevice->GetPipelineState(pipelineDecl);
	pipelineDecl.pEffect = pResMgr->GetEffect(pDevice, "PostProcess.Gauss5x5");
	m_gaussBlur5x5Effect = pDevice->GetPipelineState(pipelineDecl);

	pipelineDecl.layout.descriptorSets[0] = singleDesc;

	pipelineDecl.pEffect = pResMgr->GetEffect(pDevice, "PostProcess.CopyScreen");
	m_downscale2x2Effect = pDevice->GetPipelineState(pipelineDecl);
#endif

	m_pointSampler	= pDevice->GetSampler(pointDecl);
	m_linearSampler = pDevice->GetSampler(linearDecl);


	// Register the default effects
	if(uInitFlags & PostFXSys::EFFECT_FXAA)
	{
		m_pFXAA = vnew(ALLOC_OBJECT) FXAA();
		m_pDefaultEffects[m_uDefaultEffects++] = m_pFXAA;
	}
	if (uInitFlags & PostFXSys::EFFECT_SMAA)
	{
		m_pSMAA = vnew(ALLOC_OBJECT) SMAA();
		m_pDefaultEffects[m_uDefaultEffects++] = m_pSMAA;
	}
	if(uInitFlags & PostFXSys::EFFECT_BLOOM)
	{
		m_pBloom = vnew(ALLOC_OBJECT) Bloom();
		m_pDefaultEffects[m_uDefaultEffects++] = m_pBloom;
	}
	if (uInitFlags & PostFXSys::EFFECT_SKY_FOG)
	{
		m_pSkyFog = vnew(ALLOC_OBJECT) SkyFog();
		m_pDefaultEffects[m_uDefaultEffects++] = m_pSkyFog;
	}
	if(uInitFlags & PostFXSys::EFFECT_DEFERRED_SHADING )
	{
		m_pDeferredShading = vnew(ALLOC_OBJECT) DeferredShading();
		m_pDefaultEffects[m_uDefaultEffects++] = m_pDeferredShading;

		m_pSetLinDepthTarget = vnew(ALLOC_OBJECT) SetSceneLinDepthTarget();
		m_pDefaultEffects[m_uDefaultEffects++] = m_pSetLinDepthTarget;
	}

	m_pSetNoDepthTarget = vnew(ALLOC_OBJECT) SetSceneTarget();
	m_pDefaultEffects[m_uDefaultEffects++] = m_pSetNoDepthTarget;

	if(uInitFlags & PostFXSys::EFFECT_FILM_GRAIN)
	{
		m_pFilmGrain = vnew(ALLOC_OBJECT) FilmGrain();
		m_pDefaultEffects[m_uDefaultEffects++] = m_pFilmGrain;
	}
	if(uInitFlags & PostFXSys::EFFECT_SSAO)
	{
		m_pSSAO = vnew(ALLOC_OBJECT) ASSAO();
		m_pDefaultEffects[m_uDefaultEffects++] = m_pSSAO;
	}

	for(uint32 i = 0; i < m_uDefaultEffects; i++)
	{
		m_pDefaultEffects[i]->Init(pDevice, pResMgr, pParent);
	}

	EnableEffects(pDevice, uInitFlags);

	
	
	for(uint32 i=0; i<m_uDefaultEffects; i++)
	{
		m_pParent->RegisterEffect(m_pDefaultEffects[i]);
	}	
}

void PostFXSys_ps::Cleanup(GFXDevice* pDevice)
{
	for (auto & colorBuffer : m_colorBuffer)
	{
		if (colorBuffer.IsValid())
		{
			colorBuffer.Cleanup(pDevice);
		}
	}

	m_depthStencil.Cleanup(pDevice);

	for (auto* const pPostEffect : m_pDefaultEffects)
	{
		if (pPostEffect != nullptr)
		{
			pPostEffect->Cleanup(pDevice);
		}
	}

	ClearDynamicTargets(pDevice);
}


void PostFXSys_ps::ClearDynamicTargets(GFXDevice* pDevice)
{
	for(auto itr : m_dynamicTargets)
	{
		itr->Cleanup(pDevice);
		vdelete itr;
	}

	m_dynamicTargets.clear();
}


bool PostFXSys_ps::CanReuseTarget(memsize pass)
{
	if(pass == 0)
		return false;	// TODO: Handle re-using initial target

	for (int iTarget = 0; iTarget < (int)PostEffect::Input::Count; iTarget++)
	{
		PostEffect::Input eTarget = PostEffect::Input(iTarget);
		if( m_activeEffects[pass - 1]->WritesTexture(eTarget) != m_activeEffects[pass]->WritesTexture(eTarget) )
		{
			return false;
		}

		if (m_activeEffects[pass]->WritesTexture(eTarget) && m_activeEffects[pass]->ReadsTexture(eTarget))
		{
			return false;
		}

		if(NeedsStoring(pass, eTarget) != NeedsStoring(pass -1, eTarget))
		{
			return false;
		}

		if (NeedsShaderRead((int)pass, eTarget) != NeedsShaderRead((int)(pass) - 1, eTarget))
		{
			return false;
		}
	}

	return true;
}

bool PostFXSys_ps::IsLastTarget(memsize pass)
{
	memsize count = m_activeEffects.size();
	for(memsize i = pass+1; i < count; i++)
	{
		if( !CanReuseTarget(i))
		{
			return false;
		}
	}
	return true;
}

TextureHndl PostFXSys_ps::GetTexture(PostEffect::Input eInput, bool bHDR, memsize hdrIdx, memsize ldrIdx)
{
	switch(eInput)
	{
		case PostEffect::Input::Color:
			if(bHDR)
			{
				return m_colorBuffer[BUFFER_HDR_0 + hdrIdx].GetTexture();
			}
			else
			{
				return m_colorBuffer[BUFFER_LDR_0 + ldrIdx].GetTexture();
			}
		case PostEffect::Input::LinearDepth:
			return m_colorBuffer[BUFFER_LIN_DEPTH].GetTexture();
		case PostEffect::Input::Depth:
			return m_depthStencil.GetTexture();
		case PostEffect::Input::Albedo:
			return m_colorBuffer[BUFFER_DIFFUSE].GetTexture();
		case PostEffect::Input::Normal:
			return m_colorBuffer[BUFFER_NORMAL].GetTexture();
		case PostEffect::Input::Emissive:
			return m_colorBuffer[BUFFER_EMISSIVE].GetTexture();
		case PostEffect::Input::Specular:
			return m_colorBuffer[BUFFER_SPECULAR].GetTexture();
		default:
			ASSERT(false);
			return nullptr;
	}
}

void PostFXSys_ps::AddCustomEffect(PostEffect* pEffect)
{
	auto itr = eastl::find(m_customEffects.begin(), m_customEffects.end(), pEffect);
	if(itr == m_customEffects.end())
	{
		m_customEffects.push_back(pEffect);
	}
}

void PostFXSys_ps::RemoveCustomEffect(PostEffect* pEffect)
{
	m_customEffects.remove(pEffect);
}

void PostFXSys_ps::EnableEffectsInt(GFXDevice* pDevice, uint32 uEffectFlags)
{
	usg::RenderTarget::RenderPassFlags flags;

	usg::vector<ColorBuffer*> pBuffers;

	ClearDynamicTargets(pDevice);

	m_activeEffects.clear();
	for (uint32 i = 0; i < m_uDefaultEffects; i++)
	{
		if (m_pDefaultEffects[i]->GetEnabled())
		{
			m_activeEffects.push_back(m_pDefaultEffects[i]);
		}
	}
	for(auto itr : m_customEffects)
	{
		if(itr->GetEnabled())
		{
			m_activeEffects.push_back(itr);
		}
	}

	qsort(&m_activeEffects[0], m_activeEffects.size(), sizeof(RenderNode*), CompareNodes);


	// Max buffers
	int iFinalHDRTarget = -1;	// TODO: Set to last if final target is HDR
	int iGBufferPass = -1;
	memsize hdrIdx = 0;
	memsize ldrIdx = 0;
	flags.uLoadFlags = 0;// RenderTarget::RT_FLAG_COLOR_1 | RenderTarget::RT_FLAG_DS;
	flags.uStoreFlags = 0;
	flags.uClearFlags = 0;

	if(uEffectFlags & PostFXSys::EFFECT_FINAL_TARGET_HDR)
	{
		iFinalHDRTarget = (int)m_activeEffects.size();
	}

	for (memsize i = 0; i < m_activeEffects.size(); i++)
	{
		if( m_activeEffects[i]->RequiresHDR() )
		{
			iFinalHDRTarget = Math::Max(iFinalHDRTarget, (int)i);
		}
	}


	if ((uEffectFlags & PostFXSys::EFFECT_DEFERRED_SHADING) == 0)
	{
		// Start off without GBuffer
		if(iFinalHDRTarget < 0)
		{
			pBuffers.push_back( &m_colorBuffer[BUFFER_LDR_0] );
		}
		else
		{
			pBuffers.push_back(&m_colorBuffer[BUFFER_HDR_0]);
		}
		pBuffers.push_back(&m_colorBuffer[BUFFER_LIN_DEPTH]);

		flags.uStoreFlags = RenderTarget::RT_FLAG_COLOR_0 | RenderTarget::RT_FLAG_COLOR_1 | RenderTarget::RT_FLAG_DS;
		flags.uShaderReadFlags = 0;
		if( NeedsShaderRead(-1, PostEffect::Input::Color)  )
			flags.uShaderReadFlags |= RenderTarget::RT_FLAG_COLOR_0;
		if (NeedsShaderRead(-1, PostEffect::Input::LinearDepth))
			flags.uShaderReadFlags |= RenderTarget::RT_FLAG_COLOR_1;

		if (uEffectFlags & PostFXSys::EFFECT_SSAO)
		{
			m_pSSAO->SetDepthSource();
		}
	}
	else
	{
		pBuffers.push_back(&m_colorBuffer[BUFFER_DIFFUSE]);
		pBuffers.push_back(&m_colorBuffer[BUFFER_LIN_DEPTH]);
		pBuffers.push_back(&m_colorBuffer[BUFFER_NORMAL]);
		pBuffers.push_back(&m_colorBuffer[BUFFER_EMISSIVE]);
		pBuffers.push_back(&m_colorBuffer[BUFFER_SPECULAR]);

		flags.uStoreFlags = RenderTarget::RT_FLAG_COLOR_0 | RenderTarget::RT_FLAG_COLOR_1 | RenderTarget::RT_FLAG_COLOR_2 | RenderTarget::RT_FLAG_COLOR_3 | RenderTarget::RT_FLAG_COLOR_4 | RenderTarget::RT_FLAG_DS;

		if (uEffectFlags & PostFXSys::EFFECT_SSAO)
		{
			m_pSSAO->SetLinearDepthSource();
		}

		for (int iTarget = (int)PostEffect::Input::LinearDepth; iTarget <= (int)PostEffect::Input::Specular; iTarget++)
		{
			PostEffect::Input eTarget = PostEffect::Input(iTarget);
			if (NeedsShaderRead(-1, eTarget))
				flags.uShaderReadFlags |= GetFlagForTarget(eTarget);
		}
	}

	// Always clear the depth and linear depth on the first pass
	flags.uClearFlags = RenderTarget::RT_FLAG_COLOR_1 | RenderTarget::RT_FLAG_DS;

	usg::RenderTarget* pTarget = vnew(ALLOC_GFX_RENDER_TARGET)RenderTarget;
	pTarget->InitMRT(pDevice, (uint32)pBuffers.size(), pBuffers.data(), &m_depthStencil, "PostFX Chain");

	// Linear depth needs to clear to 1
	pTarget->SetClearColor(Color(1.0f, 0.f, 0.0f, 0.0f), 1);

	pTarget->InitRenderPass(pDevice, flags);

	m_renderPasses.SetRenderPass(RenderLayer::LAYER_BACKGROUND, 0, pTarget->GetRenderPass());

	m_dynamicTargets.push_back(pTarget);


	for (memsize i = 0; i < m_activeEffects.size(); i++)
	{
		flags.uLoadFlags = 0;
		flags.uStoreFlags = 0;
		flags.uShaderReadFlags = 0;
		flags.uTransferSrcFlags = 0;
		flags.uClearFlags = 0;			// We currently don't let an effect request a clear

		for (int iTarget =0; iTarget < (int)PostEffect::Input::Count; iTarget++)
		{
			PostEffect::Input eTarget = PostEffect::Input(iTarget);
			if(m_activeEffects[i]->LoadsTexture(eTarget))
			{
				flags.uLoadFlags |= GetFlagForTarget(eTarget);
			}

			if (NeedsStoring(i, eTarget))
			{
				flags.uStoreFlags |= GetFlagForTarget(eTarget);
			}

			if(NeedsShaderRead((int)i, eTarget))
			{
				flags.uShaderReadFlags |= GetFlagForTarget(eTarget);
			}

			if(m_activeEffects[i]->ReadsTexture(eTarget))
			{
				m_activeEffects[i]->SetTexture(pDevice, eTarget, GetTexture(eTarget, (int)i<=iFinalHDRTarget, hdrIdx, ldrIdx));
			}
		
		}

		GetRenderTargetBuffers(i, pBuffers, iFinalHDRTarget, hdrIdx, ldrIdx);


		// The last render target needs to be used as a transfer source
		if(IsLastTarget(i))
		{
			flags.uTransferSrcFlags = RenderTarget::RT_FLAG_COLOR_0;
			flags.uShaderReadFlags |= RenderTarget::RT_FLAG_COLOR_0;
			flags.uStoreFlags |= RenderTarget::RT_FLAG_COLOR_0;
		}


		usg::RenderTarget* pTarget = nullptr;
		usg::RenderTarget* pSrc = m_dynamicTargets.back();
		if(!CanReuseTarget(i))
		{
			pTarget = vnew(ALLOC_GFX_RENDER_TARGET)RenderTarget;
			DepthStencilBuffer* pDS = m_activeEffects[i]->WritesTexture(PostEffect::Input::Depth) ? &m_depthStencil : nullptr;
			pTarget->InitMRT(pDevice, (uint32)pBuffers.size(), pBuffers.data(), pDS, "Post FX Chain");
			pTarget->InitRenderPass(pDevice, flags);
			m_dynamicTargets.push_back(pTarget);
		}
		else
		{
			pTarget = m_dynamicTargets.back();
		}

		//m_activeEffects[i]->SetSourceTarget(pDevice, pSrc);
		m_activeEffects[i]->SetDestTarget(pDevice, pTarget);

		m_activeEffects[i]->PassDataSet(pDevice);

		m_renderPasses.SetRenderPass(m_activeEffects[i]->GetLayer(), m_activeEffects[i]->GetPriority(), pTarget->GetRenderPass());


	}
	m_renderPasses.UpdateEnd(pDevice);

}

void PostFXSys_ps::ForceUpdateRenderPasses(GFXDevice* pDevice)
{
	m_renderPasses.UpdateEnd(pDevice);
}

void PostFXSys_ps::GetRenderTargetBuffers(memsize pass, usg::vector<ColorBuffer*>& pBuffers, int iFinalHdr, memsize& hdrIdx, memsize& ldrIdx)
{
	// TODO: We could probably refactor to remove these dependencies so long as we keep the ordering ok
	pBuffers.clear();

	if(m_activeEffects[pass]->WritesTexture(PostEffect::Input::Albedo))
	{
		pBuffers.push_back(&m_colorBuffer[BUFFER_DIFFUSE]);
		ASSERT(!m_activeEffects[pass]->WritesTexture(PostEffect::Input::Color));
	}
	else if(m_activeEffects[pass]->WritesTexture(PostEffect::Input::Color))
	{	
		// FIXME: OR HDR enabled at a system level
		if((int)pass < iFinalHdr)
		{	
			// If it reads that texture as a source we need to swap the buffers
			if (m_activeEffects[pass]->ReadsTexture(PostEffect::Input::Color))
				hdrIdx = (hdrIdx + 1) % 2;

			pBuffers.push_back(&m_colorBuffer[BUFFER_HDR_0 + hdrIdx]);
		}
		else
		{
			// If it reads that texture as a source we need to swap the buffers
			if (m_activeEffects[pass]->ReadsTexture(PostEffect::Input::Color))
				ldrIdx = (ldrIdx + 1) % 2;

			pBuffers.push_back(&m_colorBuffer[BUFFER_LDR_0 + ldrIdx]);
		}
		ASSERT(!m_activeEffects[pass]->WritesTexture(PostEffect::Input::Albedo));
	}

	if (m_activeEffects[pass]->WritesTexture(PostEffect::Input::LinearDepth))
	{
		pBuffers.push_back(&m_colorBuffer[BUFFER_LIN_DEPTH]);
	}

	if (m_activeEffects[pass]->WritesTexture(PostEffect::Input::Normal))
	{
		pBuffers.push_back(&m_colorBuffer[BUFFER_NORMAL]);
	}
	
	if (m_activeEffects[pass]->WritesTexture(PostEffect::Input::Emissive))
	{
		pBuffers.push_back(&m_colorBuffer[BUFFER_EMISSIVE]);
	}

	if (m_activeEffects[pass]->WritesTexture(PostEffect::Input::Specular))
	{
		pBuffers.push_back(&m_colorBuffer[BUFFER_SPECULAR]);
	}
}

int PostFXSys_ps::GetFlagForTarget(PostEffect::Input eTarget)
{
	switch(eTarget)
	{
		case PostEffect::Input::Color:
			return RenderTarget::RTFlags::RT_FLAG_COLOR_0;
		case PostEffect::Input::LinearDepth:
			return RenderTarget::RTFlags::RT_FLAG_COLOR_1;
		case PostEffect::Input::Depth:
			return RenderTarget::RTFlags::RT_FLAG_DS;
		case PostEffect::Input::Albedo:
			return RenderTarget::RTFlags::RT_FLAG_COLOR_0;
		case PostEffect::Input::Normal:
			return RenderTarget::RTFlags::RT_FLAG_COLOR_2;
		case PostEffect::Input::Emissive:
			return RenderTarget::RTFlags::RT_FLAG_COLOR_3;
		case PostEffect::Input::Specular:
			return RenderTarget::RTFlags::RT_FLAG_COLOR_4;
		default:
			ASSERT(false);
	}
	return 0;
}

bool PostFXSys_ps::NeedsShaderRead(sint32 pass, PostEffect::Input eInput)
{
	for (int i = pass + 1; i < (int)m_activeEffects.size(); i++)
	{
		// TODO: Optimise me, it's possible that a multi-pass effect would both read and write, so for now just set to true
		if (m_activeEffects[i]->ReadsTexture(eInput))
		{
			return true;
		}
		else if (m_activeEffects[i]->WritesTexture(eInput))
		{
			return false;
		}
	}
	return false;
}

bool PostFXSys_ps::NeedsStoring(memsize pass, PostEffect::Input eInput)
{
	if(eInput == PostEffect::Input::Color 
		|| ( (eInput == PostEffect::Input::Depth) && m_activeEffects[pass]->WritesTexture(eInput) ) )
	{
		return true;	// Always keep color
	}
	for (memsize i = pass+1; i < m_activeEffects.size(); i++)
	{
		if( m_activeEffects[i]->ReadsTexture(eInput) )
		{
			return true;
		}
	}
	return false;
}


void PostFXSys_ps::EnableEffects(GFXDevice* pDevice, uint32 uEffectFlags)
{
	m_renderPasses.ClearAllPasses();
	
	if(m_pDeferredShading)
		m_pDeferredShading->SetEnabled( (uEffectFlags & PostFXSys::EFFECT_DEFERRED_SHADING) != 0);
	if(m_pSetNoDepthTarget)
		m_pSetNoDepthTarget->SetEnabled(true);//(uEffectFlags & PostFXSys::EFFECT_DEFERRED_SHADING) == 0);
	if (m_pSetLinDepthTarget)
		m_pSetLinDepthTarget->SetEnabled((uEffectFlags & PostFXSys::EFFECT_DEFERRED_SHADING) != 0);
	if(m_pSkyFog)
		m_pSkyFog->SetEnabled((uEffectFlags & PostFXSys::EFFECT_SKY_FOG) != 0);
	if(m_pBloom)
		m_pBloom->SetEnabled((uEffectFlags & PostFXSys::EFFECT_BLOOM) != 0);
	if(m_pFXAA)
		m_pFXAA->SetEnabled((uEffectFlags & PostFXSys::EFFECT_FXAA) != 0);
	if(m_pSMAA)
		m_pSMAA->SetEnabled((uEffectFlags & PostFXSys::EFFECT_SMAA) != 0);
	if (m_pFilmGrain)
		m_pFilmGrain->SetEnabled((uEffectFlags & PostFXSys::EFFECT_FILM_GRAIN) != 0);
	if (m_pSSAO)
	{
		m_pSSAO->SetEnabled((uEffectFlags & PostFXSys::EFFECT_SSAO) != 0);
		m_pSSAO->SetDeferred((uEffectFlags & PostFXSys::EFFECT_DEFERRED_SHADING) != 0 );
	}

	m_renderPasses.SetDeferredEnabled(m_pDeferredShading && (uEffectFlags & PostFXSys::EFFECT_DEFERRED_SHADING) != 0);

	EnableEffectsInt(pDevice, uEffectFlags);
	

}


PostEffect* PostFXSys_ps::GetFinalEffect()
{
	uint64 uMaxCmpValue = 0;
	RenderLayer eMaxLayer = RenderLayer::LAYER_BACKGROUND;
	PostEffect* pEffectOut = nullptr;
	for (uint32 i = 0; i < m_uDefaultEffects; i++)
	{
		if( m_pDefaultEffects[i]->GetLayer() > eMaxLayer ||
			( m_pDefaultEffects[i]->GetLayer() == eMaxLayer &&
			m_pDefaultEffects[i]->GetComparisonValue() > uMaxCmpValue ) )
		{
			uMaxCmpValue = m_pDefaultEffects[i]->GetComparisonValue();
			eMaxLayer = m_pDefaultEffects[i]->GetLayer();
			pEffectOut = m_pDefaultEffects[i];
		}
	}
	return pEffectOut;
}


void PostFXSys_ps::ResizeTargetsInt(GFXDevice* pDevice, uint32 uWidth, uint32 uHeight)
{
	uint32 uScaledWidth = (uint32)((m_fPixelScale * uWidth)+0.5f);
	uint32 uScaledHeight = (uint32)((m_fPixelScale * uHeight) + 0.5f);

	for (auto & colorBuffer : m_colorBuffer)
	{
		if (colorBuffer.IsValid())
		{
			colorBuffer.Resize(pDevice, uScaledWidth, uScaledHeight);
		}
	}

	m_depthStencil.Resize(pDevice, uScaledWidth, uScaledHeight);

	for (auto & screenRT : m_dynamicTargets)
	{
		if (screenRT->IsValid())
		{
			screenRT->Resize(pDevice);
		}
	}

	for (auto* const pPostEffect : m_pDefaultEffects)
	{
		if (pPostEffect != nullptr)
		{
			pPostEffect->Resize(pDevice, uScaledWidth, uScaledHeight);
		}
	}

	/*
	if (m_pFinalEffect)
	{
		m_pFinalEffect->SetDestTarget(pDevice)
	}*/
}

void PostFXSys_ps::Resize(GFXDevice* pDevice, uint32 uWidth, uint32 uHeight)
{
	ResizeTargetsInt(pDevice, uWidth, uHeight);
}

const TextureHndl& PostFXSys_ps::GetLinearDepthTex() const
{
	return m_colorBuffer[BUFFER_LIN_DEPTH].GetTexture();
}


void PostFXSys_ps::UpdateRTSize(GFXDevice* pDevice, Display* pDisplay)
{
	// We haven't optimised resizing on the PC yet
#ifdef PLATFORM_SWITCH
	float fFrameTime = pDevice->GetPlatform().GetGPUTime();
	uint32 uWidth, uHeight;
	pDisplay->GetActualDimensions(uWidth, uHeight, false);
	if (m_fPixelScale < 1.0f)
	{
		if (fFrameTime < 12.0f)
		{
			m_fPixelScale = 1.0f;
			ResizeTargetsInt(pDevice, uWidth, uHeight);
		}
	}
	else
	{
		if (fFrameTime > 15.5f)
		{
			m_fPixelScale = 1.f / 1.2f;
			ResizeTargetsInt(pDevice, uWidth, uHeight);
		}
	}
#endif
}


void PostFXSys_ps::SetSkyTexture(GFXDevice* pDevice, const TextureHndl& tex)
{
	if (m_pSkyFog)
	{
		m_pSkyFog->SetTexture(pDevice, tex, m_colorBuffer[BUFFER_LIN_DEPTH].GetTexture());
	}
}



PipelineStateHndl PostFXSys_ps::GetDownscale4x4Pipeline(GFXDevice* pDevice, ResourceMgr* pResMgr, const RenderPassHndl& renderPass) const
{

	PipelineStateDecl pipelineDecl;
	pipelineDecl.inputBindings[0].Init(GetVertexDeclaration(VT_POSITION));
	pipelineDecl.uInputBindingCount = 1;
	pipelineDecl.alphaState.SetColor0Only();

	pipelineDecl.rasterizerState.eCullFace = CULL_FACE_NONE;

	DescriptorSetLayoutHndl multiDesc = pDevice->GetDescriptorSetLayout(g_multiSampleDescriptor);
	pipelineDecl.layout.descriptorSets[0] = pDevice->GetDescriptorSetLayout(SceneConsts::g_globalDescriptorDecl);
	pipelineDecl.layout.descriptorSets[1] = multiDesc;
	pipelineDecl.layout.uDescriptorSetCount = 2;

	pipelineDecl.pEffect = pResMgr->GetEffect(pDevice, "PostProcess.Downscale2x2");
	return pDevice->GetPipelineState(renderPass, pipelineDecl);
}

PipelineStateHndl PostFXSys_ps::GetGaussBlurPipeline(GFXDevice* pDevice, ResourceMgr* pResMgr, const RenderPassHndl& renderPass) const
{
	PipelineStateDecl pipelineDecl;


	pipelineDecl.inputBindings[0].Init(GetVertexDeclaration(VT_POSITION));
	pipelineDecl.uInputBindingCount = 1;
	pipelineDecl.alphaState.SetColor0Only();

	pipelineDecl.rasterizerState.eCullFace = CULL_FACE_NONE;

	DescriptorSetLayoutHndl multiDesc = pDevice->GetDescriptorSetLayout(g_multiSampleDescriptor);
	pipelineDecl.layout.descriptorSets[0] = pDevice->GetDescriptorSetLayout(SceneConsts::g_globalDescriptorDecl);
	pipelineDecl.layout.descriptorSets[1] = multiDesc;
	pipelineDecl.layout.uDescriptorSetCount = 2;

	// FIXME: Cache rather than grabbing the resource mgr
	pipelineDecl.pEffect = pResMgr->GetEffect(pDevice, "PostProcess.Gauss5x5");
	return pDevice->GetPipelineState(renderPass, pipelineDecl);

}

void PostFXSys_ps::SetupDownscale4x4(GFXDevice* pDevice, ConstantSet& cb, DescriptorSet& des, uint32 uWidth, uint32 uHeight) const
{
	SetupOffsets4x4(pDevice, cb, uWidth, uHeight);

	DescriptorSetLayoutHndl multiDesc = pDevice->GetDescriptorSetLayout(g_multiSampleDescriptor);

	des.Init(pDevice, multiDesc);
	des.SetConstantSet(1, &cb);

}

void PostFXSys_ps::SetupGaussBlur(GFXDevice* pDevice, ConstantSet& cb, DescriptorSet& des, uint32 uWidth, uint32 uHeight, float fMultiplier) const
{
	SetupGaussBlurBuffer(pDevice, cb, uWidth, uHeight, fMultiplier);
	DescriptorSetLayoutHndl multiDesc = pDevice->GetDescriptorSetLayout(g_multiSampleDescriptor);

	des.Init(pDevice, multiDesc);
	des.SetConstantSet(1, &cb);
}

float PostFXSys_ps::GaussianDistribution( float x, float y, float rho ) const
{
	float g = 1.0f / sqrtf( 2.0f * Math::pi * rho * rho );
    g *= expf( -( x * x + y * y ) / ( 2 * rho * rho ) );

    return g;
}


RenderTarget* PostFXSys_ps::GetInitialRT()
{
	return m_dynamicTargets.front();
}

RenderTarget* PostFXSys_ps::GetFinalRT()
{
	return m_dynamicTargets.back();
}


void PostFXSys_ps::SetupOffsets4x4(GFXDevice* pDevice, ConstantSet& cb, uint32 uWidth, uint32 uHeight ) const
{
	if(!cb.IsValid())
	{
		cb.Init(pDevice, g_downscale44ConstantDef);
	}

	// Divide by two as we are handling half the samples by using the linear filter
	uWidth	/=2;
	uHeight /=2;

    float tU = 1.0f / (((float)uWidth)/2.f);
    float tV = 1.0f / (((float)uHeight)/2.f);

	Downscale44Constants* pConstants = cb.Lock<Downscale44Constants>();
	Vector4f* pOffsets = pConstants->vOffsets;

    // Sample from the 16 surrounding points.
    int index = 0;
    for( int y = 0; y < 2; y++ )
    {
        for( int x = 0; x < 2; x++ )
        {
            pOffsets[ index ].x	= ( x - 0.5f ) * tU;
            pOffsets[ index ].y = ( y - 0.5f ) * tV;

            index++;
        }
    }

	cb.Unlock();
	cb.UpdateData(pDevice);
}


void PostFXSys_ps::SetupGaussBlurBuffer(GFXDevice* pDevice, ConstantSet& cb, uint32 uWidth, uint32 uHeight, float fMultiplier) const
{
	if(!cb.IsValid())
	{
		cb.Init(pDevice, g_gaussConstantDef);
	}

    float tu = 1.0f / ( float )uWidth;
    float tv = 1.0f / ( float )uHeight;

	GaussConstants* pConstants = cb.Lock<GaussConstants>();
	Vector4f* pOffsets = pConstants->vOffsets;

    float totalWeight = 0.0f;
    int index = 0;
    for( int x = -2; x <= 2; x++ )
    {
        for( int y = -2; y <= 2; y++ )
        {
            if( Math::Abs( x ) + Math::Abs( y ) > 2 )
			{
                continue;
			}

            // Get the intesnisty (unscaled)
            pOffsets[index].x	= x * tu;
			pOffsets[index].y	= y * tv;

            pOffsets[index].z = GaussianDistribution( ( float )x, ( float )y, 1.0f );
            totalWeight +=  pOffsets[index].z;

            index++;
        }
    }

    // Divide the weight by the total weight of all the samples. 
    // Add 1.0f to ensure that the intensity of the image isn't
    // changed when the blur occurs.
    for( int i = 0; i < index; i++ )
    {
        pOffsets[i].z /= totalWeight;
        pOffsets[i].z *= fMultiplier;
    }

	cb.Unlock();
	cb.UpdateData(pDevice);
}


const SceneRenderPasses& PostFXSys_ps::GetRenderPasses() const
{
	return m_renderPasses;
}

SceneRenderPasses& PostFXSys_ps::GetRenderPasses()
{
	return m_renderPasses;
}


}

