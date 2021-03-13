
#include "Engine/Graphics/RenderConsts.h"
#include "Engine/Scene/Camera/StandardCamera.h"
#include "Engine/Scene/Camera/HMDCamera.h"

namespace usg
{
	class Scene;
	class PostFXSys;
	class ViewContext;
	class DebugRender;
	class Display;
	enum ViewType;

	class GameView
	{
	public:
		GameView(usg::GFXDevice* pDevice, usg::Scene& scene, ResourceMgr* pResMgr, usg::PostFXSys& postFXSys, const usg::GFXBounds& bounds);
		~GameView();

		void Cleanup(usg::GFXDevice* pDevice, usg::Scene& scene);
		void AddDebugRender(usg::DebugRender* pDebugRender);

		void Draw(usg::PostFXSys* pPostFXSys, usg::Display* pDisplay, usg::GFXContext* pImmContext, usg::GFXContext* pDeferredContext, usg::ViewType eType);

		const usg::GFXBounds& GetBounds() const;
		void SetBounds(usg::GFXBounds& bounds);
		usg::ViewContext* GetViewContext() { return m_pViewContext; }

	private:
		void SetViewContext(usg::ViewContext& context);

		usg::ViewContext* m_pViewContext = nullptr;
		usg::GFXBounds m_bounds;
		usg::DebugRender* m_pDebugRender = nullptr;

	};

}