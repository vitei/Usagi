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
#include "ASSAO.h"


namespace usg
{

	struct ASSAOConstants
	{
	    Vector2f    ViewportPixelSize;                      // .zw == 1.0 / ViewportSize.xy
	    Vector2f    HalfViewportPixelSize;                  // .zw == 1.0 / ViewportHalfSize.xy

	    Vector2f	DepthUnpackConsts;
	    Vector2f	CameraTanHalfFOV;

	    Vector2f	NDCToViewMul;
	    Vector2f	NDCToViewAdd;

	    Vector2i	PerPassFullResCoordOffset;
	    Vector2f	PerPassFullResUVOffset;

	    Vector2f	Viewport2xPixelSize;
	    Vector2f	Viewport2xPixelSize_x_025;              // Viewport2xPixelSize * 0.25 (for fusing add+mul into mad)

	    float		EffectRadius;                           // world (viewspace) maximum size of the shadow
	    float		EffectShadowStrength;                   // global strength of the effect (0 - 5)
	    float		EffectShadowPow;
	    float		EffectShadowClamp;

	    float		EffectFadeOutMul;                       // effect fade out from distance (ex. 25)
	    float		EffectFadeOutAdd;                       // effect fade out to distance   (ex. 100)
	    float		EffectHorizonAngleThreshold;            // limit errors on slopes and caused by insufficient geometry tessellation (0.05 to 0.5)
	    float		EffectSamplRadiusNearLimitRec;          // if viewspace pixel closer than this, don't enlarge shadow sampling radius anymore (makes no sense to grow beyond some distance, not enough samples to cover everything, so just limit the shadow growth; could be SSAOSettingsFadeOutFrom * 0.1 or less)

	    float		DepthPrecisionOffsetMod;
	    float		NegRecEffectRadius;                     // -1.0 / EffectRadius
	    float		LoadCounterAvgDiv;                      // 1.0 / ( halfDepthMip[SSAO_DEPTH_MIP_LEVELS-1].sizeX * halfDepthMip[SSAO_DEPTH_MIP_LEVELS-1].sizeY )
	    float		AdaptiveSampleCountLimit;

	    float		InvSharpness;
	    int			PassIndex;
	    Vector2f	QuarterResPixelSize;                    // used for importance map only

	    Vector4f	PatternRotScaleMatrices[5];

	    float		NormalsUnpackMul;
	    float		NormalsUnpackAdd;
	    float		DetailAOStrength;
	    float		Dummy0;

#ifdef SSAO_ENABLE_NORMAL_WORLD_TO_VIEW_CONVERSION
    mat4x4                NormalsWorldToViewspaceMatrix;
#endif
	};

	static const ShaderConstantDecl g_assaoConstantDef[] = 
	{
		SHADER_CONSTANT_ELEMENT( ASSAOConstants, ViewportPixelSize,			CT_VECTOR_2, 1 ),
		SHADER_CONSTANT_ELEMENT( ASSAOConstants, HalfViewportPixelSize,		CT_VECTOR_2, 1 ),

		SHADER_CONSTANT_ELEMENT( ASSAOConstants, DepthUnpackConsts,			CT_VECTOR_2, 1 ),
		SHADER_CONSTANT_ELEMENT( ASSAOConstants, CameraTanHalfFOV,			CT_VECTOR_2, 1 ),

		SHADER_CONSTANT_ELEMENT( ASSAOConstants, NDCToViewMul,				CT_VECTOR_2, 1 ),
		SHADER_CONSTANT_ELEMENT( ASSAOConstants, NDCToViewAdd,				CT_VECTOR_2, 1 ),		

		SHADER_CONSTANT_ELEMENT( ASSAOConstants, PerPassFullResCoordOffset,	CT_VECTOR2I, 1 ),
		SHADER_CONSTANT_ELEMENT( ASSAOConstants, PerPassFullResUVOffset,	CT_VECTOR_2, 1 ),	

		SHADER_CONSTANT_ELEMENT( ASSAOConstants, Viewport2xPixelSize,		CT_VECTOR_2, 1 ),
		SHADER_CONSTANT_ELEMENT( ASSAOConstants, Viewport2xPixelSize_x_025,	CT_VECTOR_2, 1 ),

		SHADER_CONSTANT_ELEMENT( ASSAOConstants, EffectRadius,				CT_FLOAT, 1 ),
		SHADER_CONSTANT_ELEMENT( ASSAOConstants, EffectShadowStrength,		CT_FLOAT, 1 ),
		SHADER_CONSTANT_ELEMENT( ASSAOConstants, EffectShadowPow,			CT_FLOAT, 1 ),
		SHADER_CONSTANT_ELEMENT( ASSAOConstants, EffectShadowClamp,			CT_FLOAT, 1 ),

		SHADER_CONSTANT_ELEMENT( ASSAOConstants, EffectFadeOutMul,				CT_FLOAT, 1 ),
		SHADER_CONSTANT_ELEMENT( ASSAOConstants, EffectFadeOutAdd,				CT_FLOAT, 1 ),
		SHADER_CONSTANT_ELEMENT( ASSAOConstants, EffectHorizonAngleThreshold,	CT_FLOAT, 1 ),
		SHADER_CONSTANT_ELEMENT( ASSAOConstants, EffectSamplRadiusNearLimitRec,	CT_FLOAT, 1 ),

		SHADER_CONSTANT_ELEMENT( ASSAOConstants, DepthPrecisionOffsetMod,	CT_FLOAT, 1 ),
		SHADER_CONSTANT_ELEMENT( ASSAOConstants, NegRecEffectRadius,		CT_FLOAT, 1 ),
		SHADER_CONSTANT_ELEMENT( ASSAOConstants, LoadCounterAvgDiv,			CT_FLOAT, 1 ),
		SHADER_CONSTANT_ELEMENT( ASSAOConstants, AdaptiveSampleCountLimit,	CT_FLOAT, 1 ),

		SHADER_CONSTANT_ELEMENT( ASSAOConstants, InvSharpness,				CT_FLOAT, 1 ),
		SHADER_CONSTANT_ELEMENT( ASSAOConstants, PassIndex,					CT_INT, 1 ),
		SHADER_CONSTANT_ELEMENT( ASSAOConstants, QuarterResPixelSize,		CT_VECTOR_2, 1 ),

		SHADER_CONSTANT_ELEMENT( ASSAOConstants, PatternRotScaleMatrices,	CT_VECTOR_4, 5 ),

		SHADER_CONSTANT_ELEMENT( ASSAOConstants, NormalsUnpackMul,			CT_FLOAT, 1 ),
		SHADER_CONSTANT_ELEMENT( ASSAOConstants, NormalsUnpackAdd,			CT_FLOAT, 1 ),
		SHADER_CONSTANT_ELEMENT( ASSAOConstants, DetailAOStrength,			CT_FLOAT, 1 ),
		SHADER_CONSTANT_ELEMENT( ASSAOConstants, Dummy0,					CT_FLOAT, 1 ),
#ifdef SSAO_ENABLE_NORMAL_WORLD_TO_VIEW_CONVERSION
		SHADER_CONSTANT_ELEMENT( ASSAOConstants, NormalsWorldToViewspaceMatrix,			CT_MATRIX_43, 1 ),
#endif		
		SHADER_CONSTANT_END()
	};	


	static const DescriptorDeclaration g_descriptOneTex[] =
	{
		DESCRIPTOR_ELEMENT(0,						 DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, SHADER_FLAG_PIXEL),
		DESCRIPTOR_ELEMENT(SHADER_CONSTANT_MATERIAL, DESCRIPTOR_TYPE_CONSTANT_BUFFER, 1, SHADER_FLAG_PIXEL),
		DESCRIPTOR_END()
	};

	static const DescriptorDeclaration g_descriptTwoTex[] =
	{
		DESCRIPTOR_ELEMENT(0,						 DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, SHADER_FLAG_PIXEL),
		DESCRIPTOR_ELEMENT(SHADER_CONSTANT_MATERIAL, DESCRIPTOR_TYPE_CONSTANT_BUFFER, 1, SHADER_FLAG_PIXEL),
		DESCRIPTOR_END()
	};

	static const DescriptorDeclaration g_descriptFourTex[] =
	{
		DESCRIPTOR_ELEMENT(0,	DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, SHADER_FLAG_PIXEL),
		DESCRIPTOR_ELEMENT(1,	DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, SHADER_FLAG_PIXEL),
		DESCRIPTOR_ELEMENT(2,	DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, SHADER_FLAG_PIXEL),
		DESCRIPTOR_ELEMENT(3,	DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, SHADER_FLAG_PIXEL),
		DESCRIPTOR_ELEMENT(SHADER_CONSTANT_MATERIAL, DESCRIPTOR_TYPE_CONSTANT_BUFFER, 1, SHADER_FLAG_PIXEL),
		DESCRIPTOR_END()
	};

	static const DescriptorDeclaration g_descriptGenerateQ[] =
	{
		DESCRIPTOR_ELEMENT(0,	DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, SHADER_FLAG_PIXEL),
		DESCRIPTOR_ELEMENT(1,	DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, SHADER_FLAG_PIXEL),
		DESCRIPTOR_ELEMENT(2,	DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, SHADER_FLAG_PIXEL),
		DESCRIPTOR_ELEMENT(3,	DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, SHADER_FLAG_PIXEL),
		DESCRIPTOR_ELEMENT(4,	DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, SHADER_FLAG_PIXEL),
		DESCRIPTOR_ELEMENT(SHADER_CONSTANT_MATERIAL, DESCRIPTOR_TYPE_CONSTANT_BUFFER, 1, SHADER_FLAG_PIXEL),
		DESCRIPTOR_END()
	};


	ASSAO::ASSAO()
	{
		SetRenderMask(RENDER_MASK_POST_EFFECT);
		// Don't really care if before or after but going with after
		SetLayer(LAYER_SKY);
		SetPriority(2);

		m_pSys = nullptr;
		m_pDest = nullptr;
		m_iQualityLevel = 4;
		m_bHasLinearDepth = false;
	}

	ASSAO::~ASSAO()
	{

	}

	void ASSAO::Init(GFXDevice* pDevice, ResourceMgr* pRes, PostFXSys* pSys, RenderTarget* pDst)
	{
		m_pSys = pSys;
		// Get the handles for the various descriptor layouts
		DescriptorSetLayoutHndl desc1Tex = pDevice->GetDescriptorSetLayout(g_descriptOneTex);
		DescriptorSetLayoutHndl desc2Tex = pDevice->GetDescriptorSetLayout(g_descriptTwoTex);
		DescriptorSetLayoutHndl desc4Tex = pDevice->GetDescriptorSetLayout(g_descriptFourTex);
		DescriptorSetLayoutHndl descGenQ = pDevice->GetDescriptorSetLayout(g_descriptGenerateQ);
		usg::ColorBuffer* pBuffers[4];


		SamplerDecl pointDecl(SF_POINT, SC_CLAMP);
		SamplerDecl linearDecl(SF_LINEAR, SC_CLAMP);

		m_pointSampler = pDevice->GetSampler(pointDecl);
		pointDecl.SetClamp(SC_MIRROR);
		m_pointMirrorSampler = pDevice->GetSampler(pointDecl);
		m_linearSampler = pDevice->GetSampler(linearDecl);

		Vector2i halfSize;
		halfSize.x = pDst->GetWidth() + 1 / 2;
		halfSize.y = pDst->GetHeight() + 1 / 2;
		Vector2i quarterSize;
		quarterSize.x = halfSize.x + 1 / 2;
		quarterSize.y = halfSize.x + 1 / 2;
		for (int i = 0; i < DEPTH_COUNT; i++)
		{
			m_halfDepthTargets[i].Init(pDevice, halfSize.x, halfSize.y, CF_R_16F, usg::SAMPLE_COUNT_1_BIT, TU_FLAGS_OFFSCREEN_COLOR, i, MIP_COUNT);
			pBuffers[i] = &m_halfDepthTargets[i];
		}

		m_fourDepthRT.InitMRT(pDevice, DEPTH_COUNT, pBuffers, nullptr );
		pBuffers[1] = &m_halfDepthTargets[3];	// Demo uses 0 and 3, we'll do the same to avoid confusion for now
		m_twoDepthRT.InitMRT(pDevice, 2, pBuffers, nullptr);

		m_pingPongCB1.Init(pDevice, halfSize.x, halfSize.y, CF_RG_8);
		m_pingPongCB2.Init(pDevice, halfSize.x, halfSize.y, CF_RG_8);
		m_pingPongRT1.Init(pDevice, &m_pingPongCB1);
		m_pingPongRT2.Init(pDevice, &m_pingPongCB2);

		m_importanceMapCB.Init(pDevice, quarterSize.x, quarterSize.y, CF_R_8);
		m_importanceMapPongCB.Init(pDevice, quarterSize.x, quarterSize.y, CF_R_8);
		m_importanceMapRT.Init(pDevice, &m_importanceMapCB);
		m_importanceMapPongRT.Init(pDevice, &m_importanceMapPongCB);


		m_finalResultsCB.InitCube(pDevice, halfSize.x, halfSize.y, 4, CF_RG_8);
		m_finalResultsRT.Init(pDevice, &m_finalResultsCB);

		RenderTarget::RenderPassFlags flags;
		flags.uClearFlags = 0;
		flags.uStoreFlags = RenderTarget::RT_FLAG_COLOR_0| RenderTarget::RT_FLAG_COLOR_1| RenderTarget::RT_FLAG_COLOR_2| RenderTarget::RT_FLAG_COLOR_3;
		flags.uShaderReadFlags = RenderTarget::RT_FLAG_COLOR_0 | RenderTarget::RT_FLAG_COLOR_1 | RenderTarget::RT_FLAG_COLOR_2 | RenderTarget::RT_FLAG_COLOR_3;
		m_fourDepthRT.InitRenderPass(pDevice, flags);
		flags.uStoreFlags = RenderTarget::RT_FLAG_COLOR_0 | RenderTarget::RT_FLAG_COLOR_3;
		flags.uShaderReadFlags = RenderTarget::RT_FLAG_COLOR_0 | RenderTarget::RT_FLAG_COLOR_3;
		m_twoDepthRT.InitRenderPass(pDevice, flags);
		flags.uStoreFlags = RenderTarget::RT_FLAG_COLOR_0;
		flags.uShaderReadFlags = RenderTarget::RT_FLAG_COLOR_0;
		m_pingPongRT1.InitRenderPass(pDevice, flags);
		m_pingPongRT2.InitRenderPass(pDevice, flags);
		m_finalResultsRT.InitRenderPass(pDevice, flags);

		m_importanceMapRT.InitRenderPass(pDevice, flags);
		m_importanceMapPongRT.InitRenderPass(pDevice, flags);

		m_constants.Init(pDevice, g_assaoConstantDef);

		PipelineStateDecl pipelineDecl;
		pipelineDecl.inputBindings[0].Init(GetVertexDeclaration(VT_POSITION));
		pipelineDecl.uInputBindingCount = 1;
		pipelineDecl.alphaState.SetColor0Only();

		RasterizerStateDecl& rasDecl = pipelineDecl.rasterizerState;
		rasDecl.eCullFace = CULL_FACE_NONE;

		pipelineDecl.layout.descriptorSets[0] = pDevice->GetDescriptorSetLayout(SceneConsts::g_globalDescriptorDecl);
		pipelineDecl.layout.uDescriptorSetCount = 2;

		pipelineDecl.layout.descriptorSets[1] = desc1Tex;
		pipelineDecl.pEffect = pRes->GetEffect(pDevice, "ASSAO.PrepareDepths");

		// All the depth targets should have the same render pass
		m_prepareDepthEffect = pDevice->GetPipelineState(m_fourDepthRT.GetRenderPass(), pipelineDecl);
		pipelineDecl.pEffect = pRes->GetEffect(pDevice, "ASSAO.PrepareDepths.lin");
		m_prepareDepthEffectLin = pDevice->GetPipelineState(m_fourDepthRT.GetRenderPass(), pipelineDecl);

		m_prepareDepthDesc.Init(pDevice, pipelineDecl.layout.descriptorSets[1]);
		m_prepareDepthDesc.SetConstantSetAtBinding(SHADER_CONSTANT_MATERIAL, &m_constants);

		pipelineDecl.pEffect = pRes->GetEffect(pDevice, "ASSAO.PrepareDepthsHalf");
		m_prepareDepthHalfEffect = pDevice->GetPipelineState(m_twoDepthRT.GetRenderPass(), pipelineDecl);
		pipelineDecl.pEffect = pRes->GetEffect(pDevice, "ASSAO.PrepareDepthsHalf.lin");
		m_prepareDepthHalfEffectLin = pDevice->GetPipelineState(m_twoDepthRT.GetRenderPass(), pipelineDecl);

		pipelineDecl.layout.descriptorSets[1] = desc4Tex;
		pipelineDecl.pEffect = pRes->GetEffect(pDevice, "ASSAO.PrepareDepthMip.1");
		m_mipPasses[0] = pDevice->GetPipelineState(m_fourDepthRT.GetRenderPass(), pipelineDecl);

		pipelineDecl.pEffect = pRes->GetEffect(pDevice, "ASSAO.PrepareDepthMip.2");
		m_mipPasses[1] = pDevice->GetPipelineState(m_fourDepthRT.GetRenderPass(), pipelineDecl);

		pipelineDecl.pEffect = pRes->GetEffect(pDevice, "ASSAO.PrepareDepthMip.3");
		m_mipPasses[2] = pDevice->GetPipelineState(m_fourDepthRT.GetRenderPass(), pipelineDecl);

		for (int i = 0; i < MIP_COUNT - 1; i++)
		{
			m_mipDesc[i].Init(pDevice, desc4Tex);
			m_mipDesc[i].SetConstantSetAtBinding(SHADER_CONSTANT_MATERIAL, &m_constants);
			ImageViewDef viewDef;
			viewDef.uMipCount = 1;
			for(int j=0; j<DEPTH_COUNT; j++)
			{
				viewDef.uBaseMip = i;
				m_mipDesc[i].SetImageSamplerPairAtBinding(j, m_halfDepthTargets[j].GetTexture(), m_pointSampler, 0,  viewDef);
			}
			m_mipDesc[i].UpdateDescriptors(pDevice);
		}

		// Both ping pongs are the same format, so render pass will be the same
		pipelineDecl.layout.descriptorSets[1] = desc1Tex;
		pipelineDecl.pEffect = pRes->GetEffect(pDevice, "ASSAO.SmartBlur");
		m_smartBlurEffect = pDevice->GetPipelineState(m_pingPongRT1.GetRenderPass(), pipelineDecl);

		pipelineDecl.pEffect = pRes->GetEffect(pDevice, "ASSAO.NonSmartBlur");
		m_nonSmartBlurEffect = pDevice->GetPipelineState(m_pingPongRT1.GetRenderPass(), pipelineDecl);

		pipelineDecl.pEffect = pRes->GetEffect(pDevice, "ASSAO.SmartBlurWide");
		m_smartBlurWide = pDevice->GetPipelineState(m_pingPongRT1.GetRenderPass(), pipelineDecl);

		pipelineDecl.layout.descriptorSets[1] = descGenQ;
		pipelineDecl.pEffect = pRes->GetEffect(pDevice, "ASSAO.GenerateQ.0");
		m_genQPasses[0] = pDevice->GetPipelineState(m_pingPongRT1.GetRenderPass(), pipelineDecl);

		pipelineDecl.pEffect = pRes->GetEffect(pDevice, "ASSAO.GenerateQ.1");
		m_genQPasses[1] = pDevice->GetPipelineState(m_pingPongRT1.GetRenderPass(), pipelineDecl);

		pipelineDecl.pEffect = pRes->GetEffect(pDevice, "ASSAO.GenerateQ.2");
		m_genQPasses[2] = pDevice->GetPipelineState(m_pingPongRT1.GetRenderPass(), pipelineDecl);

		pipelineDecl.pEffect = pRes->GetEffect(pDevice, "ASSAO.GenerateQ.3");
		m_genQPasses[3] = pDevice->GetPipelineState(m_pingPongRT1.GetRenderPass(), pipelineDecl);

		pipelineDecl.pEffect = pRes->GetEffect(pDevice, "ASSAO.GenerateQ.3Base");
		m_genQPasses[4] = pDevice->GetPipelineState(m_pingPongRT1.GetRenderPass(), pipelineDecl);


		pipelineDecl.layout.descriptorSets[1] = desc1Tex;

		pipelineDecl.pEffect = pRes->GetEffect(pDevice, "ASSAO.GenImportanceMap");
		m_genImportanceMap = pDevice->GetPipelineState(m_importanceMapRT.GetRenderPass(), pipelineDecl);

		pipelineDecl.pEffect = pRes->GetEffect(pDevice, "ASSAO.PPImportanceMapA");
		m_importanceMapA = pDevice->GetPipelineState(m_importanceMapRT.GetRenderPass(), pipelineDecl);

		pipelineDecl.pEffect = pRes->GetEffect(pDevice, "ASSAO.PPImportanceMapB");
		m_importanceMapB = pDevice->GetPipelineState(m_importanceMapRT.GetRenderPass(), pipelineDecl);


		// The final passes need to blend the color
		pipelineDecl.alphaState.bBlendEnable = true;
		pipelineDecl.alphaState.srcBlend = BLEND_FUNC_ZERO;
		pipelineDecl.alphaState.srcBlendAlpha = BLEND_FUNC_ZERO;
		pipelineDecl.alphaState.srcBlend = BLEND_FUNC_SRC_COLOR;
		pipelineDecl.alphaState.srcBlendAlpha = BLEND_FUNC_SRC_ALPHA;

		pipelineDecl.pEffect = pRes->GetEffect(pDevice, "ASSAO.NonSmartApply");
		m_nonSmartApplyEffect = pDevice->GetPipelineState(pDst->GetRenderPass(), pipelineDecl);

		pipelineDecl.pEffect = pRes->GetEffect(pDevice, "ASSAO.Apply");
		m_applyEffect = pDevice->GetPipelineState(pDst->GetRenderPass(), pipelineDecl);

		pipelineDecl.pEffect = pRes->GetEffect(pDevice, "ASSAO.NonSmartHalfApply");
		m_nonSmartHalfApplyEffect = pDevice->GetPipelineState(pDst->GetRenderPass(), pipelineDecl);

	}

	void ASSAO::CleanUp(GFXDevice* pDevice)
	{
		m_constants.CleanUp(pDevice);
		m_prepareDepthDesc.CleanUp(pDevice);
		for(int i=0; i<MIP_COUNT-1; i++)
		{
			m_mipDesc[i].CleanUp(pDevice);
		}
	}

	void ASSAO::SetDestTarget(GFXDevice* pDevice, RenderTarget* pDst)
	{
		pDevice->ChangePipelineStateRenderPass(pDst->GetRenderPass(), m_nonSmartApplyEffect);
		pDevice->ChangePipelineStateRenderPass(pDst->GetRenderPass(), m_applyEffect);
		pDevice->ChangePipelineStateRenderPass(pDst->GetRenderPass(), m_nonSmartHalfApplyEffect);

		m_pDest = pDst;
	}

	void ASSAO::Resize(GFXDevice* pDevice, uint32 uWidth, uint32 uHeight)
	{
		Vector2i halfSize;
		halfSize.x = uWidth + 1 / 2;
		halfSize.y = uHeight + 1 / 2;
		Vector2i quarterSize;
		quarterSize.x = halfSize.x + 1 / 2;
		quarterSize.y = halfSize.x + 1 / 2;
		for (int i = 0; i < DEPTH_COUNT; i++)
		{
			m_halfDepthTargets[i].Resize(pDevice, halfSize.x, halfSize.y);
		}

		m_fourDepthRT.Resize(pDevice);
		m_twoDepthRT.Resize(pDevice);

		m_pingPongCB1.Resize(pDevice, halfSize.x, halfSize.y);
		m_pingPongCB2.Resize(pDevice, halfSize.x, halfSize.y);
		m_pingPongRT1.Resize(pDevice);
		m_pingPongRT2.Resize(pDevice);

		m_importanceMapCB.Resize(pDevice, quarterSize.x, quarterSize.y);
		m_importanceMapPongCB.Resize(pDevice, quarterSize.x, quarterSize.y);
		m_importanceMapRT.Resize(pDevice);
		m_importanceMapPongRT.Resize(pDevice);


		m_finalResultsCB.Resize(pDevice, halfSize.x, halfSize.y);
		m_finalResultsRT.Resize(pDevice);


		for(int i=0; i<MIP_COUNT-1; i++)
		{
			m_mipDesc[i].UpdateDescriptors(pDevice);
		}

		m_prepareDepthDesc.UpdateDescriptors(pDevice);

	}


	void ASSAO::SetDepthSource(GFXDevice* pDevice, DepthStencilBuffer* pSrc)
	{
		m_prepareDepthDesc.SetImageSamplerPairAtBinding(0, pSrc->GetTexture(), m_pointSampler);
		m_prepareDepthDesc.UpdateDescriptors(pDevice);
		m_bHasLinearDepth = false;
	}

	void ASSAO::SetLinearDepthSource(GFXDevice* pDevice, ColorBuffer* pSrc)
	{
		m_prepareDepthDesc.SetImageSamplerPairAtBinding(0, pSrc->GetTexture(), m_pointSampler);
		m_prepareDepthDesc.UpdateDescriptors(pDevice);
		m_bHasLinearDepth = true;
	}

	bool ASSAO::Draw(GFXContext* pContext, RenderContext& renderContext)
	{
		if (!GetEnabled())
			return false;

		pContext->BeginGPUTag("ASSAO", Color::Green);

		// Prepare depths
		if (m_bHasLinearDepth)
		{
			if(m_iQualityLevel >= 0)
			{
				pContext->SetRenderTarget(&m_fourDepthRT);
				pContext->SetDescriptorSet(&m_prepareDepthDesc, 1);
				pContext->SetPipelineState(m_prepareDepthEffectLin);
			}
			else
			{
				pContext->SetRenderTarget(&m_twoDepthRT);
				pContext->SetDescriptorSet(&m_prepareDepthDesc, 1);
				pContext->SetPipelineState(m_prepareDepthHalfEffectLin);
			}
		}
		else
		{
			if (m_iQualityLevel >= 0)
			{
				pContext->SetRenderTarget(&m_fourDepthRT);
				pContext->SetDescriptorSet(&m_prepareDepthDesc, 1);
				pContext->SetPipelineState(m_prepareDepthEffect);
			}
			else
			{
				pContext->SetRenderTarget(&m_twoDepthRT);
				pContext->SetDescriptorSet(&m_prepareDepthDesc, 1);
				pContext->SetPipelineState(m_prepareDepthHalfEffect);
			}
		}
		m_pSys->DrawFullScreenQuad(pContext);

		if (m_iQualityLevel > 1)
		{
			// Prepare the depth mips
			for (int i = 0; i < MIP_COUNT-1; i++)
			{
				pContext->SetRenderTargetMip(&m_fourDepthRT, i+1);
				pContext->SetDescriptorSet(&m_mipDesc[i], 1);
				pContext->SetPipelineState(m_mipPasses[i]);
				m_pSys->DrawFullScreenQuad(pContext);
			}
		}


		pContext->SetRenderTarget(m_pDest);

		pContext->EndGPUTag();

		return true;
	}


}
