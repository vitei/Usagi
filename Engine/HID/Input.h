/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Raw user input (mouse/keyboard/touchscreen/pointer etc)
*****************************************************************************/
#ifndef __USG_HID_HID_H__
#define __USG_HID_HID_H__

#include "Engine/HID/InputDefines.h"
#include "Gamepad.h"
#include "Microphone.h"

namespace usg{

class Input_ps;
class Keyboard;
class Mouse;
class IGamepad;

typedef int (*EvaluatePad) (const IGamepad* pDad);

class Input
{
public:	
	static void Init();
	static void Cleanup();
	static void RenumberGamepads(EvaluatePad fnEvaluatePad = DefaultEvaluate);
	static void Update(usg::GFXDevice* pDevice, float fDelta);
	static void RegisterGamepad(IGamepad* pGamepad);

	static Gamepad*	GetGamepad(uint32 uGamepad);
	static uint32 GetGamepadCount();
	static Gamepad* GetAccerometer();
	static Keyboard* GetKeyboard();
	static Mouse* GetMouse();

	static Microphone* GetMicrophone(uint32 uMicrophone);

	static Input_ps&	GetPlatform();

	static void DeviceChange();

	static int DefaultEvaluate(const IGamepad* pPad) { return 1; }

private:
};

}

#endif
