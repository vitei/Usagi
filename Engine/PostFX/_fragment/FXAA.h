/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: FXAA - crude but super cheap anti-aliasing
*****************************************************************************/
#ifndef _USG_POSTFX_FXAA_H_
#define _USG_POSTFX_FXAA_H_
#include "Engine/Common/Common.h"
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

	virtual void Init(GFXDevice* pDevice, PostFXSys* pSys, RenderTarget* pDst);
	virtual void CleanUp(GFXDevice* pDevice);
	virtual void SetDestTarget(GFXDevice* pDevice, RenderTarget* pDst);
	virtual void Resize(GFXDevice* pDevice, uint32 uWidth, uint32 uHeight);
	void SetSourceTarget(GFXDevice* pDevice, RenderTarget* pTarget);
	virtual bool Draw(GFXContext* pContext, RenderContext& renderContext);

private:
	PostFXSys*		m_pSys;

	RenderTarget*			m_pDestTarget;
	usg::SamplerHndl		m_sampler;
	ConstantSet				m_constantSet;
	Material				m_material;
};

}

#endif
