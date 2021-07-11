/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: For forcefully changing the render target (for the benefit
//	of models/ particle effects) when the last post process may not have left
//	us in our preferred state
*****************************************************************************/
#ifndef _USG_POSTFX_SET_SCENE_TARGET_H_
#define _USG_POSTFX_SET_SCENE_TARGET_H_

#include "Engine/Graphics/Effects/Effect.h"
#include "Engine/Graphics/Materials/Material.h"
#include "Engine/PostFX/PostEffect.h"

namespace usg {

class PostFXSys;

class SetSceneTarget : public PostEffect
{
public:
	SetSceneTarget();
	~SetSceneTarget();

	virtual void Init(GFXDevice* pDevice, ResourceMgr* pResource, PostFXSys* pSys);
	virtual void Cleanup(GFXDevice* pDevice);
	virtual void SetDestTarget(GFXDevice* pDevice, RenderTarget* pDst);
	virtual void Resize(GFXDevice* pDevice, uint32 uWidth, uint32 uHeight);
	virtual bool Draw(GFXContext* pContext, RenderContext& renderContext);

	virtual bool ReadsTexture(Input eInput) const override;
	virtual bool LoadsTexture(Input eInput) const override;
	virtual void SetTexture(GFXDevice* pDevice, Input eInput, const TextureHndl& texture) override;

private:
	PostFXSys*				m_pSys;

	RenderTarget*			m_pDestTarget;

};

}

#endif
