/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Platform specific version of a keyboard. A bit shite for now
*****************************************************************************/
#ifndef __USG_HID_PC_KEYBOARD_PS_H__
#define __USG_HID_PC_KEYBOARD_PS_H__

#include "Engine/HID/Gamepad.h"
#include "Engine/HID/Keyboard.h"

namespace usg{

class Input_ps;

class Keyboard_ps : public Keyboard
{
public:
	Keyboard_ps() {}
	~Keyboard_ps() {}

	void Init(Input_ps* pOwner)
	{
		m_pOwner = pOwner;
		for(uint32 i=0; i<KEYBOARD_KEY_COUNT; i++)
		{
			m_prevKeysDown[i] = false;
			m_keysDown[i] = false;
		}
	}

	void Update();

private:
	const Input_ps*	m_pOwner;
};

}

#endif
