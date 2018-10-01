/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Memory/MemUtil.h"
#include OS_HEADER(Engine/HID, VirtualGamepad.h)
#include "Input_ps.h"
#include <string.h>

namespace usg{

Input_ps::Input_ps()
{
	memset(m_keysDown, 0, sizeof(bool)*KEYBOARD_KEY_COUNT);
	MemClear(m_pExternalPads, sizeof(m_pExternalPads));
	m_mouseWheel = 0;
	m_uInputChars = 0;
	m_uExternalPads = 0;
	m_pDirectInput = nullptr;
}

Input_ps::~Input_ps()
{

}

void Input_ps::GetActiveGamepads(usg::vector<IGamepad*>& gamepads)
{
	// First add any external pads, giving them priority
	for (uint32 i = 0; i < m_uExternalPads; i++)
	{
		if (m_pExternalPads[i]->IsConnected())
		{
			gamepads.push_back(m_pExternalPads[i]);
		}
	}

	if (m_xboxPad.IsConnected())
		gamepads.push_back(&m_xboxPad);

	if (m_virtualGamepad.IsConnected())
		gamepads.push_back(&m_virtualGamepad);
}


void Input_ps::Init()
{
	m_pDirectInput = vnew(ALLOC_OBJECT)DirectInput;
	m_pDirectInput->Init();
	m_keyboard.Init(this);
	m_mouse.Init(this);
	m_virtualGamepad.Init(&m_keyboard);
	m_xboxPad.Init(0);
}

void Input_ps::Cleanup()
{
	if (m_pDirectInput)
	{
		vdelete m_pDirectInput;
		m_pDirectInput = nullptr;
	}
}

void Input_ps::RegisterDeviceChange()
{
	m_xboxPad.TryReconnect();
}

void Input_ps::RegisterGamepad(IGamepad* pGamepad)
{
	if (m_uExternalPads < MAX_EXTERNAL_PADS)
	{
		m_pExternalPads[m_uExternalPads++] = pGamepad;
	}
}

void Input_ps::Update(GFXDevice* pDevice)
{
	m_keyboard.Update();
	m_mouse.Update();

	m_mouseWheel = 0;
	memset(m_keysThisFrame, 0, KEYBOARD_KEY_COUNT*sizeof(bool));
	memset(m_mouseButtonsThisFrame, 0, MOUSE_BUTTON_COUNT*sizeof(bool));
	m_uInputChars = 0;
}

void Input_ps::RegisterKeyStateChange(uint8 keyDown, bool bDown)
{
	m_keysDown[keyDown] = bDown;
	m_keysThisFrame[keyDown] |= bDown;
}

void Input_ps::RegisterMouseButtonChange(uint32 uButton, bool bValue)
{
	m_mouseButtonsDown[uButton] = bValue;
	m_mouseButtonsThisFrame[uButton] |= bValue;
}

void Input_ps::RegisterInputChar(char16 uChar)
{
	if(m_uInputChars < Keyboard::MAX_INPUT_CHARS)
	{
		m_inputChars[m_uInputChars] = uChar;
		m_uInputChars++;
	}
}

void Input_ps::RegisterHwnd(uint32 uDisplay, HWND hwnd)
{
	m_mouse.SetHWND(hwnd);
}


}
