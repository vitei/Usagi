/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: An interface class for full screen effects
//  Note that only color buffers can be daisy chained, other buffers must
//	be written to OR read by an effect, not both. If you need to do so the
//	effect must internally do a copy
//  If it is possible that there is normal scene rendering (e.g. a model) after
//  this effect it is currently the effects responsibility to set the
//	appropriate destination render target
*****************************************************************************/
#ifndef _USG_POSTFX_POSTEFFECT_H_
#define _USG_POSTFX_POSTEFFECT_H_

#include "Engine/Scene/RenderNode.h"
#include "Engine/Core/Callback.h"

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
		Specular,
		Count
	};


	PostEffect();
	virtual ~PostEffect();

	virtual void Init(GFXDevice* pDevice, ResourceMgr* pResource, PostFXSys* pSys, RenderTarget* pDst) {}
	virtual void Cleanup(GFXDevice* pDevice) {}
	virtual void Resize(GFXDevice* pDevice, uint32 uWidth, uint32 uHeight) {}
	
	// Needed to figure out the correct render targets to pass in
	virtual bool ReadsTexture(Input eInput) const { return false; }
	virtual bool LoadsTexture(Input eInput) const { return false; }
	// Most post effects will have a final target of color only
	virtual bool WritesTexture(Input eInput) const { return (eInput == Input::Color || eInput == Input::Depth); }
	virtual bool RequiresHDR() const { return false; }

	virtual void SetTexture(GFXDevice* pDevice, Input eInput, const TextureHndl& texture) {}
	virtual void PassDataSet(GFXDevice* pDevice) {}	// Called when dest and sources have ben set
	virtual void SetDestTarget(GFXDevice* pDevice, RenderTarget* pDst) {}
	// FIXME: Remove and replace with SetTexture
	virtual void SetSourceTarget(GFXDevice* pDevice, RenderTarget* pTarget) {}

	virtual void Update(Scene* pScene, float fElapsed) {}
	virtual void UpdateBuffer(usg::GFXDevice* pDevice) {}
	void SetEnabled(bool bEnabled);
	bool GetEnabled() { return m_bEnabled; }

private:
	bool	m_bEnabled;
	
};

}

#endif
