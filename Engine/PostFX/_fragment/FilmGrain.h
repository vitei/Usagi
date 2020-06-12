/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Film Grain
*****************************************************************************/
#ifndef _USG_POSTFX_FILM_GRAIN_H_
#define _USG_POSTFX_FILM_GRAIN_H_

#include "Engine/Graphics/Effects/Effect.h"
#include "Engine/Graphics/Materials/Material.h"
#include "Engine/PostFX/PostEffect.h"

namespace usg {

class PostFXSys;

class FilmGrain : public PostEffect
{
public:
	FilmGrain();
	~FilmGrain();

	virtual void Init(GFXDevice* pDevice, ResourceMgr* pResource, PostFXSys* pSys, RenderTarget* pDst);
	virtual void CleanUp(GFXDevice* pDevice);
	virtual void SetDestTarget(GFXDevice* pDevice, RenderTarget* pDst);
	virtual void Resize(GFXDevice* pDevice, uint32 uWidth, uint32 uHeight);
	void SetSourceTarget(GFXDevice* pDevice, RenderTarget* pTarget);
	virtual bool Draw(GFXContext* pContext, RenderContext& renderContext);
	void Update(float fElapsed);
	void UpdateBuffer(usg::GFXDevice* pDevice);

private:
	PostFXSys*		m_pSys;

	RenderTarget*			m_pDestTarget;
	usg::SamplerHndl		m_sampler;
	ConstantSet				m_constantSet;
	Material				m_material;
};

}

#endif
