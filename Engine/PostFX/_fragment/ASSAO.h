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

class ASSAO : public PostEffect
{
public:
	ASSAO();
	~ASSAO();

	virtual void Init(GFXDevice* pDevice, ResourceMgr* pResource, PostFXSys* pSys, RenderTarget* pDst);
	virtual void CleanUp(GFXDevice* pDevice);
	virtual void SetDestTarget(GFXDevice* pDevice, RenderTarget* pDst);
	virtual void Resize(GFXDevice* pDevice, uint32 uWidth, uint32 uHeight);
	void SetSourceTarget(GFXDevice* pDevice, RenderTarget* pTarget);
	virtual bool Draw(GFXContext* pContext, RenderContext& renderContext);

private:

	enum
	{
		GEN_Q_PASS_0 = 0,
		GEN_Q_PASS_1,
		GEN_Q_PASS_2,
		GEN_Q_PASS_3,
		GEN_Q_PASS_3_BASE,
		GEN_Q_PASS_COUNT,

		DEPTH_COUNT = 4,
		MIP_COUNT = 4
	};

	PostFXSys*		m_pSys;

	ConstantSet		m_constants;

	SamplerHndl		m_pointSampler;
	SamplerHndl		m_linearSampler;

	// Note these should all have MIP_COUNT mips
	ColorBuffer			m_halfDepthTargets[DEPTH_COUNT];
	RenderTarget		m_fourDepthRT;
	RenderTarget		m_twoDepthRT;
	PipelineStateHndl	m_prepareDepthEffect;
	PipelineStateHndl	m_prepareDepthEffectHalf;
	PipelineStateHndl	m_mipPasses[MIP_COUNT-1];
	PipelineStateHndl	m_genQPasses[GEN_Q_PASS_COUNT];
};

}

#endif
