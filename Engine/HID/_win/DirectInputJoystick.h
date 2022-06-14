/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Gamepads and joysticks which do not support xinput
*****************************************************************************/
#ifndef __USG_HID_PC_DIRECTINPUT_JOYSTICK_H__
#define __USG_HID_PC_DIRECTINPUT_JOYSTICK_H__

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
	virtual const char* GetProductName() const { return m_name.c_str(); }
	virtual void Update(GFXDevice* pDevice, GamepadDeviceState& deviceStateOut) override;
	void TryReconnect(DirectInput* pInput);
	virtual const char* GetModuleName() const { return "DirectInputJoystick"; }

private:
	void SetDeadzone(float fDeadZone);
	float GetAxis(DIJOYSTATE2& js, int iAxis);
	bool IsPovInRange(DWORD povVal, DWORD targVal);
	void GetPovData(DWORD pov, bool& bUp, bool& bRight, bool& bDown, bool& bleft, float& fOut);

	struct Range
	{
		long min;
		long max;
	};

	Range					m_axisRanges[GAMEPAD_AXIS_NONE];
	LPDIRECTINPUTDEVICE8	m_pDevice;
	uint32					m_uInputId;
	uint32					m_uNumButtons;
	uint32					m_uNumAxes;
	uint32					m_uCaps;
	usg::string				m_name;
	bool					m_bConnected;
	bool					m_bIsGamepad;

};
 
}

#endif