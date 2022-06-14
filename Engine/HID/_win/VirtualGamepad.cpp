/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "VirtualGamepad.h"
#include "Engine/HID/Input.h"
#include "Engine/HID/InputStructs.h"
#include "Input_ps.h"

namespace usg{

struct KeyboardMapping
{
	GamepadButton	buttonMap;
	uint8			keyboardKey;
};

struct MouseMapping
{
	GamepadButton	buttonMap;
	uint32			mouseButton;
};

static const KeyboardMapping g_buttonMapping[] =
{
	{ GAMEPAD_BUTTON_A, 'L' },
	{ GAMEPAD_BUTTON_B, 'K' },
	{ GAMEPAD_BUTTON_X, 'I' },
	{ GAMEPAD_BUTTON_Y, 'O' },

	{ GAMEPAD_BUTTON_L, VK_NUMPAD4 },
	{ GAMEPAD_BUTTON_R, VK_NUMPAD6 },

	{ GAMEPAD_BUTTON_THUMB_L, VK_NUMPAD1 },
	{ GAMEPAD_BUTTON_THUMB_R, VK_NUMPAD3 },
	
	{ GAMEPAD_BUTTON_ZL, VK_NUMPAD7 },
	{ GAMEPAD_BUTTON_ZR, VK_NUMPAD9 },
    
	{ GAMEPAD_BUTTON_HOME, VK_RETURN },

	{ GAMEPAD_BUTTON_UP, VK_UP },
	{ GAMEPAD_BUTTON_DOWN, VK_DOWN },
	{ GAMEPAD_BUTTON_LEFT, VK_LEFT },
	{ GAMEPAD_BUTTON_RIGHT,VK_RIGHT },

	{ GAMEPAD_BUTTON_START, VK_NUMPAD8 },
	{ GAMEPAD_BUTTON_SELECT,VK_BACK },

	{ GAMEPAD_BUTTON_NONE, 0 },
};

static const MouseMapping g_mouseMapping[] =
{
	{ GAMEPAD_BUTTON_ZL, 0 },
	{ GAMEPAD_BUTTON_ZR, 1 },
	{ GAMEPAD_BUTTON_NONE, 0 },
};

VirtualGamepad::VirtualGamepad():
IGamepad()
{
}

void VirtualGamepad::Init(Keyboard* pKeyboard)
{
	m_pKeyboard = pKeyboard;
}

VirtualGamepad::~VirtualGamepad()
{

}


void VirtualGamepad::Update(GFXDevice* pDevice, GamepadDeviceState& deviceStateOut)
{
	deviceStateOut.uButtonsPrevDown = deviceStateOut.uButtonsDown;
	deviceStateOut.uButtonsDown = 0;

	for(int i=0; i<_GamepadAxis_count; i++)
	{
		deviceStateOut.fAxisValues[i] = 0.0f;
	}


	const KeyboardMapping* pMapping = g_buttonMapping;
	while(pMapping->buttonMap != GAMEPAD_BUTTON_NONE )
	{
		if(m_pKeyboard->GetKey(pMapping->keyboardKey, BUTTON_STATE_HELD))
		{
			deviceStateOut.uButtonsDown |= pMapping->buttonMap;//(1 << (pMapping->buttonMap - 1));
		}
		pMapping++;
	}

	const MouseMapping* pMouseMapping = g_mouseMapping;
	while(pMouseMapping->buttonMap != GAMEPAD_BUTTON_NONE )
	{
		if(Input::GetPlatform().GetMouseButtonDown(pMouseMapping->mouseButton))
		{
			deviceStateOut.uButtonsDown |= pMapping->buttonMap;//(1 << (pMapping->buttonMap - 1));
		}
		pMouseMapping++;
	}

	// Now handle the axes
	if( m_pKeyboard->GetKey('A') )
	{
		deviceStateOut.fAxisValues[GAMEPAD_AXIS_LEFT_X] -= 1.0f;
	}
	if( m_pKeyboard->GetKey('D') )
	{
		deviceStateOut.fAxisValues[GAMEPAD_AXIS_LEFT_X] += 1.0f;
	}
	if( m_pKeyboard->GetKey('W') )
	{
		deviceStateOut.fAxisValues[GAMEPAD_AXIS_LEFT_Y] += 1.0f;
	}
	if( m_pKeyboard->GetKey('S') )
	{
		deviceStateOut.fAxisValues[GAMEPAD_AXIS_LEFT_Y] -= 1.0f;
	}
	if( m_pKeyboard->GetKey('F') )
	{
		deviceStateOut.fAxisValues[GAMEPAD_AXIS_RIGHT_X] -= 1.0f;
	}
	if( m_pKeyboard->GetKey('H') )
	{
		deviceStateOut.fAxisValues[GAMEPAD_AXIS_RIGHT_X] += 1.0f;
	}
	if (m_pKeyboard->GetKey('T'))
	{
		deviceStateOut.fAxisValues[GAMEPAD_AXIS_RIGHT_Y] += 1.0f;
	}
	if (m_pKeyboard->GetKey('G'))
	{
		deviceStateOut.fAxisValues[GAMEPAD_AXIS_RIGHT_Y] -= 1.0f;
	}

}

}
