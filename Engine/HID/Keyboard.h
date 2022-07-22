/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: The base class for the keyboard (PC and debug purposes)
*****************************************************************************/
#ifndef __USG_HID_KEYBOARD__
#define __USG_HID_KEYBOARD__

#include "Engine/HID/InputDefines.h"
#include "Engine/HID/InputEnums.pb.h"

namespace usg{

class Keyboard
{
public:
    Keyboard(){ m_uInputChars = 0; }
    ~Keyboard(){}

	bool GetKey(uint32 keyboardKey, ButtonState eState = BUTTON_STATE_HELD) const;
	bool GetToggleHeld(KeyboardToggle eToggle) const { return m_bToggles[eToggle]; }

	char16 GetInputChar(uint32 uChar) const { return m_inputChars[uChar]; }
	uint32 GetInputCharCount() const { return m_uInputChars; }

	enum
	{
		MAX_INPUT_CHARS = 16
	};

protected:
	
	char16	m_inputChars[MAX_INPUT_CHARS];
	uint32	m_uInputChars;

	bool	m_prevKeysDown[KEYBOARD_KEY_COUNT];
	bool	m_keysDown[KEYBOARD_KEY_COUNT];
	bool	m_bToggles[KEYBOARD_TOGGLE_COUNT];
};


}

#endif
