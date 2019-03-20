/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Maths/Vector2f.h"
#include "Engine/PostFX/PostFXSys.h"
#include "Engine/Graphics/GFX.h"
#include "Engine/Graphics/Device/GFXDevice.h"
#include "Engine/Resource/ResourceMgr.h"
#include "Engine/Scene/SceneConstantSets.h"
#include "Engine/Graphics/StandardVertDecl.h"
#include "Engine/Core/stl/vector.h"
#include "Engine/Graphics/Device/GFXContext.h"
#include "SMAA.h"

#define SMAA_PREDICATION 0
#define SMAA_REPROJECTION 0

namespace usg {

	struct SMAAConstants
	{
		Vector4f	vScreenMetrics;	 // (inv width, inv height, width, height)
		Vector4i	vSubsampleIndices; // Just pass zero for SMAA 1x
	};


	static const ShaderConstantDecl g_smaaConstantDef[] = 
	{
		SHADER_CONSTANT_ELEMENT(SMAAConstants, vScreenMetrics,		CT_VECTOR_4, 1 ),
		SHADER_CONSTANT_ELEMENT(SMAAConstants, vSubsampleIndices,	CT_VECTOR4I, 1 ),
		SHADER_CONSTANT_END()
	};

	static const DescriptorDeclaration g_descriptorColorLumaEdgeDecl[] =
	{
		DESCRIPTOR_ELEMENT(SHADER_CONSTANT_MATERIAL, DESCRIPTOR_TYPE_CONSTANT_BUFFER, 1, SHADER_FLAG_VS_PS),
		DESCRIPTOR_ELEMENT(0,						 DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, SHADER_FLAG_PIXEL),
#if SMAA_PREDICATION
		DESCRIPTOR_ELEMENT(1,						 DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, SHADER_FLAG_PIXEL),
#endif
		DESCRIPTOR_END()
	};

	static const DescriptorDeclaration g_descriptorDepthEdgeDecl[] =
	{
		DESCRIPTOR_ELEMENT(SHADER_CONSTANT_MATERIAL, DESCRIPTOR_TYPE_CONSTANT_BUFFER, 1, SHADER_FLAG_VS_PS),
		DESCRIPTOR_ELEMENT(0,						 DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, SHADER_FLAG_PIXEL),
		DESCRIPTOR_END()
	};

	static const DescriptorDeclaration g_descriptorBlendWeightDecl[] =
	{
		DESCRIPTOR_ELEMENT(SHADER_CONSTANT_MATERIAL, DESCRIPTOR_TYPE_CONSTANT_BUFFER, 1, SHADER_FLAG_VS_PS),
		DESCRIPTOR_ELEMENT(0,						 DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, SHADER_FLAG_PIXEL),	// Edges
		DESCRIPTOR_ELEMENT(1,						 DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, SHADER_FLAG_PIXEL),	// Area
		DESCRIPTOR_ELEMENT(2,						 DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, SHADER_FLAG_PIXEL),	// Search tex

		DESCRIPTOR_END()
	};

	static const DescriptorDeclaration g_descriptorNeighbourhoodBlendDecl[] =
	{
		DESCRIPTOR_ELEMENT(SHADER_CONSTANT_MATERIAL, DESCRIPTOR_TYPE_CONSTANT_BUFFER, 1, SHADER_FLAG_VS_PS),
		DESCRIPTOR_ELEMENT(0,						 DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, SHADER_FLAG_PIXEL),	// Color
		DESCRIPTOR_ELEMENT(1,						 DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, SHADER_FLAG_PIXEL),	// Blend
#if SMAA_PREDICATION
		DESCRIPTOR_ELEMENT(2,						 DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, SHADER_FLAG_PIXEL),	// Velocity
#endif
		DESCRIPTOR_END()
	};

	static const DescriptorDeclaration g_descriptorResolveDecl[] =
	{
		DESCRIPTOR_ELEMENT(SHADER_CONSTANT_MATERIAL, DESCRIPTOR_TYPE_CONSTANT_BUFFER, 1, SHADER_FLAG_VS_PS),
		DESCRIPTOR_ELEMENT(0,						 DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, SHADER_FLAG_PIXEL),	// Current color
		DESCRIPTOR_ELEMENT(1,						 DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, SHADER_FLAG_PIXEL),	// Prev color
#if SMAA_REPROJECTION
		DESCRIPTOR_ELEMENT(2,						 DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, SHADER_FLAG_PIXEL),	// Velocity tex
#endif
		DESCRIPTOR_END()
	};


	SMAA::SMAA():
	PostEffect()
	{
		SetRenderMask(RENDER_MASK_POST_EFFECT);
		SetLayer(LAYER_POST_PROCESS);
		SetPriority(29);
	}


	SMAA::~SMAA()
	{

	}

	void SMAA::Init(GFXDevice* pDevice, PostFXSys* pSys, RenderTarget* pDst)
	{
		m_pSys = pSys;
		m_pDestTarget = pDst;

		SamplerDecl pointDecl(SF_POINT, SC_CLAMP);
		SamplerDecl linearDecl(SF_LINEAR, SC_CLAMP);

		m_pointSampler = pDevice->GetSampler(pointDecl);
		m_linearSampler = pDevice->GetSampler(linearDecl);

		PipelineStateDecl pipelineDecl;
		pipelineDecl.inputBindings[0].Init(usg::GetVertexDeclaration(usg::VT_POSITION));
		pipelineDecl.uInputBindingCount = 1;
		pipelineDecl.ePrimType = PT_TRIANGLES;
		pipelineDecl.rasterizerState.eCullFace = CULL_FACE_NONE;
		pipelineDecl.alphaState.SetColor0Only();

		usg::DescriptorSetLayoutHndl colorLumaEdgeDescriptors = pDevice->GetDescriptorSetLayout(g_descriptorColorLumaEdgeDecl);
		usg::DescriptorSetLayoutHndl depthEdgeDescriptors = pDevice->GetDescriptorSetLayout(g_descriptorDepthEdgeDecl);
		usg::DescriptorSetLayoutHndl blendWeightDescriptors = pDevice->GetDescriptorSetLayout(g_descriptorBlendWeightDecl);
		usg::DescriptorSetLayoutHndl neighborHoodBlendDescriptors = pDevice->GetDescriptorSetLayout(g_descriptorNeighbourhoodBlendDecl);
		usg::DescriptorSetLayoutHndl resolveDescriptors = pDevice->GetDescriptorSetLayout(g_descriptorResolveDecl);

		uint32 uWidth = pSys->GetFinalTargetWidth();
		uint32 uHeight = pSys->GetFinalTargetHeight();
		m_colorBuffers[RT_EDGES].Init(pDevice, uWidth, uHeight, CF_RGBA_8888, SAMPLE_COUNT_1_BIT, TU_FLAGS_OFFSCREEN_COLOR);
		m_renderTargets[RT_EDGES].Init(pDevice, &m_colorBuffers[RT_EDGES]);
		usg::RenderTarget::RenderPassFlags flags;
		flags.uStoreFlags = RenderTarget::RT_FLAG_COLOR_0;
		flags.uShaderReadFlags = RenderTarget::RT_FLAG_COLOR_0;
		m_renderTargets[RT_EDGES].InitRenderPass(pDevice, flags);
		m_colorBuffers[RT_BLEND_WEIGHT].Init(pDevice, uWidth, uHeight, CF_RGBA_8888, SAMPLE_COUNT_1_BIT, TU_FLAGS_OFFSCREEN_COLOR);
		m_renderTargets[RT_BLEND_WEIGHT].Init(pDevice, &m_colorBuffers[RT_BLEND_WEIGHT]);
		m_renderTargets[RT_BLEND_WEIGHT].InitRenderPass(pDevice, flags);



		// Depth edge detection
		pipelineDecl.layout.descriptorSets[0] = pDevice->GetDescriptorSetLayout(SceneConsts::g_globalDescriptorDecl);
		pipelineDecl.layout.descriptorSets[1] = depthEdgeDescriptors;
		pipelineDecl.layout.uDescriptorSetCount = 2;
		pipelineDecl.pEffect = ResourceMgr::Inst()->GetEffect(pDevice, "PostProcess.SMAADepthEdgeDetection");
		m_depthEdgeDetectEffect = pDevice->GetPipelineState(m_renderTargets[RT_EDGES].GetRenderPass(), pipelineDecl);

		// Luma Edge detection
		pipelineDecl.layout.descriptorSets[1] = colorLumaEdgeDescriptors;
		pipelineDecl.pEffect = ResourceMgr::Inst()->GetEffect(pDevice, "PostProcess.SMAALumaEdgeDetection");
		m_lumaEdgeDetectEffect = pDevice->GetPipelineState(m_renderTargets[RT_EDGES].GetRenderPass(), pipelineDecl);

		// Color edge detection
		pipelineDecl.pEffect = ResourceMgr::Inst()->GetEffect(pDevice, "PostProcess.SMAAColorEdgeDetection");
		m_colorEdgeDetectEffect = pDevice->GetPipelineState(m_renderTargets[RT_EDGES].GetRenderPass(), pipelineDecl);


		// Blend weight calculation
		pipelineDecl.layout.descriptorSets[1] = blendWeightDescriptors;
		pipelineDecl.pEffect = ResourceMgr::Inst()->GetEffect(pDevice, "PostProcess.SMAABlendWeightCalc");
		m_blendWeightEffect = pDevice->GetPipelineState(m_renderTargets[RT_BLEND_WEIGHT].GetRenderPass(), pipelineDecl);

		// Neighbourhood blend
		pipelineDecl.layout.descriptorSets[1] = neighborHoodBlendDescriptors;
		pipelineDecl.pEffect = ResourceMgr::Inst()->GetEffect(pDevice, "PostProcess.SMAANeighborhoodBlend");
		m_neighbourBlendEffect = pDevice->GetPipelineState(pDst->GetRenderPass(), pipelineDecl);

#if SMAA_REPROJECTION
		// Resolve
		pipelineDecl.layout.descriptorSets[0] = resolveDescriptors;
		pipelineDecl.pEffect = ResourceMgr::Inst()->GetEffect(pDevice, "PostProcess.SMAAResolve");
		m_resolveEffect = pDevice->GetPipelineState(pipelineDecl);
#endif

		m_constantSet.Init(pDevice, g_smaaConstantDef);

		UpdateConstants(pDevice, pDst->GetWidth(), pDst->GetHeight());


		m_lumaColorEdgeDescriptorSet.Init(pDevice, colorLumaEdgeDescriptors);
		m_depthEdgeDescriptorSet.Init(pDevice, colorLumaEdgeDescriptors);
		m_blendWeightDescriptorSet.Init(pDevice, blendWeightDescriptors);
		m_neighbourBlendDescriptorSet.Init(pDevice, neighborHoodBlendDescriptors);

		m_lumaColorEdgeDescriptorSet.SetConstantSet(0, &m_constantSet);
		m_depthEdgeDescriptorSet.SetConstantSet(0, &m_constantSet);
		m_blendWeightDescriptorSet.SetConstantSet(0, &m_constantSet);
		m_neighbourBlendDescriptorSet.SetConstantSet(0, &m_constantSet);

#if SMAA_REPROJECTION
		m_resolveDescriptorSet.Init(pDevice, resolveDescriptors);
		m_resolveDescriptorSet.SetConstantSet(0, &m_constantSet);
#endif

		m_blendWeightDescriptorSet.SetImageSamplerPair(1, m_colorBuffers[RT_EDGES].GetTexture(), m_linearSampler);
		m_blendWeightDescriptorSet.SetImageSamplerPair(2, ResourceMgr::Inst()->GetTexture(pDevice, "SMAA_AreaTex"), m_linearSampler);
		m_blendWeightDescriptorSet.SetImageSamplerPair(3, ResourceMgr::Inst()->GetTexture(pDevice, "SMAA_SearchTex"), m_pointSampler);

		m_neighbourBlendDescriptorSet.SetImageSamplerPair(2, m_colorBuffers[RT_BLEND_WEIGHT].GetTexture(), m_linearSampler);
	}


	void SMAA::CleanUp(GFXDevice* pDevice)
	{
		m_lumaColorEdgeDescriptorSet.CleanUp(pDevice);
		m_depthEdgeDescriptorSet.CleanUp(pDevice);
		m_blendWeightDescriptorSet.CleanUp(pDevice);
		m_neighbourBlendDescriptorSet.CleanUp(pDevice);
		m_resolveDescriptorSet.CleanUp(pDevice);
		m_constantSet.CleanUp(pDevice);

		for (auto& it : m_colorBuffers)
		{
			it.CleanUp(pDevice);
		}

		for (auto& it : m_renderTargets)
		{
			it.CleanUp(pDevice);
		}
	}

	void SMAA::SetDestTarget(GFXDevice* pDevice, RenderTarget* pDst)
	{
		if (pDst != m_pDestTarget)
		{
			// Need to update the pipeline state if the destination changes
			PipelineStateDecl decl;
			RenderPassHndl rpOut;
			pDevice->GetPipelineDeclaration(m_neighbourBlendEffect, decl, rpOut);
			m_neighbourBlendEffect = pDevice->GetPipelineState(pDst->GetRenderPass(), decl);

			m_pDestTarget = pDst;
		}
	}

	void SMAA::UpdateConstants(GFXDevice* pDevice, uint32 uWidth, uint32 uHeight)
	{
		SMAAConstants* pConsts = m_constantSet.Lock<SMAAConstants>();

		pConsts->vScreenMetrics.x = 1.0f / uWidth;
		pConsts->vScreenMetrics.y = 1.0f / uHeight;
		pConsts->vScreenMetrics.z = (float)uWidth;
		pConsts->vScreenMetrics.w = (float)uHeight;

		pConsts->vSubsampleIndices.x = 0;
		pConsts->vSubsampleIndices.y = 0;
		pConsts->vSubsampleIndices.z = 0;
		pConsts->vSubsampleIndices.w = 0;

		m_constantSet.Unlock();
		m_constantSet.UpdateData(pDevice);
	}

	void SMAA::SetSourceTarget(GFXDevice* pDevice, RenderTarget* pTarget)
	{
		// TODO: Confirm sampling mode
		m_lumaColorEdgeDescriptorSet.SetImageSamplerPair(1, pTarget->GetColorTexture(0), m_linearSampler);
		m_depthEdgeDescriptorSet.SetImageSamplerPair(1, pTarget->GetDepthTexture(), m_linearSampler);	// FIXME: Use the linear depth
		m_neighbourBlendDescriptorSet.SetImageSamplerPair(1, pTarget->GetColorTexture(0), m_linearSampler);
#if SMAA_REPROJECTION
		m_resolveDescriptorSet.SetImageSamplerPair(1, pTarget->GetColorTexture(0), m_linearSampler);
#endif
		m_lumaColorEdgeDescriptorSet.UpdateDescriptors(pDevice);
		m_depthEdgeDescriptorSet.UpdateDescriptors(pDevice);

		UpdateConstants(pDevice, pTarget->GetWidth(), pTarget->GetHeight());
		UpdateDescriptors(pDevice);
	}

	void SMAA::UpdateDescriptors(GFXDevice* pDevice)
	{
		m_lumaColorEdgeDescriptorSet.UpdateDescriptors(pDevice);
		m_depthEdgeDescriptorSet.UpdateDescriptors(pDevice);
		m_blendWeightDescriptorSet.UpdateDescriptors(pDevice);
		m_neighbourBlendDescriptorSet.UpdateDescriptors(pDevice);
#if SMAA_REPROJECTION
		m_resolveDescriptorSet.UpdateDescriptors(pDevice);
#endif
	}

	void SMAA::Resize(GFXDevice* pDevice, uint32 uWidth, uint32 uHeight)
	{
		for (uint32 i = 0; i < RT_COUNT; i++)
		{
			if (m_colorBuffers[i].IsValid())
			{
				m_colorBuffers[i].Resize(pDevice, uWidth, uHeight);
				m_renderTargets[i].Resize(pDevice);
			}
		}

		m_blendWeightDescriptorSet.SetImageSamplerPair(1, m_colorBuffers[RT_EDGES].GetTexture(), m_linearSampler);
		m_neighbourBlendDescriptorSet.SetImageSamplerPair(2, m_colorBuffers[RT_BLEND_WEIGHT].GetTexture(), m_linearSampler);
		UpdateConstants(pDevice, uWidth, uHeight);
		UpdateDescriptors(pDevice);
	}

	bool SMAA::Draw(GFXContext* pContext, RenderContext& renderContext)
	{

		if (!GetEnabled())
			return false;

		pContext->BeginGPUTag("SMAA");

		pContext->SetRenderTarget(&m_renderTargets[RT_EDGES]);
		pContext->ClearRenderTarget();
		pContext->SetPipelineState(m_lumaEdgeDetectEffect);
		pContext->SetDescriptorSet(&m_lumaColorEdgeDescriptorSet, 1);
		m_pSys->DrawFullScreenQuad(pContext);

		pContext->SetRenderTarget(&m_renderTargets[RT_BLEND_WEIGHT]);
		pContext->SetPipelineState(m_blendWeightEffect);
		pContext->SetDescriptorSet(&m_blendWeightDescriptorSet, 1);
		m_pSys->DrawFullScreenQuad(pContext);

		pContext->SetRenderTarget(m_pDestTarget);
		pContext->SetPipelineState(m_neighbourBlendEffect);
		pContext->SetDescriptorSet(&m_neighbourBlendDescriptorSet, 1);
		m_pSys->DrawFullScreenQuad(pContext);

		pContext->EndGPUTag();
	 
		return true;
	}

}
