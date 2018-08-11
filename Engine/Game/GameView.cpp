#include "Engine/Common/Common.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/ViewContext.h"
#include "Engine/Debug/Rendering/DebugRender.h"
#include "Engine/PostFX/PostFXSys.h"
#include "Engine/Graphics/Device/GFXDevice.h"
#include "Engine/Graphics/Device/GFXContext.h"
#include "GameView.h"

namespace usg
{

	GameView::GameView(usg::GFXDevice* pDevice, usg::Scene& scene, usg::PostFXSys& postFXSys, const usg::GFXBounds& bounds, float32 fFov, float32 fNear, float32 fFar) :
		m_bounds(bounds)
	{
		usg::StandardCamera& cam = GetCamera();
		usg::Matrix4x4 m = usg::Matrix4x4::Identity();
		float fAspect = (float)bounds.width / (float)bounds.height;
		cam.SetUp(m, fAspect, fFov, fNear, fFar);

		usg::ViewContext* pContext = scene.CreateViewContext(pDevice);
		pContext->Init(pDevice, &cam, &postFXSys);

		SetViewContext(*pContext);

		if (pDevice->GetHMD())
		{
			m_hmdCamera.Init(pDevice->GetHMD(), fNear, fFar);
			pContext->SetCamera(&m_hmdCamera);
		}
	}

	void GameView::CleanUp(usg::GFXDevice* pDevice, usg::Scene& scene)
	{
		if (m_pViewContext)
		{
			scene.DeleteViewContext(m_pViewContext);
			m_pViewContext = nullptr;
		}
	}

	GameView::~GameView()
	{

	}

	void GameView::SetViewContext(usg::ViewContext& context)
	{
		usg::Color cFog(0.14f, 0.05f, 0.05f, 1.0f);
		usg::Fog& fog = context.GetFog();
		fog.SetActive(true);
		fog.SetType(usg::FOG_TYPE_LINEAR);
		fog.SetMinDepth(m_camera.GetFar() * 0.8f);
		fog.SetMaxDepth(m_camera.GetFar());
		m_pViewContext = &context;
	}

	const usg::GFXBounds& GameView::GetBounds() const
	{
		return m_bounds;
	}

	void GameView::SetBounds(usg::GFXBounds& bounds)
	{
		memcpy(&m_bounds, &bounds, sizeof(bounds));
	}

	void GameView::Draw(usg::PostFXSys* pPostFXSys, usg::Display* pDisplay, usg::GFXContext* pImmContext, usg::GFXContext* pDeferredContext, usg::ViewType eType)
	{
		usg::ViewContext* pSceneCtxt = m_pViewContext;

		pPostFXSys->SetActiveViewContext(pSceneCtxt);
		pSceneCtxt->PreDraw(pImmContext, eType);
		pSceneCtxt->DrawScene(pImmContext);
		pPostFXSys->SetActiveViewContext(nullptr);
	}

	void GameView::AddDebugRender(usg::DebugRender* pDebugRender)
	{
		m_pDebugRender = pDebugRender;
	}

}