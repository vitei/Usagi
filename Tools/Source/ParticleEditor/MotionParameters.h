#ifndef _USG_PARTICLE_EDITOR_MOTION_PARAMETERS_H_
#define _USG_PARTICLE_EDITOR_MOTION_PARAMETERS_H_

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

class MotionParameters : public EmitterModifier
{
public:
	MotionParameters();
	~MotionParameters();

	virtual void Init(usg::GFXDevice* pDevice, usg::IMGuiRenderer* pRenderer);
	virtual void SetWidgetsFromDefinition(usg::particles::EmitterEmission& structData);
	virtual bool Update(usg::GFXDevice* pDevice, usg::particles::EmitterEmission& structData, usg::ScriptEmitter* pEffect, float fElapsed);

private:

	enum
	{
		SLIDER_POSITION_RANDOM = 0,
		SLIDER_VELOCITY_DIR,
		SLIDER_DIR_VELOCITY_CONE_DEG,
		SLIDER_SPEED_RANDOMNESS,
		SLIDER_GRAVITY_DIR,
		SLIDER_DRAG,
		SLIDER_COUNT,
		FLOAT_ENTRY_GRAVITY_STRENGTH = 0,
		FLOAT_ENTRY_COUNT,
		FLOAT_ANIM_OMNI_VELOCITY = 0,
		FLOAT_ANIM_DIR_VELOCITY,
		FLOAT_ANIM_COUNT,
		CHECKBOX_INHERIT_VELOCITY = 0,
		CHECKBOX_LOCAL_SPACE,
		CHECKBOX_UPDATE_ON_CPU,
		CHECKBOX_COUNT
	};

	FloatAnim				m_anims[FLOAT_ANIM_COUNT];
	usg::GUICheckBox		m_checkBoxes[CHECKBOX_COUNT];
	usg::GUISlider			m_sliders[SLIDER_COUNT];
	usg::GUIFloat			m_floatEntries[FLOAT_ENTRY_COUNT];
};


#endif