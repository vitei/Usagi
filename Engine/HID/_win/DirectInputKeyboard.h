/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Platform specific version of a keyboard. A bit shite for now
*****************************************************************************/
#pragma once

#include "Engine/HID/Gamepad.h"
#include "Engine/HID/Keyboard.h"
#include "Engine/Core/stl/map.h"

namespace usg{

class Input_ps;

class DirectInputKeyboard : public Keyboard
{
public:
	DirectInputKeyboard() {}
	~DirectInputKeyboard() {}
	// FIXME: Shutdown code is we adopt DI

	bool Init(Input_ps* pOwner, DirectInput* pInput, uint32 uDeviceNum = 0);
	void Update();

private:
	const Input_ps*			m_pOwner;
	DirectInput*			m_pInput;
	LPDIRECTINPUTDEVICE8	m_pDevice;
	HANDLE					m_hEvent;
	RECT					m_cage;
};

}
