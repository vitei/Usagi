/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: A simplistic bloom implementation, no dynamic adaptation,
//	just bleeds out colour from superbright areas of the screen
*****************************************************************************/
#ifndef _USG_POSTFX_BLOOM_H_
#define _USG_POSTFX_BLOOM_H_

#include "Engine/Graphics/Effects/Effect.h"
#include "Engine/Graphics/Materials/Material.h"
#include "Engine/Graphics/Textures/ColorBuffer.h"
#include "Engine/Graphics/Textures/RenderTarget.h"
#include "Engine/PostFX/PostEffect.h"

namespace usg {

class PostFXSys;

class Bloom : public PostEffect
{
public:
	Bloom();
	~Bloom();

	virtual void Init(GFXDevice* pDevice, ResourceMgr* pResource, PostFXSys* pSys, RenderTarget* pDst);
	virtual void CleanUp(GFXDevice* pDevice);
	virtual void Resize(GFXDevice* pDevice, uint32 uWidth, uint32 uHeight);
	virtual void SetDestTarget(GFXDevice* pDevice, RenderTarget* pDst);
	void SetSourceTarget(GFXDevice* pDevice, RenderTarget* pTarget);
	virtual bool Draw(GFXContext* pContext, RenderContext& renderContext);

private:
	void GetOffsetsAndWeights(uint32 texSize, float fDeviation, float fMultiplier, float* pWeights, float* pOffsets);
	void SetOffsetsHor(GFXDevice* pDevice, uint32 uWidth, float fDeviation, float fMultiplier);
	void SetOffsetsVer(GFXDevice* pDevice, uint32 uHeight, float fDeviation, float fMultiplier);

	enum
	{
		BLOOM_PASS_TEXTURES = 3
	};

	enum
	{
		PASS_4X4 = 0,
		PASS_BRIGHT_PASS,
		PASS_GUASS_BRIGHT_PASS,
		PASS_GUASS_BLOOM_SRC,
		PASS_HOR_BLOOM,
		PASS_VER_BLOOM,
		PASS_FINAL,
		PASS_COUNT
	};

	PostFXSys*			m_pSys;
	RenderTarget*		m_pDestTarget;

	PipelineStateHndl	m_bloomEffect;
	PipelineStateHndl	m_brightPassEffect;
	PipelineStateHndl	m_finalPassEffect;
	PipelineStateHndl	m_gaussBlurPipeline;
	PipelineStateHndl	m_downscalePipeline;

	SamplerHndl		m_pointSampler;
	SamplerHndl		m_linearSampler;

	Material		m_bloomMat;

	ConstantSet		m_constants[PASS_COUNT];
	DescriptorSet	m_descriptors[PASS_COUNT];

	ColorBuffer		m_scaledSceneTex;
	ColorBuffer		m_brightPassTex;
	ColorBuffer		m_bloomSourceTex;
	ColorBuffer		m_bloomTex[BLOOM_PASS_TEXTURES];

	RenderTarget	m_scaledSceneRT;
	RenderTarget	m_brightPassRT;
	RenderTarget	m_bloomSourceRT;
	RenderTarget	m_bloomRT[BLOOM_PASS_TEXTURES];
};

}

#endif
