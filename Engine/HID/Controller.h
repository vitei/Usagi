/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: A base class for player input, the game should inherit from this
//	but it should not be all encompassing (i.e. on foot controls should use a 
//	different controller to flight controls)
*****************************************************************************/
#ifndef __USG_HID_CONTROLLER_H__
#define __USG_HID_CONTROLLER_H__
#include "Engine/Common/Common.h"
#include "Input.h"

namespace usg{

class Controller
{
public:
	Controller(uint32 uGamepadId);
	virtual ~Controller();

	virtual void Update( float timeDelta );

protected:
	struct MappingOutput
	{
		OutputType eOutput;
		bool GetBool() const;
		float GetFloat() const;

		void Clear(bool bForce = false);
		union
		{
			float	fValue;
			bool	bValue;
		} data;

		float	fStickyRate;
	};

	struct ControllerDetail
	{
		ControllerDetail(){ Reset(); }
		void Reset()
		{
			deviceType = INPUT_TYPE_NONE;
			eInputState = BUTTON_STATE_PRESSED;
			axisType = AXIS_TYPE_NONE;
			bReverse = false;
			uInputIdA = USG_INVALID_ID;
			uInputIdB = USG_INVALID_ID;
		}
		InputType		deviceType;		// The type of device, e.g. button, axis, etc
		AxisType		axisType;
		bool			bReverse;

		// Should only have two inputs when mapping a button pair onto an axis
		uint32			uInputIdA;
		uint32			uInputIdB;

		ButtonState		eInputState;		// If we are looking for pressed, released or down input state.
		MappingOutput*	pResult;
	};


	bool	GetValueAsBool( ControllerDetail &detail );
	float	GetValueAsFloat( ControllerDetail &detail );

	bool CreateButtonMapping(uint32 uButton, MappingOutput& detailOut, ButtonState eInputState = BUTTON_STATE_PRESSED);
	bool CreateKeyMapping(uint8 uKey, MappingOutput& detailOut, ButtonState eInputState = BUTTON_STATE_PRESSED);
	bool CreateMouseButtonMapping(MouseButton eButton, MappingOutput& detailOut, ButtonState eInputState = BUTTON_STATE_PRESSED);
	bool CreateAxisMapping(GamepadAxis uAxis, AxisType eType, MappingOutput &detailOut, float fStickyRate = 0.0f, bool bReverse = false);
	bool CreateButtonFromAxis(GamepadAxis uAxis,  AxisType eType, MappingOutput& detailOut);
	bool CreateAxisFromButtonPair(GamepadButton uButtonA, GamepadButton uButtonB, MappingOutput& detailOut, float fStickyRate = 0.0f, bool bReverse = false);
	void ResetDetails();
	void ClearMappingSet(MappingOutput* pOutputs, uint32 uCount);

	uint32	GetGamepadId() { return m_uGamepadId; }
	Gamepad* GetGamepad() { return m_pGamepad; }

private:
	enum
	{
		MAX_CONTROL_DETAILS = 40
	};

	ControllerDetail& GetControllerDetail();
	ControllerDetail	m_details[MAX_CONTROL_DETAILS];
	uint32				m_uDetails;
	Gamepad*			m_pGamepad;
	Keyboard*			m_pKeyboard;
	Mouse*				m_pMouse;
	uint32				m_uGamepadId;
	float32				m_boolDeadZone;
	float32				m_sinceLastFrame;
};

}

#endif
