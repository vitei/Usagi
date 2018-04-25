#ifndef _USG_PARTICLE_EDITOR_ROTATION_SETTINGS_H_
#define _USG_PARTICLE_EDITOR_ROTATION_SETTINGS_H_
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

class RotationSettings : public EmitterModifier
{
public:
	RotationSettings();
	~RotationSettings();

	virtual void Init(usg::GFXDevice* pDevice, usg::IMGuiRenderer* pRenderer);
	virtual void SetWidgetsFromDefinition(usg::particles::EmitterEmission& structData);
	virtual bool Update(usg::GFXDevice* pDevice, usg::particles::EmitterEmission& structData, usg::ScriptEmitter* pEffect);

private:

	enum
	{
		SLIDER_BASE_ROTATION = 0,
		SLIDER_RANDOMISE,
		SLIDER_SPEED,
		SLIDER_SPEED_RANDOMISE,
		SLIDER_COUNT
	};


	usg::GUISlider			m_sliders[SLIDER_COUNT];
};


#endif