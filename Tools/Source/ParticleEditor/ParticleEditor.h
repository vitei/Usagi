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
#include "PreviewModel.h"
#include "MayaCamera.h"
#include "BlendSettings.h"
#include "EmissionSettings.h"
#include "RotationSettings.h"
#include "ColorSettings.h"
#include "AlphaSettings.h"
#include "ScaleSettings.h"
#include "MotionParameters.h"
#include "TextureSettings.h"
#include "ShapeSettings.h"
#include "ParticleSettings.h"
#include "SortSettings.h"
#include "FileList.h"
#include "EffectGroup.h"
#include "ColorSelection.h"
#include "EditorShapes.h"

void ReloadEmitterFromFileOrGetActive(usg::GFXDevice* pDevice, usg::ScriptEmitter* pEmitter, const char* szScriptName);

class ParticleEditor : public usg::GameInterface
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
private:

	enum
	{
		BUTTON_PLAY = 0,
		BUTTON_PAUSE,
		BUTTON_RESTART,
		BUTTON_COUNT,
		MAX_FILE_COUNT = 512,
		MAX_FILE_NAME_STRING = 8192
	};

	usg::Timer				m_timer;
	usg::PostFXSys			m_postFX;
	usg::Scene				m_scene;
	usg::Viewport			m_previewViewport;
	usg::ViewContext*		m_pSceneCtxt;
	MayaCamera				m_camera;

	usg::IMGuiRenderer		m_guiRend;
	BlendSettings			m_blendSettings;
	EmissionSettings		m_emissionSettings;
	RotationSettings		m_rotationSettings;
	ColorSettings			m_colorSettings;
	AlphaSettings			m_alphaSettings;
	ScaleSettings			m_scaleSettings;
	MotionParameters		m_motionParams;
	TextureSettings			m_textureSettings;
	ShapeSettings			m_shapeSettings;
	SortSettings			m_sortSettings;
	ParticleSettings		m_particleSettings;
	EditorShapes			m_editorShapes;
	usg::DebugStats			m_debug;
	PreviewModel			m_previewModel;

	usg::GUIWindow			m_emitterWindow;
	usg::GUIWindow			m_lifeMotionWindow;
	usg::GUIWindow			m_particleAppearanceWindow;
	usg::GUIWindow			m_effectWindow;
	usg::GUIWindow			m_previewWindow;
	usg::GUIButton			m_previewButtons[BUTTON_COUNT];
	usg::GUIWindow			m_fileWindow;
	usg::GUIComboBox		m_loadFilePaths;
	usg::GUIButton			m_loadButton;
	usg::GUITextInput		m_saveFile;
	usg::GUIButton			m_saveButton;
	usg::GUICheckBox		m_repeat;
	usg::GUIColorSelect		m_clearColor;
	usg::GUIComboBox		m_previewType;
	ColorSelection			m_colorSelection;
	bool					m_bPaused;
	FileList<MAX_FILE_COUNT> m_fileList;
	EffectGroup				m_effectGroup;
	usg::U8String			m_activeEdit;

	usg::List<EmitterModifier>	m_modifiers;
	usg::particles::EmitterEmission	m_variables;
	usg::ScriptEmitter			m_emitter;
	usg::ParticleEffect		m_effect;
};


#endif
