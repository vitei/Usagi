/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Resolve the depth buffer into a linear format which is faster
//	to read in post process effects. Will suffer from loss of precision
*****************************************************************************/
#ifndef _USG_POSTFX_LINEARDEPTH_H_
#define _USG_POSTFX_LINEARDEPTH_H_

#include "Engine/Graphics/Effects/Effect.h"
#include "Engine/Graphics/Materials/Material.h"
#include "Engine/PostFX/PostEffect.h"

namespace usg {

class PostFXSys;

class LinearDepth : public PostEffect
{
public:
	LinearDepth();
	~LinearDepth();

	virtual void Init(GFXDevice* pDevice, ResourceMgr* pResource, PostFXSys* pSys, RenderTarget* pResult);
	virtual void Cleanup(GFXDevice* pDevice);
	virtual bool Draw(GFXContext* pContext, RenderContext& renderContext);
	const TextureHndl& GetTexture() { return m_depthRT.GetColorTexture(); }

	virtual bool ReadsTexture(Input eInput) const override { return true; }
	virtual bool LoadsTexture(Input eInput) const override { return false;}
	virtual void SetTexture(GFXDevice* pDevice, Input eInput, const TextureHndl& texture) override {}

private:
	PostFXSys*		m_pSys;

	ColorBuffer				m_depthBuffer;
	RenderTarget			m_depthRT;
	Material				m_material;
};

}

#endif
