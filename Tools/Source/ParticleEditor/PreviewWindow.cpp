#include "Engine/Common/Common.h"
#include "Engine/GUI/IMGuiRenderer.h"
#include "Engine/Resource/ResourceMgr.h"
#include "Engine/Maths/MathUtil.h"
#include "PreviewWindow.h"
 

void PreviewWindow::Init(usg::GFXDevice* pDevice, usg::IMGuiRenderer* pRenderer, const char* szName)
{
	usg::Vector2f vPos(320.0f, 0.0f);
	usg::Vector2f vSize(400.f, 300.f);
	m_window.Init(szName, vPos, vSize, usg::GUIWindow::WINDOW_TYPE_PARENT);
	// TODO: Need to resize with the preview window
	vSize.Assign(320.f, 240.f);
	uint32 uEffectFlags = usg::PostFXSys::EFFECT_DEFERRED_SHADING | usg::PostFXSys::EFFECT_SKY_FOG | usg::PostFXSys::EFFECT_SMAA | usg::PostFXSys::EFFECT_BLOOM;
	m_postFX.Init(pDevice, usg::ResourceMgr::Inst(), 320, 240, uEffectFlags);
	m_texture.Init(pDevice, "Preview", vSize, m_postFX.GetFinalRT()->GetColorTexture());

	m_window.AddItem(&m_texture);
}

void PreviewWindow::CleanUp(usg::GFXDevice* pDevice)
{
	m_postFX.CleanUp(pDevice);
}

bool PreviewWindow::Update(usg::GFXDevice* pDevice)
{
	return true;
}