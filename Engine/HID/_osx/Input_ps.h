/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Raw user input (mouse/keyboard/touchscreen/pointer etc)
*****************************************************************************/
#ifndef __USG_HID_PC_INPUT_PS_H__
#define __USG_HID_PC_INPUT_PS_H__
#include "Engine/Common/Common.h"
#include "Engine/HID/InputDefines.h"
#include "Engine/HID/Microphone.h"
#include OS_HEADER(Engine/HID, XBoxPad.h)
#include OS_HEADER(Engine/HID, PSPad.h)
#include OS_HEADER(Engine/HID, VirtualGamepad.h)

#ifdef USE_VR
#include VR_HEADER(Engine/HID, OVRSensor.h)
#endif

namespace usg
{

class Gamepad;
class Keyboard;
class Mouse;

class Input_ps
{
public:
	Input_ps();
	~Input_ps();
	
	void Init();
	void Update();

	Gamepad*	GetGamepad(uint32 uGamepad);
	Gamepad*	GetAccerometer();
	Microphone* GetMicrophone(uint32 uMic) { return NULL; }
	Keyboard*	GetKeyboard() { return NULL; }
	Mouse*		GetMouse() { return NULL; }
	uint32		GetGamepadCount() { return m_uGamepads; }
	

	// Set via callbacks, should not be accessed directly as we're only using a mouse and keyboard
	// for the PC build, but we will have a virtual gamepad based on this information
	void		RegisterKeyStateChange(uint8 keyDown, bool bDown);
	void		RegisterMouseButtonChange(uint32 uButton, bool bValue);
	void		RegisterMouseMove(uint32 uAxis, long move);

	bool		GetKeyDown(int uKey) const;
	bool		GetMouseButtonDown(uint32 uButton) const;
    float       GetMouseX() const;
    float       GetMouseY() const;

private:
	PRIVATIZE_COPY(Input_ps);
	
	enum
	{
		MAX_CONTROLLERS = 3,
		MOUSE_BUTTON_COUNT = 3
	};

	XBoxPad			m_xboxPad;
	PSPad			m_psPad;
	VirtualGamepad	m_keyboard;
	
#ifdef USE_VR
	OVRSensor		m_ovrSensor;
#endif


	Gamepad*	m_gamepads[MAX_CONTROLLERS];
	uint32		m_uGamepads;

};

inline Gamepad*	Input_ps::GetGamepad(uint32 uGamepad)
{
	if(uGamepad < m_uGamepads)
	{
		return m_gamepads[uGamepad];
	}

	// ASSERT(false);
	return NULL;
}

inline Gamepad*	Input_ps::GetAccerometer()
{
	for(int i=0; i<m_uGamepads; i++)
	{
		if (m_gamepads[i]->HasAccelerometer())
			return m_gamepads[i];
	}
	return NULL;
}

}

#endif
