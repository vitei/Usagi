#include "Engine/Common/Common.h"
#include "Engine/GUI/IMGuiRenderer.h"
#include "Engine/Maths/AABB.h"
#include "Engine/Scene/ViewContext.h"
#include "Engine/Resource/ResourceMgr.h"
#include "Engine/Graphics/Lights/DirLight.h"
#include "Engine/Maths/MathUtil.h"
#include "PreviewWindow.h"
 

PreviewWindow::PreviewWindow()
	: m_bPaused(false)
{
	m_pDirLight = nullptr;
}

PreviewWindow::~PreviewWindow()
{
	m_pDirLight = nullptr;
}

void PreviewWindow::Init(usg::GFXDevice* pDevice, usg::IMGuiRenderer* pRenderer, const char* szName)
{
	usg::Vector2f vPos(320.0f, 0.0f);
	usg::Vector2f vSize(420.f, 500.f);
	m_window.Init(szName, vPos, vSize, usg::GUIWindow::WINDOW_TYPE_PARENT);
	// TODO: Need to resize with the preview window
	vSize.Assign(400.f, 400.f);
	uint32 uEffectFlags = usg::PostFXSys::EFFECT_DEFERRED_SHADING | usg::PostFXSys::EFFECT_SKY_FOG | usg::PostFXSys::EFFECT_SMAA | usg::PostFXSys::EFFECT_BLOOM;
	m_postFX.Init(pDevice, usg::ResourceMgr::Inst(), (uint32)vSize.x, (uint32)vSize.y, uEffectFlags);
	m_postFX.SetSkyTexture(pDevice, usg::ResourceMgr::Inst()->GetTexture(pDevice, "purplenebula"));
	m_texture.Init(pDevice, "Preview", vSize, m_postFX.GetFinalRT()->GetColorTexture());

	usg::AABB worldBounds;
	worldBounds.SetCentreRadii(usg::Vector3f(0.0f, 0.0f, 0.0f), usg::Vector3f(512.0f, 512.0f, 512.0f));

	m_scene.Init(pDevice, worldBounds, NULL);
	m_pSceneCtxt = m_scene.CreateViewContext(pDevice);
	m_camera.Init(vSize.x / vSize.y);
	m_pSceneCtxt->Init(pDevice, &m_postFX, 0, usg::RenderMask::RENDER_MASK_ALL);
	m_pSceneCtxt->SetCamera(&m_camera.GetCamera());

	m_previewButtons[BUTTON_PLAY].InitAsTexture(pDevice, "Play", usg::ResourceMgr::Inst()->GetTexture(pDevice, "play"));
	m_previewButtons[BUTTON_PAUSE].InitAsTexture(pDevice, "Play", usg::ResourceMgr::Inst()->GetTexture(pDevice, "pause"));
	m_previewButtons[BUTTON_RESTART].InitAsTexture(pDevice, "Play", usg::ResourceMgr::Inst()->GetTexture(pDevice, "backtostart"));
	for (uint32 i = 0; i < BUTTON_COUNT; i++)
	{
		m_previewButtons[i].SetSameLine(true);
		m_window.AddItem(&m_previewButtons[i]);
	}

	m_repeat.Init("Repeat", true);
	m_repeat.SetSameLine(true);
	m_window.AddItem(&m_repeat);
	m_clearColor.Init("Background");
	usg::Color defaultCol(0.1f, 0.1f, 0.1f);
	m_clearColor.SetValue(defaultCol);
	m_window.AddItem(&m_clearColor);

	m_previewModel.Init(pDevice, &m_scene);
	m_previewModel.AddToWindow(&m_window);

	m_window.AddItem(&m_texture);

	bool bRestart = m_previewButtons[BUTTON_RESTART].GetValue();

	if (m_previewButtons[BUTTON_PAUSE].GetValue())
		m_bPaused = true;

	if (m_previewButtons[BUTTON_PLAY].GetValue())
		m_bPaused = false;


}

void PreviewWindow::Draw(usg::GFXContext* pImmContext)
{
	m_postFX.BeginScene(pImmContext, 0);

	m_postFX.SetActiveViewContext(m_pSceneCtxt);
	m_pSceneCtxt->PreDraw(pImmContext, usg::VIEW_CENTRAL);
	m_pSceneCtxt->DrawScene(pImmContext);
	m_postFX.SetActiveViewContext(nullptr);

	m_postFX.EndScene();
}

void PreviewWindow::CleanUp(usg::GFXDevice* pDevice)
{
	m_previewButtons[BUTTON_PLAY].CleanUp(pDevice);
	m_previewButtons[BUTTON_PAUSE].CleanUp(pDevice);
	m_previewButtons[BUTTON_RESTART].CleanUp(pDevice);
	m_previewModel.CleanUp(pDevice); 
	m_scene.DeleteViewContext(m_pSceneCtxt);
	m_postFX.CleanUp(pDevice);
	m_texture.CleanUp(pDevice);
	m_scene.Cleanup(pDevice);

}

bool PreviewWindow::Update(usg::GFXDevice* pDevice, float fElapsed)
{
	m_camera.Update(fElapsed);
	m_previewModel.Update(pDevice, fElapsed);
	m_scene.TransformUpdate(fElapsed);
	m_scene.Update(pDevice);

	return true;
}