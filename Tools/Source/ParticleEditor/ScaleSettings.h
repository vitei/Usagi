#ifndef _USG_PARTICLE_EDITOR_SCALE_SETTINGS_H_
#define _USG_PARTICLE_EDITOR_SCALE_SETTINGS_H_

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

class ScaleSettings : public EmitterModifier
{
public:
	ScaleSettings();
	~ScaleSettings();

	virtual void Init(usg::GFXDevice* pDevice, usg::IMGuiRenderer* pRenderer);
	virtual void SetWidgetsFromDefinition(usg::particles::EmitterEmission& structData);
	virtual bool Update(usg::GFXDevice* pDevice, usg::particles::EmitterEmission& structData, usg::ScriptEmitter* pEffect);

private:

	enum
	{
		SLIDER_RANDOMNESS = 0,
		SLIDER_INITIAL,
		SLIDER_INTERMEDIATE,
		SLIDER_ENDING,
		SLIDER_BEGIN_SCALE_IN,
		SLIDER_START_SCALE_OUT,
		SLIDER_COUNT
	};

	FloatAnim				m_baseScale;
	usg::GUISlider			m_sliders[SLIDER_COUNT];
};


#endif