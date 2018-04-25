#ifndef _USG_PARTICLE_EDITOR_PARTICLE_SETTINGS_H_
#define _USG_PARTICLE_EDITOR_PARTICLE_SETTINGS_H_
#include "Engine/Common/Common.h"
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

class ParticleSettings : public EmitterModifier
{
public:
	ParticleSettings();
	~ParticleSettings();

	virtual void Init(usg::GFXDevice* pDevice, usg::IMGuiRenderer* pRenderer);
	virtual void SetWidgetsFromDefinition(usg::particles::EmitterEmission& structData);
	virtual bool Update(usg::GFXDevice* pDevice, usg::particles::EmitterEmission& structData, usg::ScriptEmitter* pEffect);

private:

	
	FloatAnim				m_life;
	usg::GUIComboBox		m_particleType;
	usg::GUISlider			m_lifeRandomness;
	usg::GUISlider			m_particleCenter;
	usg::GUIFloat			m_userRotation;
};


#endif