/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: A quick blit to screen function
*****************************************************************************/
#ifndef USG_POSTFX_BLIT_IMAGE
#define USG_POSTFX_BLIT_IMAGE
#include "Engine/Graphics/Effects/Effect.h"
#include "Engine/Graphics/Materials/Material.h"
#include "Engine/Scene/Common/CustomEffectRuntime.h"


namespace usg {

class BlitImage
{
public:
	BlitImage();
	~BlitImage();

	void InitForDisplay(GFXDevice* pDevice, usg::ResourceMgr* pResMgr, uint32 uDisplay = 0);
	void Init(GFXDevice* pDevice, EffectHndl effect, const RenderPassHndl& pass);
	void Cleanup(GFXDevice* pDevice);
	void ChangeRenderPass(GFXDevice* pDevice, const RenderPassHndl& pass);
	void SetSourceTexture(GFXDevice* pDevice, const TextureHndl& tex);
	bool Draw(GFXContext* pContext);

	usg::CustomEffectRuntime& GetRuntimeEffect() { return m_runtimeEffect; }
private:
	usg::CustomEffectRuntime	m_runtimeEffect;
	Material					m_material;
	VertexBuffer				m_fullScreenVB;
	IndexBuffer					m_fullScreenIB;
	SamplerHndl					m_sampler;
	bool						m_bCustomEffect;
};

}

#endif
