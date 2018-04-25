/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: A gamepad interface which uses the XBox controller
*****************************************************************************/
#ifndef __USG_HID_XBOX_PAD_H__
#define __USG_HID_XBOX_PAD_H__
#include "Engine/Common/Common.h"
#include "Engine/HID/Gamepad.h"

namespace usg
{

class XBoxPad : public Gamepad
{
public:
	XBoxPad() {}
	~XBoxPad() {}

	void Init(uint32 uPadNum);
	virtual void Update();
	bool IsConnected();

private:
	
	uint32	m_xinputID;
};
 
}

#endif
