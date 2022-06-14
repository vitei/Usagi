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

	struct AxisMapping
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

	static const AxisMapping g_axisMappingPad[] =
	{
		{ GAMEPAD_AXIS_LEFT_X,			DIJOFS_X, 1.0f },
		{ GAMEPAD_AXIS_LEFT_Y,			DIJOFS_Y, -1.0f },
		{ GAMEPAD_AXIS_RIGHT_X,			DIJOFS_Z, 1.0f },
		{ GAMEPAD_AXIS_RIGHT_Y,			DIJOFS_RZ, -1.0f },
		{ GAMEPAD_AXIS_NONE,		0 }
	};

	static const AxisMapping g_axisMappingJoy[] =
	{
		{ GAMEPAD_AXIS_LEFT_X,			DIJOFS_X, 1.0f },
		{ GAMEPAD_AXIS_LEFT_Y,			DIJOFS_Y, -1.0f },
		{ GAMEPAD_AXIS_RIGHT_X,			DIJOFS_RZ, 1.0f },
		{ GAMEPAD_AXIS_RIGHT_Y,			DIJOFS_SLIDER(0), -1.0f },
		{ GAMEPAD_AXIS_NONE,		0 }
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

		if (FAILED(m_pDevice->SetCooperativeLevel(pInput->GetWindow(), DISCL_BACKGROUND | DISCL_EXCLUSIVE)))
		{
			return;
		}

		m_pDevice->Acquire();
		SetDeadzone(0.05f);

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
			if (diCaps.dwPOVs > 0)
			{
				m_uCaps |= CAP_POV;
			}
			if (diCaps.dwPOVs > 1)
			{
				m_uCaps |= CAP_POV2;
			}
			m_uNumAxes = diCaps.dwAxes;
			m_uNumButtons = diCaps.dwButtons;
		}
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
	deviceStateOut.uButtonsPrevDown = deviceStateOut.uButtonsDown;
	deviceStateOut.uButtonsDown = 0;

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
				deviceStateOut.uButtonsDown |= (1 << (pMapping->uAbstractID -1));
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
				deviceStateOut.uButtonsDown |= (1 << i);
			}
		}
	}
	

	if (m_bIsGamepad)
	{
		for (int i = 0; i < GAMEPAD_AXIS_NONE; i++)
		{
			if (g_axisMappingPad[i].uAbstractID == GAMEPAD_AXIS_NONE)
				break;

			deviceStateOut.fAxisValues[g_axisMappingPad[i].uAbstractID] = GetAxis(js, i);

		}
	}
	else
	{
		for (int i = 0; i < GAMEPAD_AXIS_NONE; i++)
		{
			if (g_axisMappingJoy[i].uAbstractID == GAMEPAD_AXIS_NONE)
				break;

			deviceStateOut.fAxisValues[g_axisMappingJoy[i].uAbstractID] = GetAxis(js, i);

		}

		bool bLeft, bRight, bUp, bDown;
		float fPovAxis;

		if (m_uCaps & CAP_POV)
		{
			GetPovData(js.rgdwPOV[0], bLeft, bRight, bUp, bDown, fPovAxis);

			deviceStateOut.fAxisValues[GAMEPAD_AXIS_POV_ANGLE] = fPovAxis;

			deviceStateOut.uButtonsDown |= bUp ? JOYSTICK_BUTTON_POV_UP : 0;
			deviceStateOut.uButtonsDown |= bRight ? JOYSTICK_BUTTON_POV_RIGHT : 0;
			deviceStateOut.uButtonsDown |= bDown ? JOYSTICK_BUTTON_POV_DOWN : 0;
			deviceStateOut.uButtonsDown |= bLeft ? JOYSTICK_BUTTON_POV_LEFT : 0;
		}

		if (m_uCaps & CAP_POV2)
		{
			GetPovData(js.rgdwPOV[1], bLeft, bRight, bUp, bDown, fPovAxis);

			deviceStateOut.fAxisValues[GAMEPAD_AXIS_POV2_ANGLE] = fPovAxis;

			deviceStateOut.uButtonsDown |= bUp ? JOYSTICK_BUTTON_POV2_UP : 0;
			deviceStateOut.uButtonsDown |= bRight ? JOYSTICK_BUTTON_POV2_RIGHT : 0;
			deviceStateOut.uButtonsDown |= bDown ? JOYSTICK_BUTTON_POV2_DOWN : 0;
			deviceStateOut.uButtonsDown |= bLeft ? JOYSTICK_BUTTON_POV2_LEFT : 0;
		}

		if (m_uCaps & CAP_POV3)
		{
			GetPovData(js.rgdwPOV[1], bLeft, bRight, bUp, bDown, fPovAxis);

			deviceStateOut.fAxisValues[GAMEPAD_AXIS_POV3_ANGLE] = fPovAxis;

			deviceStateOut.uButtonsDown |= bUp ? JOYSTICK_BUTTON_POV3_UP : 0;
			deviceStateOut.uButtonsDown |= bRight ? JOYSTICK_BUTTON_POV3_RIGHT : 0;
			deviceStateOut.uButtonsDown |= bDown ? JOYSTICK_BUTTON_POV3_DOWN : 0;
			deviceStateOut.uButtonsDown |= bLeft ? JOYSTICK_BUTTON_POV3_LEFT : 0;
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
	const AxisMapping* pMapping = m_bIsGamepad ? &g_axisMappingPad[iAxis] : &g_axisMappingJoy[iAxis];
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