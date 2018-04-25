/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef __USG_HID_INPUT_STRUCTS_H__
#define __USG_HID_INPUT_STRUCTS_H__

#include "Engine/Common/Common.h"
#include "Engine/Maths/Matrix4x4.h"
#include "Engine/Maths/Vector2f.h"
#include "InputDefines.h"

namespace usg{


	struct GamepadDeviceState
	{
		Matrix4x4	leftHand;
		Matrix4x4	rightHand;

		bool		bScreenTouch;
		Vector2f	vTouchPos;

		uint32		uButtonsDown;
		uint32		uButtonsPrevDown;	// FIXME: This should live in Gamepad and be automatically recorded

		uint32		uButtonsContact;	// Finger contact but no press
		uint32		uFingerStates;

		float		fAxisValues[GAMEPAD_AXIS_NONE];
	};

}


#endif
