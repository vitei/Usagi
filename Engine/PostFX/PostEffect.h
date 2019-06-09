/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: An interface class for full screen effects
*****************************************************************************/
#ifndef _USG_POSTFX_POSTEFFECT_H_
#define _USG_POSTFX_POSTEFFECT_H_

#include "Engine/Scene/RenderNode.h"

namespace usg {

class PostFXSys;
class GFXDevice;
class RenderTarget;
class ResourceMgr;

class PostEffect : public RenderNode
{
public:
	PostEffect();
	virtual ~PostEffect();

	virtual void Init(GFXDevice* pDevice, ResourceMgr* pResource, PostFXSys* pSys, RenderTarget* pDst) {}
	virtual void CleanUp(GFXDevice* pDevice) {}
	virtual void Resize(GFXDevice* pDevice, uint32 uWidth, uint32 uHeight) {}
	virtual void SetDestTarget(GFXDevice* pDevice, RenderTarget* pDst) {}
	void SetEnabled(bool bEnabled);
	bool GetEnabled() { return m_bEnabled; }

private:
	bool	m_bEnabled;
	
};

}

#endif
