#ifndef _USG_PARTICLE_EDITOR_EFFECT_GROUP_H_
#define _USG_PARTICLE_EDITOR_EFFECT_GROUP_H_
#include "Engine/Common/Common.h"
#include "Engine/Particles/Scripted/ScriptEmitter.h"
#include "Engine/Particles/Scripted/EffectGroup.pb.h"
#include "Engine/Particles/ParticleEffect.h"
#include "Engine/GUI/GuiItems.h"
#include "Engine/GUI/GuiWindow.h"
#include "Engine/GUI/IMGuiRenderer.h"
#include "EmitterInstance.h"
#include "RibbonInstance.h"
#include "FileList.h"

class EffectGroup
{
public:
	enum
	{
		MAX_INSTANCES = usg::particles::EffectGroup::emitters_max_count,
		MAX_RIBBONS = usg::particles::EffectGroup::ribbons_max_count,
		MAX_FILE_COUNT = 512
	};

	EffectGroup() {}
	~EffectGroup() {}

	void Init(usg::GFXDevice* pDevice, usg::Scene* pScene, usg::IMGuiRenderer* pRenderer, ColorSelection* pSelection);
	void CleanUp(usg::GFXDevice* pDevice);
	void Update(usg::GFXDevice* pDevice, float fElapsed, bool bRepeat, bool bPause, bool bRestart);
	void EmitterModified(usg::GFXDevice* pDevice, const char* szName, const usg::particles::EmitterEmission& emitterData, const usg::particles::EmitterShapeDetails& shapeData);
	usg::Color GetBackgroundColor() const;
	void SetBackgroundColor(const usg::Color& color);

	FileList<MAX_FILE_COUNT>&	GetFileList() { return m_instanceFileList; }
	FileList<MAX_FILE_COUNT>&	GetRibbonTexFileList() { return m_textureFileList; }
	usg::GUIWindow&				GetWindow() { return m_window; }
	usg::ParticleEffect&		GetEffect() { return m_effect; }
	void						Reset(usg::GFXDevice* pDevice);
	bool						LoadEmitterRequested(usg::U8String& name);
private:

	EmitterInstance				m_instances[MAX_INSTANCES];
	RibbonInstance				m_ribbons[MAX_RIBBONS];
	

	FileList<MAX_FILE_COUNT>	m_fileList;
	FileList<MAX_FILE_COUNT>	m_instanceFileList;
	FileList<MAX_FILE_COUNT>	m_textureFileList;
	usg::GUIComboBox			m_loadFilePaths;
	usg::GUIButton				m_loadButton;
	usg::GUITextInput			m_saveFile;
	usg::GUIButton				m_saveButton;
	
	usg::GUIWindow					m_window;
	usg::GUIButton					m_addEmitterButton;
	usg::GUIButton					m_addTrailButton;
	usg::GUIIntInput				m_instanceCount;
	usg::particles::EffectGroup		m_effectGroup;
	usg::Scene*						m_pScene;

	usg::ParticleEffect				m_effect;
};


#endif
