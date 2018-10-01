/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Raw user input (mouse/keyboard/touchscreen/pointer etc)
*****************************************************************************/
#ifndef __USG_HID_PC_INPUT_PS_H__
#define __USG_HID_PC_INPUT_PS_H__
#include "Engine/Common/Common.h"
#include "Engine/HID/InputDefines.h"
#include "Engine/Core/stl/vector.h"
#include "Engine/HID/Microphone.h"
#include OS_HEADER(Engine/HID, XInputPad.h)
#include OS_HEADER(Engine/HID, VirtualGamepad.h)
#include OS_HEADER(Engine/HID, Keyboard_ps.h)
#include OS_HEADER(Engine/HID, Mouse_ps.h)
#include OS_HEADER(Engine/HID, DirectInput.h)
#include OS_HEADER(Engine/HID, DirectInputJoystick.h)

namespace usg{

class Gamepad;
class GFXDevice;

class Input_ps
{
public:
	Input_ps();
	~Input_ps();
	
	void Init();
	void Cleanup();
	void Update(GFXDevice* pDevice);

	void		GetActiveGamepads(usg::vector<IGamepad*>& gamepads);
	Gamepad*	GetAccerometer() { return NULL; }	// No support on PC yet
	Microphone* GetMicrophone(uint32 uMic) { return NULL; }
	Keyboard*	GetKeyboard() { return &m_keyboard; }
	Mouse*		GetMouse() { return &m_mouse; }

	// Set via callbacks, should not be accessed directly as we're only using a mouse and keyboard
	// for the PC build, but we will have a virtual gamepad based on this information
	void		RegisterKeyStateChange(uint8 keyDown, bool bDown);
	void		RegisterMouseButtonChange(uint32 uButton, bool bValue);
	void		RegisterMouseWheel(long move) { m_mouseWheel += move; }
	void		RegisterInputChar(char16 uChar);
	void		RegisterDeviceChange();
	void		RegisterGamepad(IGamepad* pGamepad);

	bool		GetKeyDown(uint8 uKey) const;
	bool		GetMouseButtonDown(uint32 uButton) const;
	long		GetMouseWheel() const { return m_mouseWheel; }
	uint32		GetInputChars() const { return m_uInputChars; }
	char16		GetInputChar(uint32 uIndex) const { return m_inputChars[uIndex]; }

	void		RegisterHwnd(uint32 uDisplay, HWND hwnd);

private:
	PRIVATIZE_COPY(Input_ps);
	
	enum
	{
		MOUSE_BUTTON_COUNT = 3,
		MAX_EXTERNAL_PADS = 4
	};

	VirtualGamepad	m_virtualGamepad;
	Keyboard_ps		m_keyboard;
	// TODO: Have a mouse per display
	Mouse_ps		m_mouse;

	XInputPad		m_xboxPad;
	DirectInput*	m_pDirectInput;
	IGamepad*		m_pExternalPads[MAX_EXTERNAL_PADS];
	uint32			m_uExternalPads;

	long		m_mouseWheel;
	char16		m_inputChars[Keyboard::MAX_INPUT_CHARS];
	uint32		m_uInputChars;
	bool		m_keysDown[KEYBOARD_KEY_COUNT];
	bool		m_keysThisFrame[KEYBOARD_KEY_COUNT];
	bool		m_mouseButtonsDown[MOUSE_BUTTON_COUNT];
	bool		m_mouseButtonsThisFrame[MOUSE_BUTTON_COUNT];
};

inline bool Input_ps::GetKeyDown(uint8 uKey) const
{
	ASSERT(uKey<KEYBOARD_KEY_COUNT);
	return m_keysDown[uKey] | m_keysThisFrame[uKey];
}


inline bool	Input_ps::GetMouseButtonDown(uint32 uButton) const
{
	ASSERT(uButton<MOUSE_BUTTON_COUNT);
	return m_mouseButtonsDown[uButton] | m_mouseButtonsThisFrame[uButton];
}



}

#endif