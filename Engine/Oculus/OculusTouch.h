/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: A gamepad interface which actually uses the mouse and keyboard
*****************************************************************************/
#ifndef __USG_HID_PC_OCULUS_TOUCH_H__
#define __USG_HID_PC_OCULUS_TOUCH_H__
#include "Engine/Common/Common.h"
#include "Engine/HID/IGamepad.h"

namespace usg{

class Input_ps;

class OculusTouch : public IGamepad
{
public:
	OculusTouch();
	~OculusTouch();

	void Init(OculusHMD* pHmd);
	virtual uint32 GetCaps() const override;
	virtual void Update(GFXDevice* pDevice, GamepadDeviceState& state) override;
	virtual bool IsConnected() const override { return m_bConnected; }
	virtual const char* GetModuleName() const { return "OculusTouch"; }
private:
	OculusHMD* m_pHMD;
	bool m_bConnected;
};
 
}

#endif