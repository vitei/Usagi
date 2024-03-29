/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Platform specific post process code
*****************************************************************************/
#ifndef _USG_POSTFX_FRAGMENT_POSTFXSYS_PS_H_
#define _USG_POSTFX_FRAGMENT_POSTFXSYS_PS_H_

#include "Engine/Graphics/Materials/Material.h"
#include "Engine/Graphics/Effects/Effect.h"
#include "Engine/Graphics/Primitives/VertexBuffer.h"
#include "Engine/Graphics/Primitives/IndexBuffer.h"
#include "Engine/Graphics/Textures/DepthStencilBuffer.h"
#include "Engine/Graphics/Textures/ColorBuffer.h"
#include "Engine/Graphics/Textures/RenderTarget.h"
#include "Engine/Graphics/Device/GFXHandles.h"
#include "Engine/PostFX/PostEffect.h"
#include "Engine/Core/stl/vector.h"
#include "Engine/Core/stl/list.h"
#include "Engine/Scene/RenderNode.h"
#include "Engine/Scene/SceneRenderPasses.h"

namespace usg{

class SceneContext;
class PostEffect;
class PostFXSys;
class LinearDepth;
class RenderTarget;
class ResourceMgr;
class Display;
class SMAA;

class PostFXSys_ps
{
public:
	PostFXSys_ps();
	~PostFXSys_ps();

	void Init(PostFXSys* pParent, ResourceMgr* pResMgr, GFXDevice* pDevice, uint32 uEffectFlags, uint32 uWidth, uint32 uHeight);
	void Cleanup(GFXDevice* pDevice);
	void Update(Scene* pScene, float fElapsed);
	void UpdateGPU(GFXDevice* pDevice);

	void Resize(GFXDevice* pDevice, uint32 uWidth, uint32 uHeight);

	void EnableEffects(GFXDevice* pDevice, uint32 uEffectFlags);

	void AddCustomEffect(PostEffect* pEffect);
	void RemoveCustomEffect(PostEffect* pEffect);

	const TextureHndl& GetLinearDepthTex() const;

	const SceneRenderPasses& GetRenderPasses() const;
	SceneRenderPasses& GetRenderPasses();
	void SetSkyTexture(GFXDevice* pDevice, const TextureHndl& hndl);
	
	uint32 GetFinalTargetWidth(bool bOrient ) { return m_colorBuffer[BUFFER_LDR_0].GetWidth(); }
	uint32 GetFinalTargetHeight(bool bOrient) { return m_colorBuffer[BUFFER_LDR_0].GetHeight(); }
	float GetFinalTargetAspect() { return  (float)m_colorBuffer[BUFFER_LDR_0].GetWidth() / (float)m_colorBuffer[BUFFER_LDR_0].GetHeight(); }

	// For setting up pipelines, will need the render pass in future
	PipelineStateHndl GetDownscale4x4Pipeline(GFXDevice* pDevice, ResourceMgr* pResource, const RenderPassHndl& renderPass) const;
	PipelineStateHndl GetGaussBlurPipeline(GFXDevice* pDevice, ResourceMgr* pResource, const RenderPassHndl& renderPass) const;
	void SetupDownscale4x4(GFXDevice* pDevice, ConstantSet& cb, DescriptorSet& des, uint32 uWidth, uint32 uHeight) const;
	void SetupGaussBlur(GFXDevice* pDevice, ConstantSet& cb, DescriptorSet& des, uint32 uWidth, uint32 uHeight, float fMultiplier) const;
	float GaussianDistribution(float x, float y, float rho) const;

	void UpdateRTSize(GFXDevice* pDevice, Display* pDisplay);

	RenderTarget* GetInitialRT();
	RenderTarget* GetFinalRT();

protected:
	PRIVATIZE_COPY(PostFXSys_ps)

	void SetupOffsets4x4(GFXDevice* pDevice, ConstantSet& cb, uint32 uWidth, uint32 uHeight) const;
	void SetupGaussBlurBuffer(GFXDevice* pDevice, ConstantSet& cb, uint32 uWidth, uint32 uHeight, float fMultiplier) const;
	void ResizeTargetsInt(GFXDevice* pDevice, uint32 uWidth, uint32 uHeight);

	void EnableEffectsInt(GFXDevice* pDevice, uint32 uEffectFlags);
	bool NeedsStoring(memsize pass, PostEffect::Input eInput);
	bool NeedsShaderRead(sint32 pass, PostEffect::Input eInput);
	void GetRenderTargetBuffers(memsize pass, usg::vector<ColorBuffer*>& pBuffers, int iFinalHdr, memsize& hdrIdx, memsize& ldrIdx);
	int GetFlagForTarget(PostEffect::Input eTarget);
	void ClearDynamicTargets(GFXDevice* pDevice);
	bool CanReuseTarget(memsize pass);
	bool IsLastTarget(memsize pass);
	TextureHndl GetTexture(PostEffect::Input eInput, bool bHDR, memsize hdrIdx, memsize ldrIdx);

	PostEffect* GetFinalEffect();

	enum COLOR_BUFFER
	{
		BUFFER_HDR_0= 0,
		BUFFER_HDR_1,
		BUFFER_NORMAL,
		BUFFER_DIFFUSE,
		BUFFER_LIN_DEPTH,
		BUFFER_EMISSIVE,
		BUFFER_SPECULAR,
		BUFFER_LDR_0,
		BUFFER_LDR_1,
		BUFFER_COUNT
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
	class FilmGrain*		m_pFilmGrain;
	class ASSAO*			m_pSSAO;
	class DeferredShading*	m_pDeferredShading;
	class SetSceneTarget*	m_pSetNoDepthTarget;
	PostEffect*				m_pDefaultEffects[MAX_DEFAULT_EFFECTS];
	vector<PostEffect*>		m_activeEffects;
	list<PostEffect*>		m_customEffects;
	PostEffect*				m_pFinalEffect;
	uint32					m_uDefaultEffects;
	float					m_fPixelScale;
	
	SceneRenderPasses		m_renderPasses;
	DepthStencilBuffer		m_depthStencil;
	ColorBuffer				m_colorBuffer[BUFFER_COUNT];
	vector<RenderTarget*>	m_dynamicTargets;

	SamplerHndl				m_linearSampler;
	SamplerHndl				m_pointSampler;

	bool					m_bHDROut;
};


}


#endif
