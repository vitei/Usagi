/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: The base class for the mouse (PC and debug purposes)
*****************************************************************************/
#ifndef __USG_HID_MOUSE__
#define __USG_HID_MOUSE__
#include "Engine/Common/Common.h"
#include "Engine/HID/InputDefines.h"

namespace usg{

class Mouse
{
public:
    Mouse(){}
    ~Mouse(){}

	bool GetButton(MouseButton eButton, ButtonState eState = BUTTON_STATE_HELD) const;
	float GetAxis(MouseAxis eAxis) const { return m_fAxis[eAxis]; }

protected:
	bool	m_prevButtons[_MouseButton_count];
	bool	m_buttons[_MouseButton_count];
	
	float	m_fAxis[_MouseAxis_count];
};


}

#endif
