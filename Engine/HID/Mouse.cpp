/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Mouse.h"

namespace usg
{

	bool Mouse::GetButton(MouseButton eButton, ButtonState eState) const
	{
		switch(eState)
		{
		case BUTTON_STATE_PRESSED:
			{
				return (m_buttons[eButton] && !m_prevButtons[eButton]);
			}
		case BUTTON_STATE_RELEASED:
			{
				return (m_prevButtons[eButton] && !m_buttons[eButton]);
			}
		case BUTTON_STATE_HELD:
			{
				return (m_buttons[eButton] && m_prevButtons[eButton]);
			}
		default:
			ASSERT(false);	// Unhandled state
		}
		return false;
	}

};
