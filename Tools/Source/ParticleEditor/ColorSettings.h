#ifndef _USG_PARTICLE_EDITOR_COLOR_SETTINGS_H_
#define _USG_PARTICLE_EDITOR_COLOR_SETTINGS_H_

#include "Engine/Graphics/Device/GFXDevice.h"
#include "Engine/Graphics/Device/StateEnums.pb.h"
#include "Engine/GUI/GuiWindow.h"
#include "Engine/GUI/GuiItems.h"
#include "ColorSelection.h"
#include "EmitterModifier.h"
#include "FloatAnim.h"

namespace usg
{
	class IMGuiRenderer;
}

class ColorSettings : public EmitterModifier
{
public:
	ColorSettings();
	~ColorSettings();

	void SetColorSelection(ColorSelection* pSelection) { m_pColorSelection = pSelection; }
	virtual void Init(usg::GFXDevice* pDevice, usg::IMGuiRenderer* pRenderer);
	virtual void SetWidgetsFromDefinition(usg::particles::EmitterEmission& structData);
	virtual bool Update(usg::GFXDevice* pDevice, usg::particles::EmitterEmission& structData, usg::ScriptEmitter* pEffect);

private:

	enum
	{
		COLOR_COUNT = 3,
		SLIDER_TIME_IN_END = 0,
		SLIDER_PEAK_TIME,
		SLIDER_TIME_OUT_START,
		SLIDER_ENV_COLOR_LERP,
		SLIDER_COUNT
	};

	ColorSelection*			m_pColorSelection;
	usg::GUIComboBox		m_colorAnimMode;
	usg::GUIColorSelect		m_colors[COLOR_COUNT];
	usg::GUICheckBox		m_randomRepetition;
	usg::GUIIntInput		m_repetition;
	usg::GUISlider			m_sliders[SLIDER_COUNT];
};


#endif