#pragma once

#include "Engine/Graphics/Device/GFXDevice.h"
#include "Engine/Graphics/Device/StateEnums.pb.h"
#include "Engine/GUI/GuiWindow.h"
#include "Engine/GUI/GuiItems.h"
#include "Engine/PostFX/PostFXSys.h"
#include "EmitterModifier.h"
#include "FloatAnim.h"
#include "FileList.h"

namespace usg
{
	class IMGuiRenderer;
}

class PreviewWindow
{
public:
	PreviewWindow() {}
	~PreviewWindow() {}

	void Init(usg::GFXDevice* pDevice, usg::IMGuiRenderer* pRenderer, const char* szName);
	void CleanUp(usg::GFXDevice* pDevice);
	bool Update(usg::GFXDevice* pDevice);
	usg::GUIWindow& GetGUIWindow() { return m_window; }

private:
	usg::GUIWindow	m_window;
	usg::GUITexture	m_texture;

	usg::PostFXSys	m_postFX;
};
