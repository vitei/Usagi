#pragma once

#include "Engine/Graphics/Device/GFXDevice.h"
#include "Engine/Graphics/Device/StateEnums.pb.h"
#include "Engine/GUI/GuiWindow.h"
#include "Engine/GUI/GuiItems.h"
#include "Engine/PostFX/PostFXSys.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Graphics/Lights/DirLight.h"
#include "EmitterModifier.h"
#include "PreviewModel.h"
#include "FloatAnim.h"
#include "MayaCamera.h"

namespace usg
{
	class IMGuiRenderer;
	class ViewContext;
}

class PreviewWindow
{
public:
	PreviewWindow();
	~PreviewWindow();

	void Init(usg::GFXDevice* pDevice, usg::IMGuiRenderer* pRenderer, const char* szName);
	void CleanUp(usg::GFXDevice* pDevice);
	bool Update(usg::GFXDevice* pDevice, float fElapsed);
	void Draw(usg::GFXContext* pImmContext);
	usg::Scene& GetScene() { return m_scene; }
	usg::GUIWindow& GetGUIWindow() { return m_window; }

	bool GetPaused() { return m_bPaused; }
	bool GetRepeat() { return m_repeat.GetValue(); }
	bool GetRestart() { return m_previewButtons[BUTTON_RESTART].GetValue(); }
	void SetBackgroundColor(const usg::Color& color) { m_clearColor.SetValue(color); }
	const usg::Color& GetBackgroundColor() { return m_clearColor.GetValue(); }

private:
	enum
	{
		BUTTON_PLAY = 0,
		BUTTON_PAUSE,
		BUTTON_RESTART,
		BUTTON_COUNT,
	};
	usg::DirLight*		m_pDirLight;
	MayaCamera			m_camera;
	PreviewModel		m_previewModel;
	usg::Scene			m_scene;
	usg::GUIWindow		m_window;
	usg::GUITexture		m_texture;
	usg::ViewContext*	m_pSceneCtxt;

	usg::GUICheckBox	m_repeat;
	usg::GUIColorSelect	m_clearColor;

	// TODO: These might want to be moved out or set to optional (make no sense for material editing)
	usg::GUIButton		m_previewButtons[BUTTON_COUNT];
	bool				m_bPaused;

	usg::PostFXSys		m_postFX;
};
