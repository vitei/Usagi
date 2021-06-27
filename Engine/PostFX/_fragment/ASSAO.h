/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Adaptive Screen Space Ambient Occlusion
//  For license details see LICENSE.MD
//  https://github.com/GameTechDev/ASSAO
*****************************************************************************/
#ifndef _USG_POSTFX_ASSAO_H_
#define _USG_POSTFX_ASSAO_H_

#include "Engine/Graphics/Effects/Effect.h"
#include "Engine/Graphics/Materials/Material.h"
#include "Engine/PostFX/PostEffect.h"

namespace usg {

class PostFXSys;
class Camera;

class ASSAO : public PostEffect
{
public:
	ASSAO();
	~ASSAO();

	virtual void Init(GFXDevice* pDevice, ResourceMgr* pResource, PostFXSys* pSys, RenderTarget* pDst);
	virtual void Cleanup(GFXDevice* pDevice);
	virtual void SetDestTarget(GFXDevice* pDevice, RenderTarget* pDst);
	virtual void Resize(GFXDevice* pDevice, uint32 uWidth, uint32 uHeight);
	void SetDepthSource(GFXDevice* pDevice, DepthStencilBuffer* pSrc);
	void SetLinearDepthSource(GFXDevice* pDevice, ColorBuffer* pSrc, ColorBuffer* pNorm);
	virtual bool Draw(GFXContext* pContext, RenderContext& renderContext);
	virtual void Update(Scene* pScene, float fElapsed) override;
	virtual void UpdateBuffer(usg::GFXDevice* pDevice) override;

	virtual bool ReadsTexture(Input eInput) const override;
	virtual bool LoadsTexture(Input eInput) const override;
	virtual void SetTexture(GFXDevice* pDevice, Input eInput, const TextureHndl& texture) override;

private:
	void UpdateConstants(uint32 uWidth, uint32 uHeight, const usg::Camera* pCamera);
	void UpdatePassConstants(uint32 uPass, uint32 uWidth, uint32 uHeight);
	void GenerateSSAO(GFXContext* pContext, bool bAdaptive);

	struct ASSAO_Settings
	{
		float Radius = 6.f;//1.2f;
		float ShadowMultiplier = 1.0f;
		float ShadowPower = 1.50f;
		float ShadowClamp = 0.98f;
		float HorizonAngleThreshold = 0.06f;
		float FadeOutFrom = 900.0f;
		float FadeOutTo = 1000.0f;
		float AdaptiveQualityLimit = 0.45f;
		int	QualityLevel = 3;
		int BlurPassCount = 2;
		float Sharpness = 0.98f;
		float TemporalSupersamplingAngleOffset = 0.0f;
		float TemporalSupersamplingRadiusOffset = 1.0f;
		float DetailShadowStrength = 0.5f;
	};

	enum
	{
		GEN_Q_PASS_0 = 0,
		GEN_Q_PASS_1,
		GEN_Q_PASS_2,
		GEN_Q_PASS_3,
		GEN_Q_PASS_3_BASE,
		GEN_Q_PASS_COUNT,

		CONST_PASS_COUNT = 4,
		DEPTH_COUNT = 4,
		MIP_COUNT = 4
	};

	PostFXSys*		m_pSys;
	RenderTarget*	m_pDest;


	ASSAO_Settings	m_settings;

	int				m_iQualityLevel;
	ConstantSet		m_constants;
	ConstantSet		m_passConstants[CONST_PASS_COUNT];

	SamplerHndl		m_pointSampler;
	SamplerHndl		m_pointMirrorSampler;
	SamplerHndl		m_linearSampler;

	// Note these should all have MIP_COUNT mips
	ColorBuffer			m_halfDepthTargets[DEPTH_COUNT];
	ColorBuffer			m_pingPongCB1;
	ColorBuffer			m_pingPongCB2;
	ColorBuffer			m_finalResultsCB;
	ColorBuffer			m_importanceMapCB;
	ColorBuffer			m_importanceMapPongCB;
	ColorBuffer			m_loadTargetCB;

	RenderTarget		m_fourDepthRT;
	RenderTarget		m_twoDepthRT;
	RenderTarget		m_pingPongRT1;
	RenderTarget		m_pingPongRT2;
	RenderTarget		m_finalResultsRT;

	RenderTarget		m_importanceMapRT;
	RenderTarget		m_importanceMapPongRT;


	PipelineStateHndl	m_prepareDepthEffect;
	PipelineStateHndl	m_prepareDepthHalfEffect;
	PipelineStateHndl	m_prepareDepthEffectLin;
	PipelineStateHndl	m_prepareDepthHalfEffectLin;

	PipelineStateHndl	m_smartBlurEffect;
	PipelineStateHndl	m_nonSmartBlurEffect;
	PipelineStateHndl	m_smartBlurWide;
	PipelineStateHndl	m_mipPasses[MIP_COUNT-1];
	PipelineStateHndl	m_genQPasses[GEN_Q_PASS_COUNT];
	PipelineStateHndl	m_genImportanceMap;
	PipelineStateHndl	m_importanceMapA;
	PipelineStateHndl	m_importanceMapB;
	PipelineStateHndl	m_applyEffect;
	PipelineStateHndl	m_nonSmartApplyEffect;
	PipelineStateHndl	m_nonSmartHalfApplyEffect;

	DescriptorSet		m_applyDesc;
	DescriptorSet		m_prepareDepthDesc;
	DescriptorSet		m_mipDesc[MIP_COUNT-1];
	DescriptorSet		m_genQDesc[GEN_Q_PASS_COUNT][DEPTH_COUNT];
	DescriptorSet		m_blurDescPing;
	DescriptorSet		m_blurDescPong;
	DescriptorSet		m_importanceMapDesc;
	DescriptorSet		m_importanceMapADesc;
	DescriptorSet		m_importanceMapBDesc;

	bool				m_bHasLinearDepth;
	bool				m_bGenerateNormals;
};

}

#endif
