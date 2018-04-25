#ifndef _USG_PARTICLE_EDITOR_ALPHA_SETTINGS_H_
#define _USG_PARTICLE_EDITOR_ALPHA_SETTINGS_H_
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

class AlphaSettings : public EmitterModifier
{
public:
	AlphaSettings();
	~AlphaSettings();

	virtual void Init(usg::GFXDevice* pDevice, usg::IMGuiRenderer* pRenderer);
	virtual bool Update(usg::GFXDevice* pDevice, usg::particles::EmitterEmission& structData, usg::ScriptEmitter* pEffect);
	virtual void SetWidgetsFromDefinition(usg::particles::EmitterEmission& structData);

private:

	enum
	{
		SLIDER_INITIAL_ALPHA = 0,
		SLIDER_INTERMEDIATE_ALPHA,
		SLIDER_END_ALPHA,
		SLIDER_FINISH_IN_TIME,
		SLIDER_FADE_OUT_TIMING,
		SLIDER_COUNT
	};


	usg::GUISlider			m_sliders[SLIDER_COUNT];
};


#endif