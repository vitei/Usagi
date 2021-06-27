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

	enum class Input
	{
		Color,
		LinearDepth,
		Depth,
		Albedo,
		Normal,
		Emissive,
		Specular
	};


	PostEffect();
	virtual ~PostEffect();

	virtual void Init(GFXDevice* pDevice, ResourceMgr* pResource, PostFXSys* pSys, RenderTarget* pDst) {}
	virtual void Cleanup(GFXDevice* pDevice) {}
	virtual void Resize(GFXDevice* pDevice, uint32 uWidth, uint32 uHeight) {}
	
	// Needed to figure out the correct render targets to pass in
	virtual bool ReadsTexture(Input eInput) const { return false; }
	virtual bool LoadsTexture(Input eInput) const { return false; }
	virtual void SetTexture(GFXDevice* pDevice, Input eInput, const TextureHndl& texture) {}
	virtual bool RequiresHDR() const { return false; }
	virtual void SetDestTarget(GFXDevice* pDevice, RenderTarget* pDst) {}

	virtual void Update(Scene* pScene, float fElapsed) {}
	virtual void UpdateBuffer(usg::GFXDevice* pDevice) {}
	void SetEnabled(bool bEnabled);
	bool GetEnabled() { return m_bEnabled; }

private:
	bool	m_bEnabled;
	
};

}

#endif
