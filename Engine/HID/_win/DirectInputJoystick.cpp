/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Maths/MathUtil.h"
#include "Engine/HID/InputStructs.h"
#include "DirectInput.h"
#include "DirectInputJoystick.h"
#include <winerror.h>

namespace usg{

	struct ButtonMapping
	{
		uint32	uAbstractID;
		long	uDirectInputId;
		float	fSign;
	};

	struct ControlMapping
	{
		uint32	uAbstractID;
		uint32	uDirectInputId;
	};

	static const ButtonMapping g_axisMappingPad[] =
	{
		{ GAMEPAD_AXIS_LEFT_X,			DIJOFS_X, 1.0f },
		{ GAMEPAD_AXIS_LEFT_Y,			DIJOFS_Y, -1.0f },
		{ GAMEPAD_AXIS_RIGHT_X,			DIJOFS_Z, 1.0f },
		{ GAMEPAD_AXIS_RIGHT_Y,			DIJOFS_RZ, -1.0f },
		{ GAMEPAD_AXIS_NONE,		0 }
	};

	static const ButtonMapping g_axisMappingJoy[] =
	{
		{ JOYSTICK_AXIS_STICK_X,			DIJOFS_X, 1.0f },
		{ JOYSTICK_AXIS_STICK_Y,			DIJOFS_Y, -1.0f },
		{ JOYSTICK_AXIS_STICK_Z,			DIJOFS_Z, 1.0f },
		{ JOYSTICK_AXIS_STICK_RX,			DIJOFS_RX, 1.0f },
		{ JOYSTICK_AXIS_STICK_RY,			DIJOFS_RY, 1.0f },
		{ JOYSTICK_AXIS_STICK_RZ,			DIJOFS_RZ, 1.0f },
		{ JOYSTICK_AXIS_STICK_SLIDER_1,		DIJOFS_SLIDER(0), 1.0f},
		{ JOYSTICK_AXIS_STICK_SLIDER_2,		DIJOFS_SLIDER(1), 1.0f},
		{ GAMEPAD_AXIS_NONE,				0 }
	};


	static const ControlMapping g_controlMappingPad[] =
	{
		{ GAMEPAD_BUTTON_X,			3 },
		{ GAMEPAD_BUTTON_Y,			0 },
		{ GAMEPAD_BUTTON_A,			2 },
		{ GAMEPAD_BUTTON_B,			1 },
		{ GAMEPAD_BUTTON_L,			4 },
		{ GAMEPAD_BUTTON_R,			5 },
		{ GAMEPAD_BUTTON_START,		9 },
		{ GAMEPAD_BUTTON_SELECT,	8 },
		{ GAMEPAD_BUTTON_THUMB_L,	10 },
		{ GAMEPAD_BUTTON_THUMB_R,	11 },
		{ GAMEPAD_BUTTON_ZL,		6 },
		{ GAMEPAD_BUTTON_ZR,		7 },
		{ GAMEPAD_BUTTON_HOME,		12 },
		{ GAMEPAD_BUTTON_NONE,	0 }
	};

DirectInputJoystick::DirectInputJoystick()
{
	m_uInputId = 0;
	m_uCaps = 0;
	m_uNumButtons = 0;
	m_bHasPovDpad = false;
	m_uNumAxes = 0;
	m_bIsGamepad = false;
	m_bConnected = false;
	m_pDevice = nullptr;
}

DirectInputJoystick::~DirectInputJoystick()
{
	if (m_pDevice)
	{
		m_pDevice->Unacquire();
		m_pDevice->Release();
		m_pDevice = nullptr;
	}
}

void DirectInputJoystick::Init(DirectInput* pInput, uint32 uPadNum)
{
	m_uInputId = uPadNum;
	TryReconnect(pInput);
}


void DirectInputJoystick::TryReconnect(DirectInput* pInput)
{
	if (m_bConnected != pInput->IsDeviceConnected(m_uInputId))
	{
		m_bConnected = false;
		m_bIsGamepad = pInput->IsGamepad(m_uInputId);

		if (pInput->IsThrottle(m_uInputId))
		{
			m_uCaps |= CAP_HOTAS_THROTTLE;
		}
		
		if (m_bIsGamepad)
		{
			m_uCaps |= CAP_GAMEPAD;
		}

		if (m_pDevice)
		{
			m_pDevice->Unacquire();
			m_pDevice->Release();
			m_pDevice = nullptr;
		}

		//Try to create the device
		if (FAILED(pInput->GetDirectInput()->CreateDevice(pInput->GetGUIDForDevice(m_uInputId), &m_pDevice, NULL)))
		{
			return;
		}

		//Set the data format
		if (FAILED(m_pDevice->SetDataFormat(&c_dfDIJoystick2)))
		{
			return;
		}

		if (FAILED(m_pDevice->SetCooperativeLevel(pInput->GetWindow(), DISCL_FOREGROUND | DISCL_EXCLUSIVE)))
		{
			return;
		}

		m_pDevice->Acquire();

		DIDEVCAPS diCaps;
		diCaps.dwSize = sizeof(DIDEVCAPS);
		HRESULT hr = m_pDevice->GetCapabilities(&diCaps);
		if ( !FAILED(hr) )
		{
			if (diCaps.dwAxes > 1)
			{
				m_uCaps |= CAP_LEFT_STICK;
			}
			if (diCaps.dwAxes > 3)
			{
				m_uCaps |= CAP_RIGHT_STICK;
			}

			m_bHasPovDpad = false;
			if(!m_bIsGamepad)
			{
				if (diCaps.dwPOVs > 0)
				{
					m_uCaps |= CAP_POV;
				}
				if (diCaps.dwPOVs > 1)
				{
					m_uCaps |= CAP_POV2;
				}
				if (diCaps.dwPOVs > 2)
				{
					m_uCaps |= CAP_POV3;
				}
				if (diCaps.dwPOVs > 3)
				{
					m_uCaps |= CAP_POV4;
				}
			}
			else if (diCaps.dwPOVs > 0)
			{
				m_bHasPovDpad = true;
			}
			
			m_uNumAxes = diCaps.dwAxes;
			m_uNumButtons = diCaps.dwButtons;
		}

		if ((m_uCaps & (CAP_HOTAS_THROTTLE | CAP_GAMEPAD)) == 0)
		{
			// Assume we are joystick
			m_uCaps |= CAP_JOYSTICK;
		}

		SetDeadzone(0.05f);


		m_bConnected = true;
		m_name = pInput->GetName(m_uInputId);
	}
}

void DirectInputJoystick::SetDeadzone(float fDeadZone)
{
	DIPROPDWORD DIpdw;
	DIpdw.diph.dwSize = sizeof(DIPROPDWORD);
	DIpdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
	DIpdw.diph.dwHow = DIPH_BYOFFSET;

	DIpdw.dwData = (long)(10000 * fDeadZone);

	DIpdw.diph.dwObj = DIJOFS_X;
	m_pDevice->SetProperty(DIPROP_DEADZONE, &DIpdw.diph);

	DIpdw.diph.dwObj = DIJOFS_Y;
	m_pDevice->SetProperty(DIPROP_DEADZONE, &DIpdw.diph);

	DIpdw.diph.dwObj = DIJOFS_Z;
	m_pDevice->SetProperty(DIPROP_DEADZONE, &DIpdw.diph);

	DIpdw.diph.dwObj = DIJOFS_RX;
	m_pDevice->SetProperty(DIPROP_DEADZONE, &DIpdw.diph);

	DIpdw.diph.dwObj = DIJOFS_RY;
	m_pDevice->SetProperty(DIPROP_DEADZONE, &DIpdw.diph);

	DIpdw.diph.dwObj = DIJOFS_RZ;
	m_pDevice->SetProperty(DIPROP_DEADZONE, &DIpdw.diph);

	DIpdw.diph.dwObj = DIJOFS_SLIDER(0);
	m_pDevice->SetProperty(DIPROP_DEADZONE, &DIpdw.diph);

	DIpdw.diph.dwObj = DIJOFS_SLIDER(1);
	m_pDevice->SetProperty(DIPROP_DEADZONE, &DIpdw.diph);

	DIPROPRANGE range;
	range.diph.dwSize = sizeof(DIPROPRANGE);
	range.diph.dwHeaderSize = sizeof(DIPROPHEADER);
	range.diph.dwHow = DIPH_BYOFFSET;


	for (int i=0; i< GAMEPAD_AXIS_NONE; i++)
	{
		if (m_bIsGamepad)
		{
			if (g_axisMappingPad[i].uAbstractID == GAMEPAD_AXIS_NONE)
				break;

			range.diph.dwObj = g_axisMappingPad[i].uDirectInputId;
		}
		else
		{
			if (g_axisMappingJoy[i].uAbstractID == GAMEPAD_AXIS_NONE)
				break;

			range.diph.dwObj = g_axisMappingJoy[i].uDirectInputId;
		}
		range.lMin = -(1 << 7);
		range.lMax = (1 << 7);
		if (m_pDevice->GetProperty(DIPROP_RANGE, &range.diph) == DI_OK)
		{
			m_axisRanges[i].min = range.lMin;
			m_axisRanges[i].max = range.lMax;
		}
	}
} 

bool DirectInputJoystick::IsPovInRange(DWORD povVal, DWORD targVal)
{
	const int povButtonRange = 2000;

	if (targVal < povButtonRange)
	{
		if (povVal <= (targVal + povButtonRange))
		{
			return true;
		}
		DWORD remainder = povButtonRange - targVal;
		DWORD wrap = 36000 - remainder;
		if (povVal >= wrap)
		{
			return true;
		}
	}
	else
	{
		if (povVal >= (targVal - povButtonRange)
			&& povVal <= (targVal + povButtonRange))
		{
			return true;
		}
	}

	return false;
}

void DirectInputJoystick::Update(GFXDevice* pDevice, GamepadDeviceState& deviceStateOut)
{
	deviceStateOut.ClearButtons();

	DIJOYSTATE2 js;

	m_pDevice->Poll();

	DWORD size = sizeof(DIJOYSTATE2);

	HRESULT hr;
	hr = m_pDevice->GetDeviceState(size, &js);

	if (hr == DIERR_INVALIDPARAM)
	{
		DWORD size = sizeof(DIJOYSTATE);
		// The first part of the struct matches
		memset(&js, 0, sizeof(DIJOYSTATE2));

		hr = m_pDevice->GetDeviceState(size, &js);
	}

	if ((hr == DIERR_NOTACQUIRED) || (hr == DIERR_INPUTLOST))
	{
		hr = m_pDevice->Acquire();

		if (SUCCEEDED(hr))
		{
			hr = m_pDevice->GetDeviceState(size, &js);
		}
	}

	if (hr == DIERR_OTHERAPPHASPRIO)
	{
		// Not ours to poll
		return;
	}

	if (!SUCCEEDED(hr))
	{
		return;
	}

	if (m_bIsGamepad)
	{
		const ControlMapping* pMapping = g_controlMappingPad;
		while(pMapping->uAbstractID != GAMEPAD_BUTTON_NONE)
		{
			if (js.rgbButtons[pMapping->uDirectInputId] & 0x80)
			{
				deviceStateOut.SetButton(pMapping->uAbstractID, true);
			}
			pMapping++;
		}
	}
	else
	{
		for (uint32 i = 0; i < m_uNumButtons; i++)
		{
			if (js.rgbButtons[i] & 0x80)
			{
				deviceStateOut.SetButton(i+1, true);
			}
		}
	}

	bool bLeft, bRight, bUp, bDown;
	float fPovAxis;
	

	if (m_bIsGamepad)
	{
		for (int i = 0; i < GAMEPAD_AXIS_NONE; i++)
		{
			if (g_axisMappingPad[i].uAbstractID == GAMEPAD_AXIS_NONE)
				break;

			deviceStateOut.fAxisValues[g_axisMappingPad[i].uAbstractID] = GetAxis(js, i);

		}

		if (m_bHasPovDpad)
		{
			GetPovData(js.rgdwPOV[0], bUp, bRight, bDown, bLeft, fPovAxis);

			deviceStateOut.SetButton(GAMEPAD_BUTTON_UP, bUp);
			deviceStateOut.SetButton(GAMEPAD_BUTTON_RIGHT, bRight);
			deviceStateOut.SetButton(GAMEPAD_BUTTON_DOWN, bDown);
			deviceStateOut.SetButton(GAMEPAD_BUTTON_LEFT, bLeft);
		}
	
	}
	else
	{
		const ButtonMapping* pMapping = g_axisMappingJoy;
		for (int i = 0; i < GAMEPAD_AXIS_NONE; i++)
		{
			if (pMapping[i].uAbstractID == GAMEPAD_AXIS_NONE)
				break;

			deviceStateOut.fAxisValues[pMapping[i].uAbstractID] = GetAxis(js, i);

		}

		if (m_uCaps & CAP_POV)
		{
			GetPovData(js.rgdwPOV[0], bUp, bRight, bDown, bLeft, fPovAxis);

			deviceStateOut.fAxisValues[JOYSTICK_AXIS_POV_ANGLE] = fPovAxis;

			deviceStateOut.SetButton(JOYSTICK_BUTTON_POV_UP, bUp);
			deviceStateOut.SetButton(JOYSTICK_BUTTON_POV_RIGHT, bRight);
			deviceStateOut.SetButton(JOYSTICK_BUTTON_POV_DOWN, bDown);
			deviceStateOut.SetButton(JOYSTICK_BUTTON_POV_LEFT, bLeft);			
		}

		if (m_uCaps & CAP_POV2)
		{
			GetPovData(js.rgdwPOV[1], bUp, bRight, bDown, bLeft, fPovAxis);

			deviceStateOut.fAxisValues[JOYSTICK_AXIS_POV2_ANGLE] = fPovAxis;

			deviceStateOut.SetButton(JOYSTICK_BUTTON_POV2_UP, bUp);
			deviceStateOut.SetButton(JOYSTICK_BUTTON_POV2_RIGHT, bRight);
			deviceStateOut.SetButton(JOYSTICK_BUTTON_POV2_DOWN, bDown);
			deviceStateOut.SetButton(JOYSTICK_BUTTON_POV2_LEFT, bLeft);
		}

		if (m_uCaps & CAP_POV3)
		{
			GetPovData(js.rgdwPOV[2], bUp, bRight, bDown, bLeft, fPovAxis);

			deviceStateOut.fAxisValues[JOYSTICK_AXIS_POV3_ANGLE] = fPovAxis;

			deviceStateOut.SetButton(JOYSTICK_BUTTON_POV3_UP, bUp);
			deviceStateOut.SetButton(JOYSTICK_BUTTON_POV3_RIGHT, bRight);
			deviceStateOut.SetButton(JOYSTICK_BUTTON_POV3_DOWN, bDown);
			deviceStateOut.SetButton(JOYSTICK_BUTTON_POV3_LEFT, bLeft);
		}

		if (m_uCaps & CAP_POV4)
		{
			GetPovData(js.rgdwPOV[3], bUp, bRight, bDown, bLeft, fPovAxis);

			deviceStateOut.fAxisValues[JOYSTICK_AXIS_POV4_ANGLE] = fPovAxis;

			deviceStateOut.SetButton(JOYSTICK_BUTTON_POV4_UP, bUp);
			deviceStateOut.SetButton(JOYSTICK_BUTTON_POV4_RIGHT, bRight);
			deviceStateOut.SetButton(JOYSTICK_BUTTON_POV4_DOWN, bDown);
			deviceStateOut.SetButton(JOYSTICK_BUTTON_POV4_LEFT, bLeft);
		}

	}
}

void DirectInputJoystick::GetPovData(DWORD pov, bool& bUp, bool& bRight, bool& bDown, bool& bLeft, float& fOut)
{
	bUp = bRight = bDown = bLeft = false; 
	fOut = -1.0f;

	if (pov & 0xf0000000)
	{
		fOut = -1.0f;
	}
	else
	{
		fOut = ((float)pov) / 100.f;

		if (IsPovInRange(pov, 0))
		{
			bUp = true;
		}
		else if (IsPovInRange(pov, 9000))
		{
			bRight = true;
		}
		else if (IsPovInRange(pov, 18000))
		{
			bDown = true;
		}
		else if (IsPovInRange(pov, 27000))
		{
			bLeft = true;
		}
	}
}

float DirectInputJoystick::GetAxis(DIJOYSTATE2& js, int iAxis)
{
	if(m_axisRanges[iAxis].max == m_axisRanges[iAxis].min)
		return 0.0f;
	const ButtonMapping* pMapping = m_bIsGamepad ? &g_axisMappingPad[iAxis] : &g_axisMappingJoy[iAxis];
	long value = *((long*)((uint8*)(&js) + pMapping->uDirectInputId));
	float range = (float)m_axisRanges[iAxis].max - (float)m_axisRanges[iAxis].min;

	float fNormalised = ((float)value - (float)m_axisRanges[iAxis].min) / range;
	return (fNormalised - 0.5f) * 2.f * pMapping->fSign;
}

uint32 DirectInputJoystick::GetCaps() const
{
	return m_uCaps;
}



}