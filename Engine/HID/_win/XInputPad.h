/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: A gamepad interface which actually uses the mouse and keyboard
*****************************************************************************/
#ifndef __USG_HID_PC_XINPUT_PAD_H__
#define __USG_HID_PC_XINPUT_PAD_H__
#include "Engine/Common/Common.h"
#include "Engine/HID/IGamepad.h"
#include <XInput.h>

namespace usg{

class Input_ps;

class XInputPad : public IGamepad
{
public:
	XInputPad();
	~XInputPad();

	void Init(uint32 uPadNum);
	virtual uint32 GetCaps() const override { return CAP_LEFT_STICK | CAP_RIGHT_STICK; }
	virtual bool IsConnected() const { return m_bConnected; }
	virtual void Update(GFXDevice* pDevice, GamepadDeviceState& deviceStateOut) override;
	void TryReconnect();
	virtual const char* GetModuleName() const { return "XInputPad"; }

private:
	float GetAxisWithDeadZone(SHORT value, SHORT deadzone);
	// Call with zero to cease vibration
	void Vibrate(int leftVal, int rightVal);

	bool			m_bConnected;
	uint32			m_xinputID;
	XINPUT_STATE	m_controllerState;
};
 
}

#endif