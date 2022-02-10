#ifndef _USG_PARTICLE_EDITOR_EMISSION_SETTINGS_H_
#define _USG_PARTICLE_EDITOR_EMISSION_SETTINGS_H_

#include "Engine/Graphics/Device/GFXDevice.h"
#include "Engine/Graphics/Device/StateEnums.pb.h"
#include "Engine/GUI/GuiWindow.h"
#include "Engine/GUI/GuiItems.h"
#include "EmitterModifier.h"
#include "FloatAnim.h"

namespace usg
{
	class IMGuiRenderer;
}

class EmissionSettings : public EmitterModifier
{
public:
	EmissionSettings();
	~EmissionSettings();

	virtual void Init(usg::GFXDevice* pDevice, usg::IMGuiRenderer* pRenderer);
	virtual void SetWidgetsFromDefinition(usg::particles::EmitterEmission& structData);
	virtual bool Update(usg::GFXDevice* pDevice, usg::particles::EmitterEmission& structData, usg::ScriptEmitter* pEffect, float fElapsed);
	
private:
	
	enum
	{
		SLIDER_RELEASE_INTERVAL = 0,
		SLIDER_RELEASE_INTERVAL_RANDOM,
		SLIDER_EMISSION_TIME,
		SLIDER_COUNT
	};

	usg::GUIIntInput		m_maxParticles;
	//usg::GUICheckBox		m_oneTimeCheckBox;
	usg::GUIComboBox		m_emissionType;
	usg::GUISlider			m_releaseRandom;
	usg::GUIFloat			m_sliders[SLIDER_COUNT];
	FloatAnim				m_emissionTiming;
};


#endif
