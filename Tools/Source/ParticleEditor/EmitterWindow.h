#pragma once

#include "Engine/Particles/Scripted/ScriptEmitter.h"
#include "Engine/Particles/Scripted/EffectGroup.pb.h"
#include "Engine/Particles/ParticleEffect.h"
#include "Engine/GUI/GuiItems.h"
#include "Engine/GUI/GuiWindow.h"
#include "Engine/GUI/GuiMenu.h"
#include "Engine/GUI/GuiTabBar.h"
#include "Engine/GUI/GuiTab.h"
#include "Engine/GUI/IMGuiRenderer.h"
#include "MayaCamera.h"
#include "ShapeSettings.h"
#include "EditorShapes.h"
#include "ParticleSettings.h"
#include "SortSettings.h"
#include "BlendSettings.h"
#include "EmissionSettings.h"
#include "RotationSettings.h"
#include "ColorSettings.h"
#include "AlphaSettings.h"
#include "TextureSettings.h"
#include "ScaleSettings.h"
#include "MotionParameters.h"
#include "EmitterInstance.h"
#include "RibbonInstance.h"
#include "FileList.h"

class EmitterWindow : public usg::GUICallbacks
{
public:
	EmitterWindow();
	~EmitterWindow() {}

	void Init(usg::GFXDevice* pDevice, usg::IMGuiRenderer* pRenderer);
	void CleanUp(usg::GFXDevice* pDevice);
	void Update(usg::GFXDevice* pDevice, float fElapsed);

	usg::GUIWindow&		GetWindow() { return m_emitterWindow; }
	ShapeSettings& GetShapeSettings() { return m_shapeSettings; }
	usg::particles::EmitterEmission& GetVariables() { return m_variables; }
	usg::List<EmitterModifier>& GetModifiers() { return m_modifiers; }
	bool IsShapeTabOpen() const { return m_emitterShapeTab.IsOpen(); }
	const char* GetEditFileName() const { return m_fileName.GetName(); }

	// GUI Callbacks
	virtual void LoadCallback(const char* szName, const char* szFilePath, const char* szRelPath) override;
	virtual void SaveCallback(const char* szName, const char* szFilePath, const char* szRelPath) override;
	virtual void FileOption(const char* szName) override;
	bool GetLoaded() { bool bRet = m_bLoaded; m_bLoaded = false; return bRet; }
private:
	usg::particles::EmitterEmission	m_variables;
	ShapeSettings			m_shapeSettings;
	SortSettings			m_sortSettings;

	BlendSettings			m_blendSettings;
	EmissionSettings		m_emissionSettings;
	RotationSettings		m_rotationSettings;
	ColorSettings			m_colorSettings;
	AlphaSettings			m_alphaSettings;
	ScaleSettings			m_scaleSettings;
	MotionParameters		m_motionParams;
	TextureSettings			m_textureSettings;
	ParticleSettings		m_particleSettings;

	usg::GUIWindow			m_emitterWindow;
	usg::GUIMenu			m_fileMenu;
	usg::GUIMenuLoadSave	m_saveAsItem;
	usg::GUIMenuLoadSave	m_loadItem;
	usg::GUIMenuItem		m_saveItem;
	usg::GUIText			m_fileName;

	usg::GUITabBar			m_emitterTabBar;
	usg::GUITab				m_emitterShapeTab;
	usg::GUITab				m_emissionTab;
	usg::GUITab				m_textureTab;
	usg::GUITab				m_colorTab;
	usg::GUITab				m_blendTab;
	usg::GUITab				m_scaleTab;
	usg::GUITab				m_motionTab;

	usg::List<EmitterModifier>	m_modifiers;
	bool						m_bLoaded;

};

