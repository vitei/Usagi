/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef __USG_HID_INPUT_STRUCTS_H__
#define __USG_HID_INPUT_STRUCTS_H__


#include "Engine/Maths/Matrix4x4.h"
#include "Engine/Maths/Vector2f.h"
#include "InputDefines.h"

namespace usg{


	struct GamepadDeviceState
	{
		Matrix4x4	leftHand;
		Matrix4x4	rightHand;

		bool		bScreenTouch = false;
		Vector2f	vTouchPos;

		uint32		uButtonsDown[6] = {};
		uint32		uButtonsPrevDown[6] = {};	// FIXME: This should live in Gamepad and be automatically recorded

		uint32		uButtonsContact = 0;	// Finger contact but no press
		uint32		uFingerStates = 0;

		float		fAxisValues[GAMEPAD_AXIS_NONE] = {};

		void ClearButtons()
		{
			for (uint32 i = 0; i < ARRAY_SIZE(uButtonsDown); i++)
			{
				uButtonsPrevDown[i] = uButtonsDown[i];
				uButtonsDown[i] = 0;
			}
		}

		// Helper function
		void SetButton(uint32 uButton, bool bValue)
		{
			uButton -= 1;
			uint32 uIndex = uButton / 32;
			uButton -= (uIndex * 32);

			uint32 uButtonMask = 1 << (uButton);

			ASSERT(uIndex < ARRAY_SIZE(uButtonsDown));

			if(bValue)
			{
				uButtonsDown[uIndex] |= uButtonMask;
			}
			else
			{
				uButtonsDown[uIndex] &= ~uButtonMask;
			}
		}
	};

}


#endif
