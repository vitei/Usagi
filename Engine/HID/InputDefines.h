/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef __USG_HID_INPUT_DEFINES_H__
#define __USG_HID_INPUT_DEFINES_H__

#include "Engine/HID/InputEnums.pb.h"

namespace usg{

const int KEYBOARD_KEY_COUNT = 256;

enum SampleType
{
	SAMPLING_TYPE_SIGNED_8BIT = 0,
	SAMPLING_TYPE_SIGNED_16BIT,
	SAMPLING_TYPE_UNSIGNED_8BIT,
	SAMPLING_TYPE_UNSIGNED_16BIT
};

enum SampleRate
{
	SAMPLING_RATE_32KHZ = 0,		//!< 32.73kHz
	SAMPLING_RATE_16KHZ,            //!< 16.36kHz
	SAMPLING_RATE_11KHZ,            //!< 10.91kHz
	SAMPLING_RATE_8KHZ              //!< 8.18kHz
};


enum KeyboardToggles
{
	KEYBOARD_TOGGLE_SHIFT = 0,
	KEYBOARD_TOGGLE_CTRL,
	KEYBOARD_TOGGLE_ALT,
	KEYBOARD_TOGGLE_COUNT
};

enum class GamepadHand
{
	Left = 0,
	Right
};

enum GamepadCaps
{
	CAP_POINTER = (1 << 0),
	CAP_ACCELEROMETER = (1 << 1),
	CAP_DUAL_HANDED = (1 << 2),
	CAP_LEFT_STICK = (1<<3),
	CAP_RIGHT_STICK = (1<<4),
	CAP_GAMEPAD = (1<<5),
	CAP_POV = (1<<6)
};

}


#endif
