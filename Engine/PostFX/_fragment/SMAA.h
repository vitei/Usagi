/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Enhanced Subpixel Morphological Antialiasing
//  http://www.iryoku.com/smaa/
*****************************************************************************/
#ifndef _USG_POSTFX_SMAA_H_
#define _USG_POSTFX_SMAA_H_
#include "Engine/Common/Common.h"
#include "Engine/Graphics/Effects/Effect.h"
#include "Engine/Graphics/Materials/Material.h"
#include "Engine/PostFX/PostEffect.h"

namespace usg {

class PostFXSys;

class SMAA : public PostEffect
{
public:
	SMAA();
	~SMAA();

	virtual void Init(GFXDevice* pDevice, PostFXSys* pSys, RenderTarget* pDst);
	virtual void CleanUp(GFXDevice* pDevice);
	virtual void SetDestTarget(GFXDevice* pDevice, RenderTarget* pDst);
	virtual void Resize(GFXDevice* pDevice, uint32 uWidth, uint32 uHeight);
	void SetSourceTarget(GFXDevice* pDevice, RenderTarget* pTarget);
	virtual bool Draw(GFXContext* pContext, RenderContext& renderContext);

private:
	void UpdateConstants(GFXDevice* pDevice, uint32 uScrWidth, uint32 uScrHeight);
	void UpdateDescriptors(GFXDevice* pDevice);

	enum 
	{
		RT_EDGES = 0,
		RT_BLEND_WEIGHT,
		RT_COUNT
	};

	PostFXSys*		m_pSys;

	RenderTarget*			m_pDestTarget;
	ConstantSet				m_constantSet;

	DescriptorSet			m_lumaColorEdgeDescriptorSet;
	DescriptorSet			m_depthEdgeDescriptorSet;
	DescriptorSet			m_blendWeightDescriptorSet;
	DescriptorSet			m_neighbourBlendDescriptorSet;
	DescriptorSet			m_resolveDescriptorSet;

	PipelineStateHndl		m_depthEdgeDetectEffect;
	PipelineStateHndl		m_lumaEdgeDetectEffect;
	PipelineStateHndl		m_colorEdgeDetectEffect;
	PipelineStateHndl		m_blendWeightEffect;
	PipelineStateHndl		m_neighbourBlendEffect;
	PipelineStateHndl		m_resolveEffect;

	SamplerHndl				m_pointSampler;
	SamplerHndl				m_linearSampler;

	ColorBuffer				m_colorBuffers[RT_COUNT];
	RenderTarget			m_renderTargets[RT_COUNT];
};

}

#endif
