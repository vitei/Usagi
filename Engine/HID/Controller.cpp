/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Base class for controllers (e.g. menu and player avatar) allowing
//	easy mapping of various input devices such as joystick, mouse, keyboard
*****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Core/Utility.h"
#include "Engine/Maths/MathUtil.h"
#include "Engine/HID/Keyboard.h"
#include "Engine/HID/Mouse.h"
#include "Controller.h"
#include "Input.h"

namespace usg{


bool Controller::MappingOutput::GetBool() const
{ 
	switch(eOutput)
	{
	case OUTPUT_TYPE_BOOL:
		return data.bValue;
	case OUTPUT_TYPE_FLOAT:
		return data.fValue > 0.5f;
	default:
		return false;
	}
}

float Controller::MappingOutput::GetFloat() const
{
	switch(eOutput)
	{
	case OUTPUT_TYPE_BOOL:
		return data.bValue == true ? 1.0f : 0.0f;
	case OUTPUT_TYPE_FLOAT:
		return data.fValue;
	default:
		return 0.0f;
	}
}

void Controller::MappingOutput::Clear(bool bForce)
{ 
	if(eOutput == OUTPUT_TYPE_BOOL)
		data.bValue = false;
	else if(eOutput == OUTPUT_TYPE_FLOAT)
	{
		if(bForce || (fStickyRate == 0.0f))
		{
			data.fValue = 0.0f;
		}
	}
}

Controller::Controller(uint32 uGamepadId):
m_uGamepadId(uGamepadId),
m_boolDeadZone(0.15f)
{
	m_pKeyboard = Input::GetKeyboard();
	m_pMouse = Input::GetMouse();
}

void Controller::SetGamepad(uint32 uGamepadId)
{
	m_uGamepadId = uGamepadId;
}

Controller::~Controller(void)
{
}


void Controller::ResetDetails()
{
	m_details.clear();
}

Gamepad* Controller::GetGamepad(uint32 uSubDevice) 
{
	return Input::GetGamepad(m_uGamepadId + uSubDevice);
}

void Controller::ClearMappingSet(MappingOutput* pOutputs, uint32 uCount)
{
	for(uint32 i=0; i<uCount; i++)
	{
		pOutputs[i].Clear();
	}
}

void Controller::Update( float timeDelta )
{
	m_sinceLastFrame = timeDelta;
	for(memsize i=0; i<m_details.size(); i++)
	{
		ControllerDetail& detail = m_details[i];
		MappingOutput* pOutput = detail.pResult;
		ASSERT(pOutput!=NULL);
		switch(pOutput->eOutput)
		{
		case OUTPUT_TYPE_BOOL:
			pOutput->data.bValue |= GetValueAsBool(detail);
			break;
		case OUTPUT_TYPE_FLOAT:
			{
				float fValue = GetValueAsFloat(detail);

				if(pOutput->fStickyRate != 0.0f)
				{
					pOutput->data.fValue += fValue;
					pOutput->data.fValue = Math::Clamp(pOutput->data.fValue, -1.0f, 1.0f);
				}
				else
				{
					if(fabsf(fValue) > fabsf(pOutput->data.fValue))
					{
						pOutput->data.fValue = fValue;
					}
				}
				break;

			}
		default:
			ASSERT(false);
		}
	}
}

bool Controller::IsToggleValidInt(ControllerDetail& detail)
{
	switch (detail.toggleType)
	{
	case INPUT_TYPE_NONE:
		return true;
	case INPUT_TYPE_KEY:
		return detail.bReverseToggle != m_pKeyboard->GetKey(detail.uInputToggle);
	case INPUT_TYPE_BUTTON:
	{
		Gamepad* pGamepad = GetGamepad(detail.uToggleSubDevice);
		return detail.bReverseToggle != (pGamepad && pGamepad->GetButtonDown(detail.uInputToggle, usg::BUTTON_STATE_HELD));
	}
	case INPUT_TYPE_MOUSE_BUTTON:
		return detail.bReverseToggle != m_pMouse->GetButton((MouseButton)detail.uInputToggle, BUTTON_STATE_HELD);
	default:
		ASSERT(false);
		return false;
	}
}

bool Controller::IsToggleValid(ControllerDetail& detail)
{
	// First iterate through the other details
	for (auto& itr : m_details)
	{
		if (&itr != &detail)
		{
			if (itr.uInputIdA == detail.uInputIdA)
			{
				if (itr.toggleType != INPUT_TYPE_NONE && (itr.deviceType == detail.deviceType) && (itr.uSubDevice == detail.uSubDevice)
					&& ((itr.uInputToggle != detail.uInputToggle) || detail.toggleType == INPUT_TYPE_NONE) )
				{
					// Block if another function that requires a toggle is active
					if (IsToggleValidInt(itr))
					{
						return false;
					}
				}
			}
		}
	}


	return IsToggleValidInt(detail);

}

bool Controller::GetValueAsBool( ControllerDetail &detail )
{
	if (!IsToggleValid(detail))
		return false;
	// Default to false if the controller detail is unset
	switch( detail.deviceType )
	{
	case INPUT_TYPE_AXIS:
	case INPUT_TYPE_MOUSE_AXIS:
		{
			float fOutput = 0.0f;
			if (detail.deviceType != INPUT_TYPE_MOUSE_AXIS)
			{
				Gamepad* pGamepad = GetGamepad(detail.uSubDevice);
				fOutput = pGamepad ? pGamepad->GetAxisValue(detail.uInputIdA) : 0.0f;
			}
			else
			{
				fOutput = m_pMouse->GetAxis((MouseAxis)detail.uInputIdA);
			}
			switch( detail.axisType )
			{
			case AXIS_TYPE_ABSOLUTE:
				return ( fOutput > ( -1.0f + m_boolDeadZone ) );
			case AXIS_TYPE_POSITIVE:
				return ( fOutput > m_boolDeadZone );
			case AXIS_TYPE_NEGATIVE:
				return ( fOutput < -m_boolDeadZone );
			case AXIS_TYPE_ABSOLUTE_TO_POSITIVE:
				return (fOutput > m_boolDeadZone);
			default:
				ASSERT(false);
			}
		}
		break;
	case INPUT_TYPE_BUTTON:
	{
		Gamepad* pGamepad = GetGamepad(detail.uSubDevice);

		return pGamepad ? pGamepad->GetButtonDown(detail.uInputIdA, detail.eInputState) : false;
	}
	case INPUT_TYPE_KEY:
		return m_pKeyboard ? m_pKeyboard->GetKey((uint8)detail.uInputIdA, detail.eInputState) : false;
	case INPUT_TYPE_MOUSE_BUTTON:
		return m_pMouse ? m_pMouse->GetButton((MouseButton)detail.uInputIdA, detail.eInputState) : false;
	case INPUT_TYPE_SCREEN_TOUCH:
		ASSERT(false); // Not yet supported
		return false;
	case INPUT_TYPE_NONE:
		return false;
	}

	return false;
}


float Controller::GetValueAsFloat( ControllerDetail &detail )
{
	if (!IsToggleValid(detail))
		return 0.0f;

	float fValue = detail.bReverse ? -1.0f : 1.0f;

	if(detail.pResult->fStickyRate != 0.0f)
	{
		fValue *= detail.pResult->fStickyRate;
	}



	// This case is easy if we're dealing with a button press
	switch( detail.deviceType )
	{
	case INPUT_TYPE_BUTTON:
		{
			Gamepad* pGamepad = GetGamepad(detail.uSubDevice);
			if(pGamepad)
			{
				switch( detail.axisType )
				{
					case AXIS_TYPE_ABSOLUTE:
						{
							float fOutput = pGamepad->GetButtonDown( detail.uInputIdB, BUTTON_STATE_HELD ) ? fValue : 0.0f;
							fOutput += pGamepad->GetButtonDown( detail.uInputIdA, BUTTON_STATE_HELD ) ? -fValue : 0.0f;
							return fOutput;
						}
					case AXIS_TYPE_POSITIVE:
					case AXIS_TYPE_ABSOLUTE_TO_POSITIVE:
						return pGamepad->GetButtonDown( detail.uInputIdA, BUTTON_STATE_HELD) ? 1.0f : 0.0f;
					case AXIS_TYPE_NEGATIVE:
						return pGamepad->GetButtonDown( detail.uInputIdA, BUTTON_STATE_HELD) ? -1.0f : 0.0f;
					default:
						ASSERT(false);
					break;
				}
			}
			return false;
		}
	case INPUT_TYPE_AXIS:
	case INPUT_TYPE_MOUSE_AXIS:
		{
			float fOutput = 0.0f;
			if (detail.deviceType == INPUT_TYPE_AXIS)
			{
				Gamepad* pGamepad = GetGamepad(detail.uSubDevice);
				if(pGamepad)
				{
					fOutput = pGamepad->GetAxisValue(detail.uInputIdA);
				}
			}
			else
			{
				fOutput = m_pMouse->GetAxis((MouseAxis)detail.uInputIdA);
			}

			switch( detail.axisType )
			{
			case AXIS_TYPE_ABSOLUTE:
				fOutput = fOutput * fValue;
				break;
			case AXIS_TYPE_POSITIVE:
				fOutput =  Math::Clamp( fOutput, 0.0f, 1.0f );
				break;
			case AXIS_TYPE_NEGATIVE:
				fOutput = Math::Abs(Math::Clamp( fOutput, -1.0f, 0.0f ));
				break;
			case AXIS_TYPE_ABSOLUTE_TO_POSITIVE:
				fOutput = ((fOutput * fValue) + 1.0f)/2.0f;
				break;
			default:
				ASSERT(false);
			}
			return fOutput;
		}
	case INPUT_TYPE_KEY:
	{
		if (!m_pKeyboard)
			return false;

		switch (detail.axisType)
		{
		case AXIS_TYPE_ABSOLUTE:
		{
			float fOutput = m_pKeyboard->GetKey((uint8)detail.uInputIdB, BUTTON_STATE_HELD) ? fValue : 0.0f;
			fOutput += m_pKeyboard->GetKey((uint8)detail.uInputIdA, BUTTON_STATE_HELD) ? -fValue : 0.0f;
			return fOutput;
		}
		case AXIS_TYPE_POSITIVE:
			return m_pKeyboard->GetKey((uint8)detail.uInputIdA, BUTTON_STATE_HELD) ? 1.0f : 0.0f;
		case AXIS_TYPE_NEGATIVE:
			return m_pKeyboard->GetKey((uint8)detail.uInputIdA, BUTTON_STATE_HELD) ? -1.0f : 0.0f;
		default:
			ASSERT(false);
			break;
		}
	}
	case INPUT_TYPE_MOUSE_BUTTON:
	{
		if (!m_pMouse)
			return 0.0f;

		switch (detail.axisType)
		{
		case AXIS_TYPE_ABSOLUTE:
		{
			float fOutput = m_pMouse->GetButton((MouseButton)detail.uInputIdB, BUTTON_STATE_HELD) ? fValue : 0.0f;
			fOutput += m_pMouse->GetButton((MouseButton)detail.uInputIdA, BUTTON_STATE_HELD) ? -fValue : 0.0f;
			return fOutput;
		}
		case AXIS_TYPE_POSITIVE:
			return m_pMouse->GetButton((MouseButton)detail.uInputIdA, BUTTON_STATE_HELD) ? 1.0f : 0.0f;
		case AXIS_TYPE_NEGATIVE:
			return m_pMouse->GetButton((MouseButton)detail.uInputIdA, BUTTON_STATE_HELD) ? -1.0f : 0.0f;
		default:
			ASSERT(false);
			break;
		}
	}
	default:
		//ASSERT(false);
		return 0.0f;
	}
}

bool Controller::CreateButtonMapping(uint32 uButton, MappingOutput& detailOut, ButtonState eInputState /*= BUTTON_STATE_PRESSED*/, uint32 uSubDevice /*= 0*/)
{
	ControllerDetail detail;
	detail.pResult		= &detailOut;
	detailOut.eOutput		= OUTPUT_TYPE_BOOL;
	detailOut.Clear();
	detail.deviceType	= INPUT_TYPE_BUTTON;
	detail.axisType		= AXIS_TYPE_POSITIVE;
	detail.uInputIdA	= uButton;
	detail.uInputIdB	= GAMEPAD_BUTTON_NONE;
	detail.eInputState  = eInputState;
	detail.uSubDevice	= uSubDevice;

	m_details.push_back(detail);

	return true;
}

bool Controller::CreateKeyMapping(uint8 uKey, MappingOutput& output, ButtonState eInputState)
{
	ControllerDetail detail;
	detail.pResult = &output;
	output.eOutput = OUTPUT_TYPE_BOOL;
	output.Clear();
	detail.deviceType = INPUT_TYPE_KEY;
	detail.axisType = AXIS_TYPE_POSITIVE;
	detail.uInputIdA = (uint32)uKey;
	detail.uInputIdB = 0;
	detail.eInputState = eInputState;

	m_details.push_back(detail);

	return true;
}


bool Controller::CreateMouseButtonMapping(MouseButton eButton, MappingOutput& output, ButtonState eInputState)
{
	ControllerDetail detail;
	detail.pResult = &output;
	output.eOutput = OUTPUT_TYPE_BOOL;
	output.Clear();
	detail.deviceType = INPUT_TYPE_MOUSE_BUTTON;
	detail.axisType = AXIS_TYPE_POSITIVE;
	detail.uInputIdA = (uint32)eButton;
	detail.uInputIdB = (uint32)MOUSE_BUTTON_NONE;
	detail.eInputState = eInputState;

	m_details.push_back(detail);

	return true;
}

bool Controller::CreateAxisMapping(uint32 uAxis, AxisType eType, MappingOutput &detailOut, float fStickyRate /*= 0.0f*/, bool bReverse /*= false*/, uint32 uDeviceIdx /*= 0*/)
{
	ControllerDetail detail;
	detail.pResult		= &detailOut;
	detail.pResult->fStickyRate = fStickyRate;
	detailOut.eOutput		= OUTPUT_TYPE_FLOAT;
	detailOut.Clear();
	detailOut.data.fValue = 0.0f;
	detail.deviceType	= INPUT_TYPE_AXIS;
	detail.axisType		= eType;
	detail.bReverse		= bReverse;
	detail.uInputIdA	= uAxis;
	detail.uSubDevice	= uDeviceIdx;
	detail.uInputIdB	= _GamepadAxis_count;

	m_details.push_back(detail);

	return true;
}

bool Controller::CreateMouseAxisMapping(MouseAxis uAxis, AxisType eType, MappingOutput &output, float fStickyRate, bool bReverse)
{
	ControllerDetail detail;
	detail.pResult = &output;
	detail.pResult->fStickyRate = fStickyRate;
	output.eOutput = OUTPUT_TYPE_FLOAT;
	output.Clear();
	output.data.fValue = 0.0f;
	detail.deviceType = INPUT_TYPE_MOUSE_AXIS;
	detail.axisType = eType;
	detail.bReverse = bReverse;
	detail.uInputIdA = uAxis;
	detail.uInputIdB = MOUSE_BUTTON_NONE;

	m_details.push_back(detail);

	return true;
}


bool Controller::CreateButtonFromAxis( uint32 uAxis,  AxisType eType, MappingOutput& detailOut )
{
	ControllerDetail detail;
	detail.pResult		= &detailOut;
	detailOut.eOutput		= OUTPUT_TYPE_BOOL;
	detailOut.Clear();

	detail.deviceType	= INPUT_TYPE_AXIS;
	detail.axisType		= eType;
	detail.uInputIdA	= uAxis;
	detail.uInputIdB	= _GamepadAxis_count;

	m_details.push_back(detail);

	return true;
}

bool Controller::CreateAxisFromButtonPair(uint32 uButtonA, uint32 uButtonB, MappingOutput& detailOut, float fStickyRate /*= 0.0f*/, bool bReverse /*= false*/, uint32 uSubDevice /*= 0*/)
{
	ControllerDetail detail;
	detail.pResult		= &detailOut;
	detail.pResult->fStickyRate = fStickyRate;
	detailOut.eOutput		= OUTPUT_TYPE_FLOAT;
	detailOut.Clear();
	detailOut.data.fValue = 0.0f;

	detail.deviceType	= INPUT_TYPE_BUTTON;
	detail.axisType		= AXIS_TYPE_ABSOLUTE;
	detail.bReverse		= bReverse;
	detail.uInputIdA	= uButtonA;
	detail.uInputIdB	= uButtonB;
	detail.uSubDevice	= uSubDevice;

	m_details.push_back(detail);

	return true;
}

bool Controller::CreateAxisFromKeyPair(uint8 uKeyA, uint8 uKeyB, MappingOutput& output, float fStickyRate, bool bReverse)
{
	ControllerDetail detail;
	detail.pResult = &output;
	detail.pResult->fStickyRate = fStickyRate;
	output.eOutput = OUTPUT_TYPE_FLOAT;
	output.Clear();
	output.data.fValue = 0.0f;

	detail.deviceType = INPUT_TYPE_KEY;
	detail.axisType = AXIS_TYPE_ABSOLUTE;
	detail.bReverse = bReverse;
	detail.uInputIdA = (uint32)uKeyA;
	detail.uInputIdB = (uint32)uKeyB;

	m_details.push_back(detail);

	return true;
}


void Controller::AddPadToggleToPrev(uint32 eButton, uint32 uToggleSubDevice, bool bReverseToggle)
{
	memsize index = m_details.size();
	ASSERT(index > 0);
	m_details[index - 1].toggleType = INPUT_TYPE_BUTTON;
	m_details[index - 1].uInputToggle = eButton;
	m_details[index - 1].bReverseToggle = bReverseToggle;
	m_details[index - 1].uToggleSubDevice = uToggleSubDevice;
}

void Controller::AddMouseToggleToPrev(MouseButton eButton, bool bReverseToggle)
{
	memsize index = m_details.size();
	ASSERT(index > 0);
	m_details[index - 1].toggleType = INPUT_TYPE_MOUSE_BUTTON;
	m_details[index - 1].uInputToggle = (uint32)eButton;
	m_details[index - 1].bReverseToggle = bReverseToggle;
	m_details[index - 1].uToggleSubDevice = 0;

}

void Controller::AddKeyToggleToPrev(uint8 uKey, bool bReverseToggle)
{
	memsize index = m_details.size();
	ASSERT(index > 0);
	m_details[index - 1].toggleType = INPUT_TYPE_KEY;
	m_details[index - 1].uInputToggle = (uint32)uKey;
	m_details[index - 1].bReverseToggle = bReverseToggle;
	m_details[index - 1].uToggleSubDevice = 0;

}


}
