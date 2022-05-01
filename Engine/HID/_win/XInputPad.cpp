/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Maths/MathUtil.h"
#include "Engine/HID/InputStructs.h"
#include "XInputPad.h"

namespace usg{

const SHORT g_sDeadZone = 6000;

struct AxisMapping
{
	uint32	uAbstractID;
	uint32	uXInputID;
};

static const AxisMapping g_axisMappingPad[] =
{
	{ GAMEPAD_BUTTON_A,			XINPUT_GAMEPAD_B },
	{ GAMEPAD_BUTTON_B,			XINPUT_GAMEPAD_A },
	{ GAMEPAD_BUTTON_X,			XINPUT_GAMEPAD_Y },
	{ GAMEPAD_BUTTON_Y,			XINPUT_GAMEPAD_X },
	{ GAMEPAD_BUTTON_L,			XINPUT_GAMEPAD_LEFT_SHOULDER },
	{ GAMEPAD_BUTTON_R,			XINPUT_GAMEPAD_RIGHT_SHOULDER },
	{ GAMEPAD_BUTTON_UP,		XINPUT_GAMEPAD_DPAD_UP },
	{ GAMEPAD_BUTTON_DOWN,		XINPUT_GAMEPAD_DPAD_DOWN },
	{ GAMEPAD_BUTTON_LEFT,		XINPUT_GAMEPAD_DPAD_LEFT },
	{ GAMEPAD_BUTTON_RIGHT,		XINPUT_GAMEPAD_DPAD_RIGHT },
	{ GAMEPAD_BUTTON_START,		XINPUT_GAMEPAD_START },
	{ GAMEPAD_BUTTON_SELECT,	XINPUT_GAMEPAD_BACK },
	{ GAMEPAD_BUTTON_THUMB_L,	XINPUT_GAMEPAD_LEFT_THUMB },
	{ GAMEPAD_BUTTON_THUMB_R,	XINPUT_GAMEPAD_RIGHT_THUMB },
	{ GAMEPAD_BUTTON_HOME,		XINPUT_GAMEPAD_START },
	{ GAMEPAD_BUTTON_NONE,		0 }
};

XInputPad::XInputPad()
{
    m_xinputID = 0;

    m_bConnected = false;
}

XInputPad::~XInputPad()
{
}

void XInputPad::Init(uint32 uPadNum)
{
	m_xinputID = uPadNum;
	TryReconnect();
}

float XInputPad::GetAxisWithDeadZone(SHORT value, SHORT deadzone)
{
	if(value > deadzone || value < -deadzone)
	{
		float fSign = Math::Sign(value);
		float fDead = fSign * deadzone;
		return ((float)value - fDead)/(32768.f - fDead);
	}
	
	return 0.0f;
}

void XInputPad::TryReconnect()
{
	DWORD result = XInputGetState(m_xinputID, &m_controllerState);
	// Shouldn't really be an assert, valid for the pad to not be connected
	m_bConnected = (result == ERROR_SUCCESS);
}

void XInputPad::Update(GFXDevice* pDevice, GamepadDeviceState& deviceStateOut)
{
	deviceStateOut.uButtonsPrevDown = deviceStateOut.uButtonsDown;
	deviceStateOut.uButtonsDown = 0;
    // Zeroise the state
    ZeroMemory(&m_controllerState, sizeof(XINPUT_STATE));

    // Get the state
    DWORD result = XInputGetState(m_xinputID, &m_controllerState);
	// Shouldn't really be an assert, valid for the pad to not be connected
//    ASSERT(result == ERROR_SUCCESS);

	deviceStateOut.fAxisValues[GAMEPAD_AXIS_LEFT_X] = GetAxisWithDeadZone(m_controllerState.Gamepad.sThumbLX, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
	deviceStateOut.fAxisValues[GAMEPAD_AXIS_LEFT_Y] = GetAxisWithDeadZone(m_controllerState.Gamepad.sThumbLY, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
	deviceStateOut.fAxisValues[GAMEPAD_AXIS_RIGHT_X] = GetAxisWithDeadZone(m_controllerState.Gamepad.sThumbRX, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
	deviceStateOut.fAxisValues[GAMEPAD_AXIS_RIGHT_Y] = GetAxisWithDeadZone(m_controllerState.Gamepad.sThumbRY, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);

	const AxisMapping *pMapping = g_axisMappingPad;
	while(pMapping->uAbstractID != GAMEPAD_BUTTON_NONE)
	{
		if(pMapping->uXInputID & m_controllerState.Gamepad.wButtons)
		{
			deviceStateOut.uButtonsDown |= (1 << (pMapping->uAbstractID - 1));
		}
		pMapping++;
	}

	if(m_controllerState.Gamepad.bLeftTrigger)
		deviceStateOut.uButtonsDown |= (1 << (GAMEPAD_BUTTON_ZL - 1));

	if(m_controllerState.Gamepad.bRightTrigger)
		deviceStateOut.uButtonsDown |= (1 << (GAMEPAD_BUTTON_ZR - 1));
}

void XInputPad::VibrateInt(int leftVal, int rightVal)
{
    // Create a Vibraton State
    XINPUT_VIBRATION Vibration;

    ZeroMemory(&Vibration, sizeof(XINPUT_VIBRATION));

    Vibration.wLeftMotorSpeed = leftVal;
    Vibration.wRightMotorSpeed = rightVal;

    // Vibrate the controller
    XInputSetState(m_xinputID, &Vibration);
}


void XInputPad::Vibrate(float fLeft, float fRight)
{
	int iLeft = (int)(fLeft * 65535.f);
	int iRight = (int)(fRight * 65535.f);

	VibrateInt(iLeft, iRight);
}

}