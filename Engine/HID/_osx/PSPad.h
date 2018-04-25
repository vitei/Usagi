/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: A gamepad interface which uses the PlayStation controller
*****************************************************************************/
#ifndef __USG_HID_PSPAD_H__
#define __USG_HID_PSPAD_H__
#include "Engine/Common/Common.h"
#include "Engine/HID/Gamepad.h"

namespace usg
{

class PSPad : public Gamepad
{
public:
	PSPad() {}
	~PSPad() {}

	void Init(uint32 uPadNum);
	virtual void Update();
	bool IsConnected();
	

private:
	
	uint32	m_inputID;
};
 
}

#endif
