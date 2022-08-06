/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: The base class for the mouse (PC and debug purposes)
*****************************************************************************/
#ifndef __USG_HID_MOUSE__
#define __USG_HID_MOUSE__

#include "Engine/HID/InputDefines.h"

namespace usg{

class Mouse
{
public:
    Mouse(){}
    ~Mouse(){}

	bool GetButton(MouseButton eButton, ButtonState eState = BUTTON_STATE_HELD) const;
	float GetAxis(MouseAxis eAxis) const { return m_fAxis[eAxis]; }

	struct MouseBox
	{
		usg::Vector2f vCentre = usg::Vector2f(0.0f, 0.0f);
		usg::Vector2f vBounds = usg::Vector2f(1.f, 1.f);
		bool bAspectCorrect = true;

	};

	void SetMouseBox(const MouseBox& mouseBox) { m_mouseBox = mouseBox; }


protected:
	
	MouseBox	m_mouseBox;

	bool	m_prevButtons[_MouseButton_count];
	bool	m_buttons[_MouseButton_count];
	
	float	m_fAxis[_MouseAxis_count];
};


}

#endif
