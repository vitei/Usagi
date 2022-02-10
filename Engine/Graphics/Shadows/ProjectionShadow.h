/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: A depth texture for adding shadows to spotlights
*****************************************************************************/
#ifndef _USG_POSTFX_SHADOWS_SPOTSHADOW_H_
#define _USG_POSTFX_SHADOWS_SPOTSHADOW_H_

#include "Engine/Graphics/Textures/DepthStencilBuffer.h"
#include "Engine/Graphics/Textures/RenderTarget.h"
#include "Engine/Graphics/Effects/ConstantSet.h"
#include "Engine/Scene/Camera/StandardCamera.h"

namespace usg
{

class GFXContext;
class Light;
class Scene;
class RenderNode;
class ShadowContext;

class ProjectionShadow
{
public:
	ProjectionShadow();
	virtual ~ProjectionShadow();

	void Init(GFXDevice* pDevice, Scene* pScene, uint32 uResX, uint32 uResY);
	void Cleanup(GFXDevice* pDevice, Scene* pScene);

	void GPUUpdate(GFXDevice* pDevice, Light* pLight);
	void SetStatic(bool bStatic) { m_bStatic = bStatic; }
	void EnableUpdate(bool bEnable);
	
	void CreateShadowTex(GFXContext* pContext);
	void PostDraw();

	const ConstantSet* GetShadowConstants() { return &m_readConstants; }
	const TextureHndl GetShadowTexture() { return m_depthBuffer.GetTexture(); }

private:

	ConstantSet				m_readConstants;

	StandardCamera			m_camera;
	ShadowContext*			m_pSceneContext;

	DepthStencilBuffer		m_depthBuffer;
	RenderTarget			m_depthTarget;
	bool					m_bStatic;
	bool					m_bEnableUpdate;
	bool					m_bNothingVisible;
};

}


#endif
