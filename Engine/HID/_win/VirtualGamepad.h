/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: A gamepad interface which actually uses the mouse and keyboard
*****************************************************************************/
#ifndef __USG_HID_PC_VIRTUALGAMEPAD_H__
#define __USG_HID_PC_VIRTUALGAMEPAD_H__

#include "Engine/HID/Keyboard.h"
#include "Engine/HID/IGamepad.h"

namespace usg{

class Input_ps;

#define VIRTUAL_GAMEPAD_NUM_KEYS (256)

class VirtualGamepad : public IGamepad
{
public:
	VirtualGamepad();
	~VirtualGamepad();

	void Init(Keyboard* pKeyboard);

	virtual uint32 GetCaps() const override { return CAP_POINTER|CAP_LEFT_STICK|CAP_RIGHT_STICK|CAP_VIRTUAL_PAD; }
	virtual bool IsConnected() const { return true; }
	virtual void Update(GFXDevice* pDevice, GamepadDeviceState& deviceStateOut) override;
	virtual const char* GetProductName() const { return "VirtualGamepad"; }

	virtual const char* GetModuleName() const { return "VirtualGamepad"; }
private:
	const Keyboard*	m_pKeyboard;
};

}

#endif
