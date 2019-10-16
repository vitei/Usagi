#ifndef _USG_PARTICLE_EDITOR_FLOAT_ANIM_H_
#define _USG_PARTICLE_EDITOR_FLOAT_ANIM_H_

#include "Engine/Graphics/Device/GFXDevice.h"
#include "Engine/Graphics/Device/StateEnums.pb.h"
#include "Engine/Particles/Scripted/ScriptEmitter.pb.h"
#include "Engine/GUI/GuiWindow.h"
#include "Engine/GUI/GuiItems.h"

namespace usg
{
	class IMGuiRenderer;
}

class FloatAnim
{
public:
	FloatAnim() {}
	~FloatAnim() {}

	void Init(usg::GUIWindow* pWindow, const char* szName);
	uint32 GetFrameCount() { return (uint32)m_frameCount.GetValue()[0]; }
	//const int* GetFrameInfo(uint32 uFrame) { return m_frameInfo[uFrame].GetValue(); }
	bool Update(usg::particles::FloatAnim &src);
	void SetFromDefinition(usg::particles::FloatAnim &src);
	void SetVisible(bool bVisible);
	void SetToolTip(const char* szChar);
	void SetSingleOnly(bool bSingleOnly);
	
private:
	
	
	enum
	{
		MAX_FRAME_COUNT = 10,
	};

	usg::GUIFloat			m_singleFloat;
	usg::GUIText			m_title;
	usg::GUIWindow			m_childWindow;
	usg::GUIIntInput		m_frameCount;
	usg::GUIFloat			m_frameInfo[MAX_FRAME_COUNT];
};


#endif
