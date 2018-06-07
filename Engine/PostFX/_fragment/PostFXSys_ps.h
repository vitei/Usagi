/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Platform specific post process code
*****************************************************************************/
#ifndef _USG_POSTFX_FRAGMENT_POSTFXSYS_PS_H_
#define _USG_POSTFX_FRAGMENT_POSTFXSYS_PS_H_
#include "Engine/Common/Common.h"
#include "Engine/Graphics/Materials/Material.h"
#include "Engine/Graphics/Effects/Effect.h"
#include "Engine/Graphics/Primitives/VertexBuffer.h"
#include "Engine/Graphics/Primitives/IndexBuffer.h"
#include "Engine/Graphics/Textures/DepthStencilBuffer.h"
#include "Engine/Graphics/Textures/ColorBuffer.h"
#include "Engine/Graphics/Textures/RenderTarget.h"
#include "Engine/Graphics/Device/GFXHandles.h"
#include "Engine/Scene/RenderNode.h"
#include "Engine/Scene/SceneRenderPasses.h"

namespace usg{

class SceneContext;
class PostEffect;
class PostFXSys;
class LinearDepth;
class RenderTarget;
class Display;
class SMAA;

class PostFXSys_ps
{
public:
	PostFXSys_ps();
	~PostFXSys_ps();

	void Init(PostFXSys* pParent, GFXDevice* pDevice, uint32 uEffectFlags, uint32 uWidth, uint32 uHeight);
	void CleanUp(GFXDevice* pDevice);

	void Resize(GFXDevice* pDevice, uint32 uWidth, uint32 uHeight);

	void EnableEffects(GFXDevice* pDevice, uint32 uEffectFlags);

	const TextureHndl& GetLinearDepthTex() const;

	const SceneRenderPasses& GetRenderPasses() const;
	void SetSkyTexture(GFXDevice* pDevice, const TextureHndl& hndl);
	
	uint32 GetFinalTargetWidth(bool bOrient ) { return m_colorBuffer[BUFFER_LDR_0].GetWidth(); }
	uint32 GetFinalTargetHeight(bool bOrient) { return m_colorBuffer[BUFFER_LDR_0].GetHeight(); }
	float GetFinalTargetAspect() { return  (float)m_colorBuffer[BUFFER_LDR_0].GetWidth() / (float)m_colorBuffer[BUFFER_LDR_0].GetHeight(); }

	// For setting up pipelines, will need the render pass in future
	PipelineStateHndl GetDownscale4x4Pipeline(GFXDevice* pDevice, const RenderPassHndl& renderPass) const;
	PipelineStateHndl GetGaussBlurPipeline(GFXDevice* pDevice, const RenderPassHndl& renderPass) const;
	void SetupDownscale4x4(GFXDevice* pDevice, ConstantSet& cb, DescriptorSet& des, uint32 uWidth, uint32 uHeight) const;
	void SetupGaussBlur(GFXDevice* pDevice, ConstantSet& cb, DescriptorSet& des, uint32 uWidth, uint32 uHeight, float fMultiplier) const;
	float GaussianDistribution(float x, float y, float rho) const;

	void UpdateRTSize(GFXDevice* pDevice, Display* pDisplay);

	RenderTarget* GetInitialRT() { return m_pInitialTarget; }
	RenderTarget* GetFinalRT() { return m_pFinalTarget;  }

protected:
	PRIVATIZE_COPY(PostFXSys_ps)

	void SetupOffsets4x4(GFXDevice* pDevice, ConstantSet& cb, uint32 uWidth, uint32 uHeight) const;
	void SetupGaussBlurBuffer(GFXDevice* pDevice, ConstantSet& cb, uint32 uWidth, uint32 uHeight, float fMultiplier) const;
	void ResizeTargetsInt(GFXDevice* pDevice, uint32 uWidth, uint32 uHeight);

	enum COLOR_BUFFERS
	{
		BUFFER_HDR = 0,
		BUFFER_NORMAL,
		BUFFER_DIFFUSE,
		BUFFER_LIN_DEPTH,
		BUFFER_EMISSIVE,
		BUFFER_SPECULAR,
		BUFFER_LDR_0,
		BUFFER_LDR_1,
		BUFFER_COUNT
	};

	enum RENDER_TARGETS
	{
		TARGET_GBUFFER,
		TARGET_HDR,
		TARGET_HDR_LIN_DEPTH,
		TARGET_LDR_LIN_DEPTH,
		TARGET_LDR_0,
		TARGET_LDR_1,
		TARGET_COUNT
	};

	enum
	{
		MAX_DEFAULT_EFFECTS = 20
	};

	PostFXSys*				m_pParent;
	class SkyFog*			m_pSkyFog;
	class Bloom*			m_pBloom;
	class FXAA*				m_pFXAA;
	class SMAA*				m_pSMAA;
	class DeferredShading*	m_pDeferredShading;
	PostEffect*				m_pDefaultEffects[MAX_DEFAULT_EFFECTS];
	PostEffect*				m_pFinalEffect;
	uint32					m_uDefaultEffects;
	float					m_fPixelScale;
	
	SceneRenderPasses		m_renderPasses;
	DepthStencilBuffer		m_depthStencil;
	ColorBuffer				m_colorBuffer[BUFFER_COUNT];
	RenderTarget			m_screenRT[TARGET_COUNT];
	RenderTarget*			m_pFinalTarget;
	RenderTarget*			m_pInitialTarget;

//	PipelineStateHndl		m_downscale4x4Effect;
//	PipelineStateHndl		m_downscale2x2Effect;
//	PipelineStateHndl		m_gaussBlur5x5Effect;

	SamplerHndl				m_linearSampler;
	SamplerHndl				m_pointSampler;

	uint32					m_uLDRCount;
};


}


#endif
