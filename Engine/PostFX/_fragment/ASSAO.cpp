/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
// Based on https://github.com/GameTechDev/ASSAO
// For license details see LICENSE.MD

#include "Engine/Common/Common.h"
#include "Engine/Maths/Vector2f.h"
#include "Engine/PostFX/PostFXSys.h"
#include "Engine/Graphics/GFX.h"
#include "Engine/Graphics/Device/GFXDevice.h"
#include "Engine/Resource/ResourceMgr.h"
#include "Engine/Scene/Camera/Camera.h"
#include "Engine/Scene/SceneConstantSets.h"
#include "Engine/Graphics/StandardVertDecl.h"
#include "Engine/Core/stl/vector.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/ViewContext.h"
#include "Engine/PostFX/PostFXSys.h"
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

	    Vector2f	Viewport2xPixelSize;
	    Vector2f	Viewport2xPixelSize_x_025;              // Viewport2xPixelSize * 0.25 (for fusing add+mul into mad)

	    float		EffectRadius;                           // world (viewspace) maximum size of the shadow
	    float		EffectShadowStrength;                   // global strength of the effect (0 - 5)
	    float		EffectShadowPow;
	    float		EffectShadowClamp;

	    float		EffectFadeOutMul;                       // effect fade out from distance (ex. 25)
	    float		EffectFadeOutAdd;                       // effect fade out to distance   (ex. 100)
	    float		EffectHorizonAngleThreshold;            // limit errors on slopes and caused by insufficient geometry tessellation (0.05 to 0.5)
	    float		EffectSampleRadiusNearLimitRec;          // if viewspace pixel closer than this, don't enlarge shadow sampling radius anymore (makes no sense to grow beyond some distance, not enough samples to cover everything, so just limit the shadow growth; could be SSAOSettingsFadeOutFrom * 0.1 or less)

	    float		DepthPrecisionOffsetMod;
	    float		NegRecEffectRadius;                     // -1.0 / EffectRadius
	    float		LoadCounterAvgDiv;                      // 1.0 / ( halfDepthMip[SSAO_DEPTH_MIP_LEVELS-1].sizeX * halfDepthMip[SSAO_DEPTH_MIP_LEVELS-1].sizeY )
	    float		AdaptiveSampleCountLimit;

	    float		InvSharpness;
	    Vector2f	QuarterResPixelSize;                    // used for importance map only

	    float		NormalsUnpackMul;
	    float		NormalsUnpackAdd;
	    float		DetailAOStrength;
	    float		FarDistance;

#ifdef SSAO_ENABLE_NORMAL_WORLD_TO_VIEW_CONVERSION
    mat4x4                NormalsWorldToViewspaceMatrix;
#endif
	};

	struct ASSAOPassConstants
	{
		Vector4f	PatternRotScaleMatrices[5];
		Vector2i	PerPassFullResCoordOffset;
		Vector2f	PerPassFullResUVOffset;
		int			PassIndex;
	};

	static const ShaderConstantDecl g_assaoConstantDef[] = 
	{
		SHADER_CONSTANT_ELEMENT( ASSAOConstants, ViewportPixelSize,			CT_VECTOR_2, 1 ),
		SHADER_CONSTANT_ELEMENT( ASSAOConstants, HalfViewportPixelSize,		CT_VECTOR_2, 1 ),

		SHADER_CONSTANT_ELEMENT( ASSAOConstants, DepthUnpackConsts,			CT_VECTOR_2, 1 ),
		SHADER_CONSTANT_ELEMENT( ASSAOConstants, CameraTanHalfFOV,			CT_VECTOR_2, 1 ),

		SHADER_CONSTANT_ELEMENT( ASSAOConstants, NDCToViewMul,				CT_VECTOR_2, 1 ),
		SHADER_CONSTANT_ELEMENT( ASSAOConstants, NDCToViewAdd,				CT_VECTOR_2, 1 ),		

		SHADER_CONSTANT_ELEMENT( ASSAOConstants, Viewport2xPixelSize,		CT_VECTOR_2, 1 ),
		SHADER_CONSTANT_ELEMENT( ASSAOConstants, Viewport2xPixelSize_x_025,	CT_VECTOR_2, 1 ),

		SHADER_CONSTANT_ELEMENT( ASSAOConstants, EffectRadius,				CT_FLOAT, 1 ),
		SHADER_CONSTANT_ELEMENT( ASSAOConstants, EffectShadowStrength,		CT_FLOAT, 1 ),
		SHADER_CONSTANT_ELEMENT( ASSAOConstants, EffectShadowPow,			CT_FLOAT, 1 ),
		SHADER_CONSTANT_ELEMENT( ASSAOConstants, EffectShadowClamp,			CT_FLOAT, 1 ),

		SHADER_CONSTANT_ELEMENT( ASSAOConstants, EffectFadeOutMul,			CT_FLOAT, 1 ),
		SHADER_CONSTANT_ELEMENT( ASSAOConstants, EffectFadeOutAdd,			CT_FLOAT, 1 ),
		SHADER_CONSTANT_ELEMENT( ASSAOConstants, EffectHorizonAngleThreshold,	CT_FLOAT, 1 ),
		SHADER_CONSTANT_ELEMENT( ASSAOConstants, EffectSampleRadiusNearLimitRec,	CT_FLOAT, 1 ),

		SHADER_CONSTANT_ELEMENT( ASSAOConstants, DepthPrecisionOffsetMod,	CT_FLOAT, 1 ),
		SHADER_CONSTANT_ELEMENT( ASSAOConstants, NegRecEffectRadius,		CT_FLOAT, 1 ),
		SHADER_CONSTANT_ELEMENT( ASSAOConstants, LoadCounterAvgDiv,			CT_FLOAT, 1 ),
		SHADER_CONSTANT_ELEMENT( ASSAOConstants, AdaptiveSampleCountLimit,	CT_FLOAT, 1 ),

		SHADER_CONSTANT_ELEMENT( ASSAOConstants, InvSharpness,				CT_FLOAT, 1 ),
		SHADER_CONSTANT_ELEMENT( ASSAOConstants, QuarterResPixelSize,		CT_VECTOR_2, 1 ),

		SHADER_CONSTANT_ELEMENT( ASSAOConstants, NormalsUnpackMul,			CT_FLOAT, 1 ),
		SHADER_CONSTANT_ELEMENT( ASSAOConstants, NormalsUnpackAdd,			CT_FLOAT, 1 ),
		SHADER_CONSTANT_ELEMENT( ASSAOConstants, DetailAOStrength,			CT_FLOAT, 1 ),
		SHADER_CONSTANT_ELEMENT( ASSAOConstants, FarDistance,				CT_FLOAT, 1 ),
#ifdef SSAO_ENABLE_NORMAL_WORLD_TO_VIEW_CONVERSION
		SHADER_CONSTANT_ELEMENT( ASSAOConstants, NormalsWorldToViewspaceMatrix,			CT_MATRIX_43, 1 ),
#endif		
		SHADER_CONSTANT_END()
	};	

	static const ShaderConstantDecl g_assaoPassConstantDef[] =
	{
		SHADER_CONSTANT_ELEMENT(ASSAOPassConstants, PatternRotScaleMatrices,	CT_VECTOR_4, 5),
		SHADER_CONSTANT_ELEMENT(ASSAOPassConstants, PerPassFullResCoordOffset,	CT_VECTOR2I, 1),
		SHADER_CONSTANT_ELEMENT(ASSAOPassConstants, PerPassFullResUVOffset,		CT_VECTOR_2, 1),
		SHADER_CONSTANT_ELEMENT(ASSAOPassConstants, PassIndex,					CT_INT, 1),
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
		DESCRIPTOR_ELEMENT(1,						 DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, SHADER_FLAG_PIXEL),
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

	static const DescriptorDeclaration g_descriptGenerateQAdpt[] =
	{
		DESCRIPTOR_ELEMENT(0,	DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, SHADER_FLAG_PIXEL),
		DESCRIPTOR_ELEMENT(1,	DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, SHADER_FLAG_PIXEL),
		DESCRIPTOR_ELEMENT(2,	DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, SHADER_FLAG_PIXEL),
		DESCRIPTOR_ELEMENT(3,	DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, SHADER_FLAG_PIXEL),
		DESCRIPTOR_ELEMENT(4,	DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, SHADER_FLAG_PIXEL),
		DESCRIPTOR_ELEMENT(SHADER_CONSTANT_MATERIAL, DESCRIPTOR_TYPE_CONSTANT_BUFFER, 1, SHADER_FLAG_PIXEL),
		DESCRIPTOR_ELEMENT(SHADER_CONSTANT_MATERIAL_1, DESCRIPTOR_TYPE_CONSTANT_BUFFER, 1, SHADER_FLAG_PIXEL),
		DESCRIPTOR_END()
	};

	static const DescriptorDeclaration g_descriptGenerateQ[] =
	{
		DESCRIPTOR_ELEMENT(0,	DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, SHADER_FLAG_PIXEL),
		DESCRIPTOR_ELEMENT(1,	DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, SHADER_FLAG_PIXEL),
		DESCRIPTOR_ELEMENT(SHADER_CONSTANT_MATERIAL, DESCRIPTOR_TYPE_CONSTANT_BUFFER, 1, SHADER_FLAG_PIXEL),
		DESCRIPTOR_ELEMENT(SHADER_CONSTANT_MATERIAL_1, DESCRIPTOR_TYPE_CONSTANT_BUFFER, 1, SHADER_FLAG_PIXEL),
		DESCRIPTOR_END()
	};

	static const DescriptorDeclaration g_descriptImportanceB[] =
	{
		DESCRIPTOR_ELEMENT(0,						 DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, SHADER_FLAG_PIXEL),
		DESCRIPTOR_ELEMENT(1,						 DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, SHADER_FLAG_PIXEL),
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
		m_bGenerateNormals = false;
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
		DescriptorSetLayoutHndl descGenQAdpt = pDevice->GetDescriptorSetLayout(g_descriptGenerateQAdpt);
		DescriptorSetLayoutHndl descImpB = pDevice->GetDescriptorSetLayout(g_descriptImportanceB);

		usg::ColorBuffer* pBuffers[4];


		SamplerDecl pointDecl(SF_POINT, SC_CLAMP);
		SamplerDecl linearDecl(SF_LINEAR, SC_CLAMP);

		m_pointSampler = pDevice->GetSampler(pointDecl);
		pointDecl.SetClamp(SC_MIRROR);
		m_pointMirrorSampler = pDevice->GetSampler(pointDecl);
		m_linearSampler = pDevice->GetSampler(linearDecl);

		Vector2i halfSize;
		halfSize.x = (pDst->GetWidth() + 1) / 2;
		halfSize.y = (pDst->GetHeight() + 1) / 2;
		Vector2i quarterSize;
		quarterSize.x = (halfSize.x + 1) / 2;
		quarterSize.y = (halfSize.x + 1) / 2;
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


		m_importanceMapDesc.Init(pDevice, desc1Tex);
		m_importanceMapADesc.Init(pDevice, desc1Tex);
		m_importanceMapBDesc.Init(pDevice, descImpB);
		m_importanceMapDesc.SetConstantSetAtBinding(SHADER_CONSTANT_MATERIAL, &m_constants);
		m_importanceMapDesc.SetImageSamplerPairAtBinding(0, m_finalResultsCB.GetTexture(), m_pointSampler);
		m_importanceMapADesc.SetConstantSetAtBinding(SHADER_CONSTANT_MATERIAL, &m_constants);
		m_importanceMapADesc.SetImageSamplerPairAtBinding(0, m_importanceMapCB.GetTexture(), m_pointSampler);
		m_importanceMapBDesc.SetConstantSetAtBinding(SHADER_CONSTANT_MATERIAL, &m_constants);
		m_importanceMapBDesc.SetImageSamplerPairAtBinding(0, m_importanceMapPongCB.GetTexture(), m_pointSampler);


		m_finalResultsCB.InitCube(pDevice, halfSize.x, halfSize.y, 4, CF_RG_8);
		m_loadTargetCB.Init(pDevice, 1, 1, CF_R_32);
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
		for(uint32 i=0; i< CONST_PASS_COUNT; i++)
		{
			m_passConstants[i].Init(pDevice, g_assaoPassConstantDef);
		}

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


		m_blurDescPing.Init(pDevice, desc1Tex);
		m_blurDescPong.Init(pDevice, desc1Tex);

		m_blurDescPing.SetImageSamplerPairAtBinding(0, m_pingPongCB1.GetTexture(), m_pointMirrorSampler);
		m_blurDescPong.SetImageSamplerPairAtBinding(0, m_pingPongCB2.GetTexture(), m_pointMirrorSampler);
		m_blurDescPing.SetConstantSetAtBinding(SHADER_CONSTANT_MATERIAL, &m_constants);
		m_blurDescPong.SetConstantSetAtBinding(SHADER_CONSTANT_MATERIAL, &m_constants);

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

		pipelineDecl.layout.descriptorSets[1] = descGenQAdpt;
		pipelineDecl.pEffect = pRes->GetEffect(pDevice, "ASSAO.GenerateQ.3");
		m_genQPasses[3] = pDevice->GetPipelineState(m_pingPongRT1.GetRenderPass(), pipelineDecl);

		pipelineDecl.pEffect = pRes->GetEffect(pDevice, "ASSAO.GenerateQ.3Base");
		m_genQPasses[4] = pDevice->GetPipelineState(m_pingPongRT1.GetRenderPass(), pipelineDecl);

		for (uint32 i = 0; i < GEN_Q_PASS_COUNT; i++)
		{
			for(uint32 uPass = 0; uPass < DEPTH_COUNT; uPass++)
			{
				m_genQDesc[i][uPass].Init(pDevice, i>=3 ? descGenQAdpt : descGenQ);
				m_genQDesc[i][uPass].SetImageSamplerPair(0, m_halfDepthTargets[uPass].GetTexture(), m_pointSampler);
				if (i >= 3)
				{
					m_genQDesc[i][uPass].SetImageSamplerPair(2, m_loadTargetCB.GetTexture(), m_pointSampler);
					m_genQDesc[i][uPass].SetImageSamplerPair(3, m_importanceMapCB.GetTexture(), m_pointSampler);
					m_genQDesc[i][uPass].SetImageSamplerPair(4, m_finalResultsCB.GetTexture(), m_pointSampler);
				}
				m_genQDesc[i][uPass].SetConstantSetAtBinding(SHADER_CONSTANT_MATERIAL, &m_constants);
				m_genQDesc[i][uPass].SetConstantSetAtBinding(SHADER_CONSTANT_MATERIAL_1, &m_passConstants[Math::Clamp((int)i, 0, CONST_PASS_COUNT-1)]);
				// We don't update these yet as we don't have the normals
			}
		}


		pipelineDecl.layout.descriptorSets[1] = desc1Tex;

		pipelineDecl.pEffect = pRes->GetEffect(pDevice, "ASSAO.GenImportanceMap");
		m_genImportanceMap = pDevice->GetPipelineState(m_importanceMapRT.GetRenderPass(), pipelineDecl);

		pipelineDecl.pEffect = pRes->GetEffect(pDevice, "ASSAO.PPImportanceMapA");
		m_importanceMapA = pDevice->GetPipelineState(m_importanceMapRT.GetRenderPass(), pipelineDecl);

		pipelineDecl.pEffect = pRes->GetEffect(pDevice, "ASSAO.PPImportanceMapB");
		pipelineDecl.layout.descriptorSets[1] = descImpB;
		m_importanceMapB = pDevice->GetPipelineState(m_importanceMapRT.GetRenderPass(), pipelineDecl);


		// The final passes need to blend the color
		pipelineDecl.alphaState.bBlendEnable = true;
		pipelineDecl.alphaState.srcBlend = BLEND_FUNC_ZERO;
		pipelineDecl.alphaState.srcBlendAlpha = BLEND_FUNC_ZERO;
		pipelineDecl.alphaState.dstBlend = BLEND_FUNC_SRC_COLOR;
		pipelineDecl.alphaState.dstBlendAlpha = BLEND_FUNC_SRC_ALPHA;

		m_applyDesc.Init(pDevice, desc1Tex);
		m_applyDesc.SetConstantSetAtBinding(SHADER_CONSTANT_MATERIAL, &m_constants);
		m_applyDesc.SetImageSamplerPairAtBinding(0, m_finalResultsCB.GetTexture(), m_pointSampler);

		pipelineDecl.pEffect = pRes->GetEffect(pDevice, "ASSAO.NonSmartApply");
		m_nonSmartApplyEffect = pDevice->GetPipelineState(pDst->GetRenderPass(), pipelineDecl);

		pipelineDecl.pEffect = pRes->GetEffect(pDevice, "ASSAO.Apply");


		m_applyEffect = pDevice->GetPipelineState(pDst->GetRenderPass(), pipelineDecl);

		pipelineDecl.pEffect = pRes->GetEffect(pDevice, "ASSAO.NonSmartHalfApply");
		m_nonSmartHalfApplyEffect = pDevice->GetPipelineState(pDst->GetRenderPass(), pipelineDecl);
	}


	void ASSAO::UpdatePassConstants(uint32 uPass, uint32 uWidth, uint32 uHeight)
	{
		ASSAOPassConstants* Consts = m_passConstants[uPass].Lock< ASSAOPassConstants>();

		int pass = 0;
		Consts->PerPassFullResCoordOffset = Vector2i(pass % 2, pass / 2);
		Consts->PerPassFullResUVOffset = Vector2f(((pass % 2) - 0.0f) / uWidth, ((pass / 2) - 0.0f) / uHeight);
		Consts->PassIndex = pass;
		float additionalAngleOffset = m_settings.TemporalSupersamplingAngleOffset;  // if using temporal supersampling approach (like "Progressive Rendering Using Multi-frame Sampling" from GPU Pro 7, etc.)
		float additionalRadiusScale = m_settings.TemporalSupersamplingRadiusOffset; // if using temporal supersampling approach (like "Progressive Rendering Using Multi-frame Sampling" from GPU Pro 7, etc.)
		const int subPassCount = 5;
		for (int subPass = 0; subPass < subPassCount; subPass++)
		{
			int a = pass;
			int b = subPass;

			int spmap[5]{ 0, 1, 4, 3, 2 };
			b = spmap[subPass];

			float ca, sa;
			float angle0 = ((float)a + (float)b / (float)subPassCount) * (3.1415926535897932384626433832795f) * 0.5f;
			angle0 += additionalAngleOffset;

			ca = ::cosf(angle0);
			sa = ::sinf(angle0);

			float scale = 1.0f + (a - 1.5f + (b - (subPassCount - 1.0f) * 0.5f) / (float)subPassCount) * 0.07f;
			scale *= additionalRadiusScale;

			Consts->PatternRotScaleMatrices[subPass] = Vector4f(scale * ca, scale * -sa, -scale * sa, -scale * ca);
		}

		m_passConstants[uPass].Unlock();
	}

	void ASSAO::UpdateConstants(uint32 uWidth, uint32 uHeight, const usg::Camera* pCamera)
	{

		usg::Matrix4x4 mProj = pCamera->GetProjection();
		float fNear = pCamera->GetNear();
		float fFar = pCamera->GetFar();
		ASSAOConstants* Consts = m_constants.Lock< ASSAOConstants>();

		Vector2i halfSize;
		halfSize.x = (uWidth + 1) / 2;
		halfSize.y = (uHeight + 1) / 2;
		Vector2i quarterSize;
		quarterSize.x = (halfSize.x + 1) / 2;
		quarterSize.y = (halfSize.x + 1) / 2;

		Consts->ViewportPixelSize.Assign(1.0f / (float)uWidth, 1.0f / (float)uHeight);
		Consts->HalfViewportPixelSize.Assign(1.0f / (float)halfSize.x, 1.0f / (float)halfSize.y);

		// TODO: Correct these values for vulkan. Using linear depth atm so getting ignored
		float depthLinearizeMul = -mProj[3][2];           // float depthLinearizeMul = ( clipFar * clipNear ) / ( clipFar - clipNear );
		float depthLinearizeAdd = mProj[2][2];           // float depthLinearizeAdd = clipFar / ( clipFar - clipNear );

		float tanHalfFOVY = -1.0f / mProj._22;    // = tanf( drawContext.Camera.GetYFOV( ) * 0.5f );
		float tanHalfFOVX = 1.0F / mProj._11;    // = tanHalfFOVY * drawContext.Camera.GetAspect( );

		Consts->DepthUnpackConsts.Assign(depthLinearizeMul, depthLinearizeAdd);
		Consts->CameraTanHalfFOV.Assign(tanHalfFOVX, tanHalfFOVY);

		Consts->NDCToViewMul.Assign(Consts->CameraTanHalfFOV.x * 2.0f, Consts->CameraTanHalfFOV.y * -2.0f);
		Consts->NDCToViewAdd.Assign(Consts->CameraTanHalfFOV.x * -1.0f, Consts->CameraTanHalfFOV.y * 1.0f);

		Consts->Viewport2xPixelSize.Assign(Consts->ViewportPixelSize.x * 2.0f, Consts->ViewportPixelSize.y * 2.0f);
		Consts->Viewport2xPixelSize_x_025.Assign(Consts->ViewportPixelSize.x * 0.25f, Consts->ViewportPixelSize.y * 0.25f);


		Consts->EffectRadius = Math::Clamp(m_settings.Radius, 0.0f, 100000.0f);
		Consts->EffectShadowStrength = Math::Clamp(m_settings.ShadowMultiplier * 4.3f, 0.0f, 10.0f);
		Consts->EffectShadowPow = Math::Clamp(m_settings.ShadowPower, 0.0f, 10.0f);
		Consts->EffectShadowClamp = Math::Clamp(m_settings.ShadowClamp, 0.0f, 1.0f);
		Consts->EffectFadeOutMul = -1.0f / (m_settings.FadeOutTo - m_settings.FadeOutFrom);
		Consts->EffectFadeOutAdd = m_settings.FadeOutFrom / (m_settings.FadeOutTo - m_settings.FadeOutFrom) + 1.0f;
		Consts->EffectHorizonAngleThreshold = Math::Clamp(m_settings.HorizonAngleThreshold, 0.0f, 1.0f);

		// Special settings for lowest quality level - just nerf the effect a tiny bit
		float effectSamplingRadiusNearLimit = (m_settings.Radius * 1.2f);
		if (m_iQualityLevel <= 0)
		{
			//consts.EffectShadowStrength     *= 0.9f;
			effectSamplingRadiusNearLimit *= 1.50f;

			if (m_iQualityLevel < 0)
				Consts->EffectRadius *= 0.8f;
		}
		effectSamplingRadiusNearLimit /= tanHalfFOVY; // to keep the effect same regardless of FOV

		Consts->EffectSampleRadiusNearLimitRec;          // if viewspace pixel closer than this, don't enlarge shadow sampling radius anymore (makes no sense to grow beyond some distance, not enough samples to cover everything, so just limit the shadow growth; could be SSAOSettingsFadeOutFrom * 0.1 or less)

		Consts->DepthPrecisionOffsetMod = 0.9992f;
		Consts->NegRecEffectRadius = -1.0f / Consts->EffectRadius;
		Consts->LoadCounterAvgDiv = 9.0f / (float)(quarterSize.x * quarterSize.y * 255.0);                      // 1.0 / ( halfDepthMip[SSAO_DEPTH_MIP_LEVELS-1].sizeX * halfDepthMip[SSAO_DEPTH_MIP_LEVELS-1].sizeY )
		Consts->AdaptiveSampleCountLimit = m_settings.AdaptiveQualityLimit;

		Consts->InvSharpness = Math::Clamp(1.0f - m_settings.Sharpness, 0.0f, 1.0f);
		Consts->QuarterResPixelSize.Assign(1.0f / (float)quarterSize.x, 1.0f / (float)quarterSize.y);


		// Our normals are a signed format, otherwise it'd be 2.0, -1.0
		Consts->NormalsUnpackMul = 1.0f;
		Consts->NormalsUnpackAdd = 0.0f;

		Consts->DetailAOStrength = m_settings.DetailShadowStrength;
		Consts->FarDistance = fFar;

#ifdef SSAO_ENABLE_NORMAL_WORLD_TO_VIEW_CONVERSION
		Consts->NormalsWorldToViewspaceMatrix = inputs->NormalsWorldToViewspaceMatrix;
#endif

		m_constants.Unlock();


		for(uint32 i=0; i< CONST_PASS_COUNT; i++)
		{
			UpdatePassConstants(i, uWidth, uHeight);
		}
	}

	void ASSAO::CleanUp(GFXDevice* pDevice)
	{
		for(int i=0; i< DEPTH_COUNT; i++)
		{
			m_halfDepthTargets[i].CleanUp(pDevice);
		}
		m_blurDescPing.CleanUp(pDevice);
		m_blurDescPong.CleanUp(pDevice);
		m_pingPongCB1.CleanUp(pDevice);
		m_applyDesc.CleanUp(pDevice);
		m_pingPongCB2.CleanUp(pDevice);
		m_finalResultsCB.CleanUp(pDevice);
		m_importanceMapCB.CleanUp(pDevice);
		m_importanceMapPongCB.CleanUp(pDevice);
		m_loadTargetCB.CleanUp(pDevice);

		m_importanceMapDesc.CleanUp(pDevice);
		m_importanceMapADesc.CleanUp(pDevice);
		m_importanceMapBDesc.CleanUp(pDevice);


		m_constants.CleanUp(pDevice);
		m_prepareDepthDesc.CleanUp(pDevice);
		for(int i=0; i<MIP_COUNT-1; i++)
		{
			m_mipDesc[i].CleanUp(pDevice);
		}

		for (int i = 0; i < CONST_PASS_COUNT; i++)
		{
			m_passConstants[i].CleanUp(pDevice);
		}	
		
		for (uint32 i = 0; i < GEN_Q_PASS_COUNT; i++)
		{
			for (uint32 uPass = 0; uPass < DEPTH_COUNT; uPass++)
			{
				m_genQDesc[i][uPass].CleanUp(pDevice);
			}
		}
	}

	void ASSAO::SetDestTarget(GFXDevice* pDevice, RenderTarget* pDst)
	{
		pDevice->ChangePipelineStateRenderPass(pDst->GetRenderPass(), m_nonSmartApplyEffect);
		pDevice->ChangePipelineStateRenderPass(pDst->GetRenderPass(), m_applyEffect);
		pDevice->ChangePipelineStateRenderPass(pDst->GetRenderPass(), m_nonSmartHalfApplyEffect);

		m_pDest = pDst;


	}

	void ASSAO::Update(Scene* pScene, float fElapsed)
	{
		const Camera* pCamera = pScene->GetViewContext(0)->GetCamera();
		UpdateConstants(m_pDest->GetWidth(), m_pDest->GetHeight(), pCamera);
	}
	
	void ASSAO::UpdateBuffer(usg::GFXDevice* pDevice)
	{
		m_constants.UpdateData(pDevice);
		for (int i = 0; i < CONST_PASS_COUNT; i++)
		{
			m_passConstants[i].UpdateData(pDevice);
		}

		for (int i = 0; i < MIP_COUNT-1; i++)
		{
			m_mipDesc[i].UpdateDescriptors(pDevice);
		}

		for (uint32 i = 0; i < GEN_Q_PASS_COUNT; i++)
		{
			for (uint32 uPass = 0; uPass < DEPTH_COUNT; uPass++)
			{
				m_genQDesc[i][uPass].UpdateDescriptors(pDevice);
			}
		}

		m_prepareDepthDesc.UpdateDescriptors(pDevice);
		m_blurDescPing.UpdateDescriptors(pDevice);
		m_blurDescPong.UpdateDescriptors(pDevice);
		m_applyDesc.UpdateDescriptors(pDevice);
		m_importanceMapDesc.UpdateDescriptors(pDevice);
		m_importanceMapADesc.UpdateDescriptors(pDevice);
		// TODO: Not yet working
		//m_importanceMapBDesc.UpdateDescriptors(pDevice);
	}

	void ASSAO::Resize(GFXDevice* pDevice, uint32 uWidth, uint32 uHeight)
	{
		Vector2i halfSize;
		halfSize.x = (uWidth + 1) / 2;
		halfSize.y = (uHeight + 1) / 2;
		Vector2i quarterSize;
		quarterSize.x = (halfSize.x + 1) / 2;
		quarterSize.y = (halfSize.x + 1) / 2;
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

	void ASSAO::SetLinearDepthSource(GFXDevice* pDevice, ColorBuffer* pSrc, ColorBuffer* pNorm)
	{
		m_prepareDepthDesc.SetImageSamplerPairAtBinding(0, pSrc->GetTexture(), m_pointSampler);
		m_prepareDepthDesc.UpdateDescriptors(pDevice);
		m_bHasLinearDepth = true;

		for (uint32 i = 0; i < GEN_Q_PASS_COUNT; i++)
		{
			for (uint32 uPass = 0; uPass < DEPTH_COUNT; uPass++)
			{
				m_genQDesc[i][uPass].SetImageSamplerPair(1, pNorm->GetTexture(), m_pointSampler);
			}
		}
	}


	void ASSAO::GenerateSSAO(GFXContext* pContext, bool bAdaptive)
	{
		static const int cMaxBlurPassCount = 6;

		if (bAdaptive)
		{
			ASSERT(m_settings.QualityLevel == 3);
		}

		int passCount = DEPTH_COUNT;

		for (int pass = 0; pass < passCount; pass++)
		{
			if ((m_settings.QualityLevel < 0) && ((pass == 1) || (pass == 2)))
				continue;

			int blurPasses = m_settings.BlurPassCount;
			blurPasses = Math::Min(blurPasses, cMaxBlurPassCount);

			if (m_settings.QualityLevel == 3)
			{
				// if adaptive, at least one blur pass needed as the first pass needs to read the final texture results - kind of awkward
				if (bAdaptive)
					blurPasses = 0;
				else
					blurPasses = Math::Max(1, blurPasses);
			}
			else
			{
				if (m_settings.QualityLevel <= 0)
				{
					// just one blur pass allowed for minimum quality 
					blurPasses = Math::Min(1, m_settings.BlurPassCount);
				}
			}


			RenderTarget* pPingRT = &m_pingPongRT1;
			RenderTarget* pPongRT = &m_pingPongRT2;
			DescriptorSet* pBlurPingDesc = &m_blurDescPing;
			DescriptorSet* pBlurPongDesc = &m_blurDescPong;

			// Generate
			{
				RenderTarget* rts[] = { pPingRT };

				if (blurPasses == 0)
				{
					pContext->SetRenderTargetLayer(&m_finalResultsRT, pass);
				}
				else
				{
					pContext->SetRenderTarget(pPingRT);
				}

				int shaderIndex = Math::Max(0, (!bAdaptive) ? (m_settings.QualityLevel) : (4));
				pContext->SetDescriptorSet(&m_genQDesc[shaderIndex][pass], 1);
				pContext->SetPipelineState(m_genQPasses[shaderIndex]);
				m_pSys->DrawFullScreenQuad(pContext);
			}

			// Blur
			if (blurPasses > 0)
			{
				int wideBlursRemaining = Math::Max(0, blurPasses - 2);

				for (int i = 0; i < blurPasses; i++)
				{
					// last pass?
					if (i == (blurPasses - 1))
					{
						pContext->SetRenderTargetLayer(&m_finalResultsRT, pass);
					}
					else
					{
						pContext->SetRenderTarget(pPongRT);

					}

					pContext->SetDescriptorSet(pBlurPingDesc, 1);

					if (m_settings.QualityLevel > 0)
					{
						if (wideBlursRemaining > 0)
						{
							pContext->SetPipelineState(m_smartBlurWide);
							wideBlursRemaining--;
						}
						else
						{
							pContext->SetPipelineState(m_smartBlurEffect);
						}
					}
					else
					{
						pContext->SetPipelineState(m_nonSmartBlurEffect);
					}

					m_pSys->DrawFullScreenQuad(pContext);

					Math::Swap(pPingRT, pPongRT);
					Math::Swap(pBlurPingDesc, pBlurPongDesc);
				}
			}
		}
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

		if (m_settings.QualityLevel == 3)
		{
			GenerateSSAO(pContext, true);
			
			// TODO: Need unordered access view unit to be implemented
			ASSERT(false);

			// Generate importance map
			pContext->SetRenderTarget(&m_importanceMapRT);
			pContext->SetDescriptorSet(&m_importanceMapDesc, 1);
			pContext->SetPipelineState(m_genImportanceMap);
			m_pSys->DrawFullScreenQuad(pContext);


			pContext->SetRenderTarget(&m_importanceMapRT);
			pContext->SetDescriptorSet(&m_importanceMapADesc, 1);
			pContext->SetPipelineState(m_importanceMapA);
			m_pSys->DrawFullScreenQuad(pContext);



			//UINT fourZeroes[4] = { 0, 0, 0, 0 };
			//dx11Context->ClearUnorderedAccessViewUint(m_loadCounterUAV, fourZeroes);
			//dx11Context->OMSetRenderTargetsAndUnorderedAccessViews(1, &m_importanceMap.RTV, NULL, SSAO_LOAD_COUNTER_UAV_SLOT, 1, &m_loadCounterUAV, NULL);
			pContext->SetDescriptorSet(&m_importanceMapBDesc, 1);
			pContext->SetPipelineState(m_importanceMapB);
			m_pSys->DrawFullScreenQuad(pContext);

		}

		GenerateSSAO(pContext, false);


		pContext->SetRenderTarget(m_pDest);

		pContext->SetDescriptorSet(&m_applyDesc, 1);

		if (m_settings.QualityLevel < 0)
		{
			pContext->SetPipelineState(m_nonSmartHalfApplyEffect);
		}
		else if (m_settings.QualityLevel == 0)
		{
			pContext->SetPipelineState(m_nonSmartApplyEffect);
		}
		else
		{
			pContext->SetPipelineState(m_applyEffect);
		}
		m_pSys->DrawFullScreenQuad(pContext);


		pContext->EndGPUTag();

		return true;
	}


}
