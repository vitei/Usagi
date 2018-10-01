/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Maths/MathUtil.h"
#include "Engine/HID/InputStructs.h"
#include "DirectInput.h"
#include "DirectInputJoystick.h"

namespace usg{



DirectInputJoystick::DirectInputJoystick()
{
	m_uInputId = 0;
}

DirectInputJoystick::~DirectInputJoystick()
{
}

void DirectInputJoystick::Init(DirectInput* pInput, uint32 uPadNum)
{
	m_uInputId = uPadNum;
	TryReconnect(pInput);
}


void DirectInputJoystick::TryReconnect(DirectInput* pInput)
{
	m_bConnected = pInput->IsDeviceConnected(m_uInputId);
}

void DirectInputJoystick::Update(GFXDevice* pDevice, GamepadDeviceState& deviceStateOut)
{

}



}