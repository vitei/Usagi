#include "Engine/Common/Common.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/ViewContext.h"
#include "Engine/Debug/Rendering/DebugRender.h"
#include "Engine/PostFX/PostFXSys.h"
#include "Engine/Graphics/Fog.h"
#include "Engine/Graphics/Device/GFXDevice.h"
#include "Engine/Graphics/Device/GFXContext.h"
#include "GameView.h"

namespace usg
{

	GameView::GameView(usg::GFXDevice* pDevice, usg::Scene& scene, ResourceMgr* pResMgr, usg::PostFXSys& postFXSys, const usg::GFXBounds& bounds) :
		m_bounds(bounds)
	{
		usg::ViewContext* pContext = scene.CreateViewContext(pDevice);
		pContext->Init(pDevice, pResMgr, &postFXSys);

		SetViewContext(*pContext);
	}

	void GameView::Cleanup(usg::GFXDevice* pDevice, usg::Scene& scene)
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