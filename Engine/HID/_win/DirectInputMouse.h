/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: DirectInput version of the mouse, more responsive despite recommendations
*****************************************************************************/
#pragma once

#include "Engine/HID/Gamepad.h"
#include "Engine/HID/Mouse.h"

namespace usg{

class Input_ps;

class DirectInputMouse : public Mouse
{
public:
	DirectInputMouse() {}
	~DirectInputMouse() {}
	// FIXME: Shutdown code is we adopt DI

	bool Init(DirectInput* pInput, uint32 uDeviceNum = 0);

	void Update();
	bool Acquire();

private:
	DirectInput*			m_pInput = nullptr;
	LPDIRECTINPUTDEVICE8	m_pDevice = nullptr;
	DIDEVICEOBJECTDATA		m_objectData[64] = {};
	DWORD					m_objectDataSize = 0;
	HANDLE					m_hEvent;
	RECT					m_cage;
};

}

