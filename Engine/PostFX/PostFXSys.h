/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Responsible for managing render targets and the various effects
*****************************************************************************/
#ifndef _USG_POSTFX_POSTFXSYS_H_
#define _USG_POSTFX_POSTFXSYS_H_

#include "Engine/Graphics/Materials/Material.h"
#include "Engine/Graphics/Effects/Effect.h"
#include "Engine/Graphics/Primitives/VertexBuffer.h"
#include "Engine/Graphics/Primitives/IndexBuffer.h"
#include "Engine/Scene/RenderNode.h"
#include FRAGMENT_HEADER(Engine/PostFX, PostFXSys_ps.h)

namespace usg{
	
class ViewContext;
class PostEffect;


class PostFXSys
{
public:
	enum TransferFlags
	{
		TRANSFER_FLAGS_NONE = 0,
		TRANSFER_FLAGS_HDR = (1 << 0),
		TRANSFER_FLAGS_CLEAR = (1 << 1),
		TRANSFER_FLAGS_G_BUFFER = (1 << 2),
		TRANSFER_FLAGS_FSAA = (1 << 3)
	};

	enum EffectFlags
	{
		EFFECT_FXAA = (1 << 0),
		EFFECT_BLOOM = (1 << 1),
		EFFECT_MOTION_BLUR = (1 << 2),
		EFFECT_DEFERRED_SHADING = (1 << 3),
		EFFECT_SKY_FOG = (1 << 4),
		EFFECT_SMAA = (1 << 5),
		EFFECT_FILM_GRAIN = (1<<6)
	};

	PostFXSys();
	~PostFXSys();

	void Init(GFXDevice* pDevice, ResourceMgr* pResource, uint32 uWidth, uint32 uHeight, uint32 uEffectFlags);
	void CleanUp(GFXDevice* pDevice);

	void Update(float fElapsed) { m_platform.Update(fElapsed); }
	void UpdateGPU(GFXDevice* pDevice) { m_platform.UpdateGPU(pDevice); }

	void EnableEffects(GFXDevice* pDevice, uint32 uEffectFlags);

	void Resize(GFXDevice* pDevice, uint32 uWidth, uint32 uHeight);

	// TODO: Remove passing in scene!!
	RenderTarget* BeginScene(GFXContext* pCtxt, uint32 uTransferFlags);
	
	RenderTarget* GetInitialRT() { return m_platform.GetInitialRT(); }
	void EndScene();

	// FIXME: Pass in the right value
	const SceneRenderPasses& GetRenderPasses() const { return m_platform.GetRenderPasses(); }
	SceneRenderPasses& GetRenderPasses() { return m_platform.GetRenderPasses(); }

	void DrawFullScreenQuad(GFXContext* pCtxt) const;

	void SetSkyTexture(GFXDevice* pDevice, const TextureHndl& hndl);
	void UpdateRTSize(GFXDevice* pDevice, Display* pDisplay);
	void SetPostDepthDescriptors(GFXContext* pCtxt);

	uint32 GetFinalTargetWidth(bool bOrient = true) { return m_platform.GetFinalTargetWidth(bOrient); }
	uint32 GetFinalTargetHeight(bool bOrient = true) { return m_platform.GetFinalTargetHeight(bOrient); }
	float GetFinalTargetAspect() { return m_platform.GetFinalTargetAspect(); }
	RenderTarget* GetFinalRT();
	
	const GFXBounds GetBounds() const;

	void SetActiveViewContext(ViewContext* pViewContext) { m_pActiveScene = pViewContext; }
	ViewContext* GetActiveViewContext() { return m_pActiveScene; }

	uint32 GetPostEffectCount() const { return m_uPostEffects; }
	PostEffect* GetEffect(uint32 uEffectId);
	void RegisterEffect(PostEffect* pEffect);
	bool IsEffectEnabled(EffectFlags uFlag) { return (m_uEffectsEnabled & uFlag) != 0; }

	// Platform specific implementations
	void Copy(GFXContext* pContext, RenderTarget* pSrc, RenderTarget* pDst);

	const PostFXSys_ps& GetPlatform() const { return m_platform; }

	const TextureHndl& GetLinearDepthTex() const { return m_platform.GetLinearDepthTex();  }
	const TextureHndl& GetDummyDepth() { return m_dummyDepth; }

protected:
	PRIVATIZE_COPY(PostFXSys)

	PostFXSys_ps	m_platform;

	enum
	{
		MAX_POST_EFFECTS = 10
	};

	RenderTarget*		m_pDepthTarget;
	TextureHndl			m_dummyDepth;

	PostEffect*			m_pPostEffects[MAX_POST_EFFECTS];
	uint32				m_uPostEffects;

	ViewContext*		m_pActiveScene;

	uint32				m_uTargetWidth;
	uint32				m_uTargetHeight;
	uint32				m_uEffectsSupported;
	uint32				m_uEffectsEnabled;


	VertexBuffer		m_fullScreenVB;
	IndexBuffer			m_fullScreenIB;

	Material			m_copyMat;
};

}

#endif
