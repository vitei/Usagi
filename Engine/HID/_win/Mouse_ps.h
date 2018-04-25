/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Platform specific version of a mouse. A bit shite for now
*****************************************************************************/
#ifndef __USG_HID_PC_MOUSE_PS_H__
#define __USG_HID_PC_MOUSE_PS_H__
#include "Engine/Common/Common.h"
#include "Engine/HID/Gamepad.h"
#include "Engine/HID/Mouse.h"

namespace usg{

class Input_ps;

class Mouse_ps : public Mouse
{
public:
	Mouse_ps() {}
	~Mouse_ps() {}

	void Init(Input_ps* pOwner)
	{
		m_pOwner = pOwner;
		for(uint32 i=0; i<_MouseButton_count; i++)
		{
			m_prevButtons[i] = false;
			m_buttons[i] = false;
		}
	}

	void Update();
	void SetHWND(HWND hwnd) { m_hwnd = hwnd; }

private:
	const Input_ps*	m_pOwner;
	HWND			m_hwnd;
};

}

#endif
