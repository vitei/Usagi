/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: A gamepad interface which actually uses the mouse and keyboard
*****************************************************************************/
#ifndef __USG_HID_PC_VIRTUALGAMEPAD_H__
#define __USG_HID_PC_VIRTUALGAMEPAD_H__
#include "Engine/Common/Common.h"
#include "Engine/HID/Gamepad.h"

namespace usg
{

class Input_ps;

#define VIRTUAL_GAMEPAD_NUM_KEYS (348)

class VirtualGamepad : public Gamepad
{
public:
	VirtualGamepad();
	~VirtualGamepad();

	void Init(Input_ps* pOwner);

	virtual void Update();

	virtual bool DebugIsKeyTriggered(uint16 keyboardKey){ return keysTrig[keyboardKey]; }
	virtual bool DebugIsKeyPressed(uint16 keyboardKey){ return keysDown[keyboardKey]; }
	virtual bool DebugIsKeyReleased(uint16 keyboardKey){ return keysRel[keyboardKey]; }

private:
	const Input_ps*	m_pOwner;

	bool keysDown[VIRTUAL_GAMEPAD_NUM_KEYS];
	bool keysTrig[VIRTUAL_GAMEPAD_NUM_KEYS];
	bool keysRel[VIRTUAL_GAMEPAD_NUM_KEYS];
};

}

#endif
