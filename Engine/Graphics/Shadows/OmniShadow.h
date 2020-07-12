/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: A depth cubemap for adding shadows to point lights
*****************************************************************************/
#ifndef _USG_POSTFX_SHADOWS_OMNISHADOW_H_
#define _USG_POSTFX_SHADOWS_OMNISHADOW_H_

#include "Engine/Graphics/Textures/DepthStencilBuffer.h"
#include "Engine/Graphics/Textures/RenderTarget.h"
#include "Engine/Graphics/Device/DescriptorSet.h"
#include "Engine/Graphics/Effects/ConstantSet.h"
#include "Engine/Core/Containers/List.h"

namespace usg
{

class GFXContext;
class PointLight;
class Scene;
class RenderNode;
class OmniShadowContext;

class OmniShadow
{
public:
	OmniShadow();
	virtual ~OmniShadow();

	void Init(GFXDevice* pDevice, Scene* pScene, uint32 uResX, uint32 uResY);
	void Cleanup(GFXDevice* pDevice, Scene* pScene);

	void GPUUpdate(GFXDevice* pDevice, PointLight* pLight);
	void SetStatic(bool bStatic) { m_bStatic = bStatic; }
	void EnableUpdate(bool bEnable);
	
	void CreateShadowTex(GFXContext* pContext);
	void PostDraw();
	void GetTexDim(Vector2f& vDimOut);

	TextureHndl GetShadowTexture();

	enum
	{
		DRAW_SIDES = 6
	};

private:

	OmniShadowContext*		m_pShadowContext;
	Sphere					m_sphere;

	DepthStencilBuffer		m_cubeBuffer;
	RenderTarget			m_cubeTarget;
	bool					m_bStatic;
	bool					m_bEnableUpdate;
	bool					m_bNothingVisible;
};

}


#endif
