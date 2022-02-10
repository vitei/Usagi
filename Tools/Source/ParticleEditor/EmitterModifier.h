#ifndef _USG_PARTICLE_EDITOR_EMITTER_MODIFIER_H_
#define _USG_PARTICLE_EDITOR_EMITTER_MODIFIER_H_

#include "Engine/Graphics/Device/GFXDevice.h"
#include "Engine/Graphics/Device/StateEnums.pb.h"
#include "Engine/Particles/Scripted/ScriptEmitter.h"
#include "Engine/Particles/Scripted/ScriptEmitter.pb.h"
#include "Engine/GUI/GuiWindow.h"
#include "Engine/GUI/GuiTab.h"
#include "Engine/GUI/GuiItems.h"

namespace usg
{
	class IMGuiRenderer;
}


class EmitterModifier
{
public:
	EmitterModifier() {}
	virtual ~EmitterModifier() {}

	virtual void Init(usg::GFXDevice* pDevice, usg::IMGuiRenderer* pRenderer) = 0;
	virtual void SetWidgetsFromDefinition(usg::particles::EmitterEmission& structData) = 0;
	virtual bool Update(usg::GFXDevice* pDevice, usg::particles::EmitterEmission& structData, usg::ScriptEmitter* pEffect, float fElapsed) = 0;
	
	template <class VariableType, class ComparisonType>
	inline bool Compare(VariableType& inOut, const ComparisonType newValue)
	{
		if(inOut == newValue)
		{
			return false;
		}
		else
		{
			inOut = (VariableType)newValue;
			return true;
		}
	}

	bool CompareUnitVector(usg::Vector3f& vInOut, const usg::Vector3f newValue, const usg::Vector3f safetyValue);

	void AddToWindow(usg::GUIWindow& parent)
	{
		parent.AddItem(&m_window);
	}

	void AddToTab(usg::GUITab& parent)
	{
		parent.AddItem(&m_window);
	}

protected:
	usg::GUIWindow m_window;
	
};


#endif
