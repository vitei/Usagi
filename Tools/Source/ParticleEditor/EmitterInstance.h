#ifndef _USG_PARTICLE_EDITOR_EMITTER_INSTANCE_H_
#define _USG_PARTICLE_EDITOR_EMITTER_INSTANCE_H_

#include "Engine/Particles/Scripted/ScriptEmitter.h"
#include "Engine/Particles/Scripted/EffectGroup.pb.h"
#include "Engine/GUI/GuiItems.h"
#include "Engine/GUI/GuiWindow.h"
#include "Engine/GUI/IMGuiRenderer.h"
#include "FileList.h"

class EmitterInstance
{
public:
	EmitterInstance() {}
	~EmitterInstance() {}

	void Init(usg::GFXDevice* pDevice, usg::Scene& scene, class EffectGroup* pGroup, uint32 uIndex);
	void Cleanup(usg::GFXDevice* pDevice);
	bool Update(usg::GFXDevice* pDevice, float fElapsed);
	void AddToScene(usg::GFXDevice* pDevice, usg::particles::EmitterData* pInstance = NULL);
	void RemoveFromScene();
	bool GetActive() { return m_bActive; }
	void UpdateEmitter(usg::GFXDevice* pDevice, usg::Scene& scene, const usg::particles::EmitterEmission& emitterData, const usg::particles::EmitterShapeDetails& shapeData);
	const usg::particles::EmitterData& GetData() { return m_emitterData; }
	usg::ScriptEmitter& GetEmitter() { return m_emitter; }
	bool GetLoadRequest() { return m_loadEmitterButton.GetValue(); }

	void LoadEmitter(usg::GFXDevice* pDevice, const char* szEmitterName);
private:
	void Add(bool bAdd) { m_emitterWindow.SetVisible(bAdd); m_bActive = bAdd; }
	void UpdateInstanceMatrix();

	usg::GUIWindow				m_emitterWindow;
	usg::GUIWindow				m_parameterWindow;
	usg::GUIText				m_emitterName;
	usg::GUIButton				m_loadEmitterButton;
	usg::GUILoadButton			m_changeAssetButton;
	usg::GUIFloat				m_position;
	usg::GUIFloat				m_rotation;
	usg::GUIFloat				m_scale;
	usg::GUIFloat				m_particleScale;
	usg::GUIFloat				m_startTime;
	usg::GUITextInput			m_eventName;
	
	usg::GUIButton				m_removeEmitterButton;
	usg::ScriptEmitter			m_emitter;

	usg::particles::EmitterData m_emitterData;
	bool						m_bActive;
	class EffectGroup*			m_pParent;
};


#endif
