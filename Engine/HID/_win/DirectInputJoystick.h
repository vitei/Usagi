/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: A gamepad interface which actually uses the mouse and keyboard
*****************************************************************************/
#ifndef __USG_HID_PC_DIRECTINPUT_JOYSTICK_H__
#define __USG_HID_PC_DIRECTINPUT_JOYSTICK_H__
#include "Engine/Common/Common.h"
#include "Engine/HID/IGamepad.h"
#include <dinput.h>

namespace usg{

class Input_ps;
class DirectInput;

class DirectInputJoystick : public IGamepad
{
public:
	DirectInputJoystick();
	~DirectInputJoystick();

	void Init(DirectInput* pInput, uint32 uDeviceNum);
	virtual uint32 GetCaps() const override;
	virtual bool IsConnected() const { return m_bConnected; }
	virtual void Update(GFXDevice* pDevice, GamepadDeviceState& deviceStateOut) override;
	void TryReconnect(DirectInput* pInput);

private:
	
	uint32			m_uInputId;
	bool			m_bConnected;

};
 
}

#endif