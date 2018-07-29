/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Shadow system for directional lights
//	Currently a cascade shadow on all platforms
*****************************************************************************/
#ifndef _USG_POSTFX_SHADOWS_DIRSHADOW_H_
#define _USG_POSTFX_SHADOWS_DIRSHADOW_H_
#include "Engine/Common/Common.h"

namespace usg
{
class GFXContext;
class DirLight;
class Camera;

class DirectionalShadow
{
public:
	DirectionalShadow() {}
	~DirectionalShadow() {}

	virtual void Init(GFXDevice* pDevice, Scene* pScene, const DirLight* pLight, uint32 uTexWidth, uint32 uTexHeight) = 0;
	virtual void Update(const Camera &sceneCam) = 0;
	virtual void CreateShadowTex(GFXContext* pContext) = 0;
	virtual void Finished(GFXContext* pContext) = 0;

private:

};

}


#endif
