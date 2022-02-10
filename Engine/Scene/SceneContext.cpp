/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Graphics/GFX.h"
#include "Engine/Graphics/Device/GFXDevice.h"
#include "Engine/Scene/Camera/Camera.h"
#include "Engine/Scene/RenderNode.h"
#include "Engine/Scene/Scene.h"
#include "Engine/PostFX/PostEffect.h"
#include "Engine/Scene/SceneContext.h"


namespace usg {

SceneContext::SceneContext():
m_visibleGroups(256)
{
	m_uRenderMask		= RenderMask::RENDER_MASK_ALL;
	m_pScene			= nullptr;
	m_bActive			= true;
	m_bDeviceDataValid = false;
}

SceneContext::~SceneContext()
{

}


void SceneContext::SetScene(Scene* pScene)
{
	m_pScene = pScene;
}


void SceneContext::SetRenderMask(uint32 uRenderMask)
{
	m_uRenderMask		= uRenderMask;
	GetSearchObject().SetMask(uRenderMask);
}

void SceneContext::ClearLists()
{
	m_visibleGroups.clear();
	m_uVisiblePVSCount = 0;
}

}

