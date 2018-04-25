/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Base class for controllers (e.g. menu and player avatar) allowing
//	easy mapping of various input devices such as joystick, mouse, keyboard
*****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Core/Utility.h"
#include "Engine/Maths/MathUtil.h"
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
	m_pGamepad = Input::GetGamepad(uGamepadId);
	m_uDetails = 0;
}

Controller::~Controller(void)
{
}


void Controller::ResetDetails()
{
	for(uint32 i=0; i<MAX_CONTROL_DETAILS; i++)
	{
		//m_details[i].Reset();
	}
	m_uDetails = 0;
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
	m_pGamepad = Input::GetGamepad(m_uGamepadId);	// Update so we can connect mid game
	m_sinceLastFrame = timeDelta;
	for(uint32 i=0; i<m_uDetails; i++)
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

bool Controller::GetValueAsBool( ControllerDetail &detail )
{
	// Default to false if the controller detail is unset
	switch( detail.deviceType )
	{
	case INPUT_TYPE_AXIS:
		{
			float fOutput = m_pGamepad->GetAxisValue(detail.uInputIdA);
			switch( detail.axisType )
			{
			case AXIS_TYPE_ABSOLUTE:
				return ( fOutput > ( -1.0f + m_boolDeadZone ) );
			case AXIS_TYPE_POSITIVE:
				return ( fOutput > m_boolDeadZone );
			case AXIS_TYPE_NEGATIVE:
				return ( fOutput < -m_boolDeadZone );
			default:
				ASSERT(false);
			}
		}
		break;
	case INPUT_TYPE_BUTTON:
		return m_pGamepad->GetButtonDown(detail.uInputIdA, detail.eInputState);
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
			switch( detail.axisType )
			{
				case AXIS_TYPE_ABSOLUTE:
					{
						float fOutput = m_pGamepad->GetButtonDown( detail.uInputIdB, BUTTON_STATE_HELD ) ? fValue : 0.0f;
						fOutput += m_pGamepad->GetButtonDown( detail.uInputIdA, BUTTON_STATE_HELD ) ? -fValue : 0.0f;
						return fOutput;
					}
				case AXIS_TYPE_POSITIVE:
					return m_pGamepad->GetButtonDown( detail.uInputIdA, BUTTON_STATE_HELD) ? 1.0f : 0.0f;
				case AXIS_TYPE_NEGATIVE:
					return m_pGamepad->GetButtonDown( detail.uInputIdA, BUTTON_STATE_HELD) ? -1.0f : 0.0f;
				default:
					ASSERT(false);
				break;
			}
		}
	case INPUT_TYPE_AXIS:
		{
			float fOutput = m_pGamepad->GetAxisValue( detail.uInputIdA);

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
			default:
				ASSERT(false);
			}
			return fOutput;
		}
	default:
		//ASSERT(false);
		return 0.0f;
	}
}

bool Controller::CreateButtonMapping(uint32 uButton, MappingOutput& output, ButtonState eInputState)
{
	ControllerDetail& detail = GetControllerDetail();
	detail.pResult		= &output;
	output.eOutput		= OUTPUT_TYPE_BOOL;
	output.Clear();
	detail.deviceType	= INPUT_TYPE_BUTTON;
	detail.axisType		= AXIS_TYPE_POSITIVE;
	detail.uInputIdA	= uButton;
	detail.uInputIdB	= GAMEPAD_BUTTON_NONE;
	detail.eInputState  = eInputState;

	return true;
}

bool Controller::CreateAxisMapping(GamepadAxis uAxis, AxisType eType, MappingOutput& output, float fStickyRate, bool bReverse)
{
	ControllerDetail& detail = GetControllerDetail();
	detail.pResult		= &output;
	detail.pResult->fStickyRate = fStickyRate;
	output.eOutput		= OUTPUT_TYPE_FLOAT;
	output.Clear();
	output.data.fValue = 0.0f;
	detail.deviceType	= INPUT_TYPE_AXIS;
	detail.axisType		= eType;
	detail.bReverse		= bReverse;
	detail.uInputIdA	= uAxis;
	detail.uInputIdB	= _GamepadAxis_count;

	return true;
}

bool Controller::CreateButtonFromAxis( GamepadAxis uAxis,  AxisType eType, MappingOutput& output )
{
	ControllerDetail& detail = GetControllerDetail();
	detail.pResult		= &output;
	output.eOutput		= OUTPUT_TYPE_BOOL;
	output.Clear();

	detail.deviceType	= INPUT_TYPE_AXIS;
	detail.axisType		= eType;
	detail.uInputIdA	= uAxis;
	detail.uInputIdB	= _GamepadAxis_count;

	return true;
}

bool Controller::CreateAxisFromButtonPair(GamepadButton uButtonA, GamepadButton uButtonB, MappingOutput& output, float fStickyRate, bool bReverse)
{
	ControllerDetail& detail = GetControllerDetail();
	detail.pResult		= &output;
	detail.pResult->fStickyRate = fStickyRate;
	output.eOutput		= OUTPUT_TYPE_FLOAT;
	output.Clear();
	output.data.fValue = 0.0f;

	detail.deviceType	= INPUT_TYPE_BUTTON;
	detail.axisType		= AXIS_TYPE_ABSOLUTE;
	detail.bReverse		= bReverse;
	detail.uInputIdA	= uButtonA;
	detail.uInputIdB	= uButtonB;

	return true;
}


Controller::ControllerDetail& Controller::GetControllerDetail()
{
	ASSERT(m_uDetails < MAX_CONTROL_DETAILS);
	ControllerDetail& out = m_details[m_uDetails];
	m_uDetails++;
	return out;
}

}
