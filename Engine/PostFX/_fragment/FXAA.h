/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: FXAA - crude but super cheap anti-aliasing
*****************************************************************************/
#ifndef _USG_POSTFX_FXAA_H_
#define _USG_POSTFX_FXAA_H_

#include "Engine/Graphics/Effects/Effect.h"
#include "Engine/Graphics/Materials/Material.h"
#include "Engine/PostFX/PostEffect.h"

namespace usg {

class PostFXSys;

class FXAA : public PostEffect
{
public:
	FXAA();
	~FXAA();

	virtual void Init(GFXDevice* pDevice, ResourceMgr* pResource, PostFXSys* pSys, RenderTarget* pDst);
	virtual void Cleanup(GFXDevice* pDevice);
	virtual void SetDestTarget(GFXDevice* pDevice, RenderTarget* pDst);
	virtual void Resize(GFXDevice* pDevice, uint32 uWidth, uint32 uHeight);
	virtual bool Draw(GFXContext* pContext, RenderContext& renderContext);

	virtual bool ReadsTexture(Input eInput) const override;
	virtual bool LoadsTexture(Input eInput) const override;
	virtual void SetTexture(GFXDevice* pDevice, Input eInput, const TextureHndl& texture) override;

private:
	PostFXSys*		m_pSys;

	RenderTarget*			m_pDestTarget;
	usg::SamplerHndl		m_sampler;
	ConstantSet				m_constantSet;
	Material				m_material;
};

}

#endif
