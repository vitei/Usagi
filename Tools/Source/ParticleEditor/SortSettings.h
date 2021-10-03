#ifndef _USG_PARTICLE_EDITOR_SORT_SETTINGS_H_
#define _USG_PARTICLE_EDITOR_SORT_SETTINGS_H_

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

class SortSettings : public EmitterModifier
{
public:
	SortSettings();
	~SortSettings();

	virtual void Init(usg::GFXDevice* pDevice, usg::IMGuiRenderer* pRenderer);
	virtual bool Update(usg::GFXDevice* pDevice, usg::particles::EmitterEmission& structData, usg::ScriptEmitter* pEffect, float fElapsed);
	virtual void SetWidgetsFromDefinition(usg::particles::EmitterEmission& structData);

private:

	bool						m_bForceUpdate;
	usg::GUIComboBox			m_renderLayer;
	usg::GUIIntInput			m_priority;
	usg::GUICheckBox			m_depthWrite;
	usg::GUIFloat				m_depthOffset;
};


#endif