#ifndef _USG_PARTICLE_EDITOR_BLEND_SETTINGS_H_
#define _USG_PARTICLE_EDITOR_BLEND_SETTINGS_H_

#include "Engine/Graphics/Device/GFXDevice.h"
#include "Engine/Graphics/Device/StateEnums.pb.h"
#include "Engine/GUI/GuiWindow.h"
#include "Engine/GUI/GuiItems.h"
#include "EmitterModifier.h"

namespace usg
{
	class IMGuiRenderer;
}

class BlendSettings : public EmitterModifier
{
public:
	BlendSettings();
	~BlendSettings();

	virtual void Init(usg::GFXDevice* pDevice, usg::IMGuiRenderer* pRenderer);
	virtual void SetWidgetsFromDefinition(usg::particles::EmitterEmission& structData);
	virtual bool Update(usg::GFXDevice* pDevice, usg::particles::EmitterEmission& structData, usg::ScriptEmitter* pEffect);
	
private:

	enum
	{
		COMBO_BOX_RGB_OP = 0,
		COMBO_BOX_RGB_SRC,
		COMBO_BOX_RGB_DST,
		COMBO_BOX_ALPHA_OP,
		COMBO_BOX_ALPHA_SRC,
		COMBO_BOX_ALPHA_DST,
		COMBO_BOX_ALPHA_TEST_OP,
		COMBO_BOX_COUNT
	};

	usg::GUIComboBox		m_comboBoxes[COMBO_BOX_COUNT];
	usg::GUIColorSelect		m_constColorSelect;
	usg::GUISlider			m_alphaRef;
	usg::GUISlider			m_softDistance;
	usg::GUISlider			m_cameraOffset;
	bool					m_bForceUpdate;
};


#endif
