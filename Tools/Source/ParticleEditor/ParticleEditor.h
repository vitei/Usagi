#ifndef _USG_TESTBED_PARTICLE_EDITOR_H_
#define _USG_TESTBED_PARTICLE_EDITOR_H_
#include "Engine/Game/GameInterface.h"
#include "Engine/PostFX/PostFXSys.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Core/Timer/Timer.h"
#include "Engine/Scene/Camera/Camera.h"
#include "Engine/GUI/IMGuiRenderer.h"
#include "Engine/GUI/GuiItems.h"
#include "Engine/Debug/DebugStats.h"
#include "Engine/Graphics/Lights/DirLight.h"
#include "Engine/GUI/GuiTab.h"
#include "Engine/GUI/GuiTabBar.h"
#include "TextureSettings.h"
#include "ShapeSettings.h"
#include "ParticleSettings.h"
#include "SortSettings.h"
#include "FileList.h"
#include "EmitterWindow.h"
#include "ParticlePreviewWindow.h"
#include "EffectGroup.h"
#include "EditorShapes.h"

void ReloadEmitterFromFileOrGetActive(usg::GFXDevice* pDevice, usg::ScriptEmitter* pEmitter, const char* szScriptName);

class ParticleEditor : public usg::GameInterface, public usg::GUICallbacks
{
public:
	ParticleEditor();
	virtual ~ParticleEditor();

	virtual void Init(usg::GFXDevice* pDevice);
	virtual void CleanUp(usg::GFXDevice* pDevice);
	virtual void Update(usg::GFXDevice* pDevice);
	virtual void Draw(usg::GFXDevice* pDevice);
	virtual void OnMessage(usg::GFXDevice* const pDevice, const uint32 messageID, const void* const pParameters);
	
	void ReloadEmitterFromFile(usg::GFXDevice* pDevice, usg::ScriptEmitter* pEmitter, const char* szScriptName);

	// GUICallbacks
	virtual void FileOption(const char* szName) override;
private:

	enum
	{
		MAX_FILE_COUNT = 512,
		MAX_FILE_NAME_STRING = 8192
	};

	usg::Timer				m_timer;
	usg::PostFXSys			m_postFX;

	usg::IMGuiRenderer		m_guiRend;
	EditorShapes			m_editorShapes;

	usg::DebugStats			m_debug;
	PreviewWindow			m_effectPreview;
	ParticlePreviewWindow	m_emitterPreview;
	usg::GUIMenu			m_windowMenu;
	usg::GUIMenuItem		m_resetWindow;
	usg::GUIMenuItem		m_increaseSize;
	usg::GUIMenuItem		m_decreaseSize;


	EffectGroup				m_effectGroup;
	EmitterWindow			m_emitterWindow;

	usg::ScriptEmitter		m_emitter;
	WindHndl				m_hwnd;

};


#endif
