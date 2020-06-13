/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Adaptive Screen Space Ambient Occlusion
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
	PostFXSys*		m_pSys;

	ConstantSet		m_constants;

	SamplerHndl		m_pointSampler;
	SamplerHndl		m_linearSampler;
};

}

#endif
