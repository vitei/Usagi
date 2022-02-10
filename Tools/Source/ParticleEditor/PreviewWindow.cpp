#include "Engine/Common/Common.h"
#include "Engine/GUI/IMGuiRenderer.h"
#include "Engine/Maths/AABB.h"
#include "Engine/Scene/ViewContext.h"
#include "Engine/Resource/ResourceMgr.h"
#include "Engine/Graphics/Device/GFXContext.h"
#include "Engine/Graphics/StandardVertDecl.h"
#include "Engine/Graphics/Lights/DirLight.h"
#include "Engine/Graphics/Lights/LightMgr.h"
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

void PreviewWindow::Init(usg::GFXDevice* pDevice, usg::IMGuiRenderer* pRenderer, const char* szName, const usg::Vector2f& vPos, uint32 uInitFlags)
{
	usg::PipelineStateDecl pipelineDecl;
	pipelineDecl.inputBindings[0].Init(usg::GetVertexDeclaration(usg::VT_POSITION));
	pipelineDecl.uInputBindingCount = 1;
	pipelineDecl.ePrimType = usg::PT_TRIANGLES;
	pipelineDecl.pEffect = usg::ResourceMgr::Inst()->GetEffect(pDevice, "PostProcess.ClearAlpha");

	pipelineDecl.layout.uDescriptorSetCount = 0;
	pipelineDecl.rasterizerState.eCullFace = usg::CULL_FACE_NONE;

	pipelineDecl.alphaState.SetColor0Only();
	pipelineDecl.alphaState.bBlendEnable = true;
	pipelineDecl.alphaState.uColorMask[0] = usg::RT_MASK_ALPHA;
	pipelineDecl.alphaState.srcBlendAlpha = usg::BLEND_FUNC_ONE;
	pipelineDecl.alphaState.dstBlendAlpha = usg::BLEND_FUNC_ZERO;
	pipelineDecl.alphaState.blendEqAlpha = usg::BLEND_EQUATION_ADD;

	usg::Vector2f vSize(420.f, 600.f);
	m_window.Init(szName, vPos, vSize, usg::GUIWindow::WINDOW_TYPE_PARENT);
	// TODO: Need to resize with the preview window
	vSize.Assign(400.f, 400.f);
	uint32 uEffectFlags = usg::PostFXSys::EFFECT_DEFERRED_SHADING | usg::PostFXSys::EFFECT_SKY_FOG | usg::PostFXSys::EFFECT_SMAA | usg::PostFXSys::EFFECT_BLOOM;
	m_postFX.Init(pDevice, usg::ResourceMgr::Inst(), (uint32)vSize.x, (uint32)vSize.y, uEffectFlags);
	m_postFX.SetSkyTexture(pDevice, usg::ResourceMgr::Inst()->GetTexture(pDevice, "purplenebula"));
	m_texture.Init(pDevice, "Preview", vSize, m_postFX.GetFinalRT()->GetColorTexture());

	usg::AABB worldBounds;
	worldBounds.SetCentreRadii(usg::Vector3f(0.0f, 0.0f, 0.0f), usg::Vector3f(512.0f, 512.0f, 512.0f));

	m_scene.Init(pDevice, usg::ResourceMgr::Inst(), worldBounds, NULL);
	m_pSceneCtxt = m_scene.CreateViewContext(pDevice);
	m_camera.Init(vSize.x / vSize.y);
	m_pSceneCtxt->Init(pDevice, usg::ResourceMgr::Inst(), &m_postFX, 0, usg::RenderMask::RENDER_MASK_ALL);
	m_pSceneCtxt->SetCamera(&m_camera.GetCamera());

	m_previewButtons[BUTTON_PLAY].InitAsTexture(pDevice, "Play", usg::ResourceMgr::Inst()->GetTexture(pDevice, "play"));
	m_previewButtons[BUTTON_PAUSE].InitAsTexture(pDevice, "Pause", usg::ResourceMgr::Inst()->GetTexture(pDevice, "pause"));
	m_previewButtons[BUTTON_RESTART].InitAsTexture(pDevice, "Restart", usg::ResourceMgr::Inst()->GetTexture(pDevice, "backtostart"));
	m_speedOverride.Init("Set Speed", false);
	m_playbackSpeed.Init("Speed", 0.05f, 2.0f, 1.0f);
	m_playbackSpeed.SetSameLine(true);

	if (uInitFlags | SHOW_PLAY_CONTROLS)
	{
		for (uint32 i = 0; i < BUTTON_COUNT; i++)
		{
			m_previewButtons[i].SetSameLine(true);
			m_window.AddItem(&m_previewButtons[i]);
		}
		m_repeat.Init("Repeat", true);
		m_repeat.SetSameLine(true);
		m_window.AddItem(&m_repeat);

		m_window.AddItem(&m_speedOverride);
		m_window.AddItem(&m_playbackSpeed);
	}

	m_clearColor.Init("Background");
	usg::Color defaultCol(0.1f, 0.1f, 0.1f);
	m_clearColor.SetValue(defaultCol);
	m_window.AddItem(&m_clearColor);

	m_previewModel.Init(pDevice, &m_scene);
	if (uInitFlags | SHOW_PREVIEW_MODEL)
	{
		m_previewModel.AddToWindow(&m_window);
	}

	m_window.AddItem(&m_texture);

	m_pDirLight = m_scene.GetLightMgr().AddDirectionalLight(pDevice, false);
	m_pDirLight->SetAmbient(usg::Color(0.3f, 0.3f, 0.3f));
	m_pDirLight->SetDiffuse(usg::Color(2.0f, 2.0f, 2.0f));
	m_pDirLight->SetSpecularColor(usg::Color(5.0f, 5.0f, 5.0f));
	m_pDirLight->SetDirection(usg::Vector4f(-1.0f, -1.0f, 0.0f, 0.0f).GetNormalised());
	m_pDirLight->SwitchOn(true);

	m_clearAlphaPipeline = pDevice->GetPipelineState(m_postFX.GetRenderPasses().GetRenderPass(usg::LAYER_OVERLAY, 128), pipelineDecl);

}

void PreviewWindow::Draw(usg::GFXContext* pImmContext)
{
	m_scene.PreDraw(pImmContext);
	m_scene.GetLightMgr().ViewShadowRender(pImmContext, &m_scene, m_pSceneCtxt);

	m_postFX.BeginScene(pImmContext, 0);

	m_postFX.SetActiveViewContext(m_pSceneCtxt);
	m_pSceneCtxt->PreDraw(pImmContext, usg::VIEW_CENTRAL);
	m_pSceneCtxt->DrawScene(pImmContext);
	pImmContext->SetPipelineState(m_clearAlphaPipeline);
	m_postFX.DrawFullScreenQuad(pImmContext);
	m_postFX.SetActiveViewContext(nullptr);

	m_postFX.EndScene();
}

void PreviewWindow::Cleanup(usg::GFXDevice* pDevice)
{
	m_scene.GetLightMgr().RemoveDirLight(m_pDirLight);
	m_pDirLight = nullptr;
	m_previewButtons[BUTTON_PLAY].Cleanup(pDevice);
	m_previewButtons[BUTTON_PAUSE].Cleanup(pDevice);
	m_previewButtons[BUTTON_RESTART].Cleanup(pDevice);
	m_previewModel.Cleanup(pDevice); 
	m_scene.DeleteViewContext(m_pSceneCtxt);
	m_postFX.Cleanup(pDevice);
	m_texture.Cleanup(pDevice);
	m_scene.Cleanup(pDevice);

}

bool PreviewWindow::Update(usg::GFXDevice* pDevice, float fElapsed)
{
	m_camera.Update(fElapsed, m_texture.IsHovered());
	m_previewModel.Update(pDevice, fElapsed);
	m_scene.TransformUpdate(fElapsed);
	m_scene.Update(pDevice);

	m_previewButtons[BUTTON_PAUSE].SetVisible(!m_bPaused);
	m_previewButtons[BUTTON_PLAY].SetVisible(m_bPaused);

	m_playbackSpeed.SetVisible(m_speedOverride.GetValue());

	if (m_previewButtons[BUTTON_PAUSE].GetValue())
		m_bPaused = true;

	if (m_previewButtons[BUTTON_PLAY].GetValue())
		m_bPaused = false;

	return true;
}

float PreviewWindow::GetPlaySpeed() const
{
	if (!m_speedOverride.GetValue())
		return 1.0f;

	return m_playbackSpeed.GetValue();
}
