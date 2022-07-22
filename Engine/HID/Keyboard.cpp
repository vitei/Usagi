/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Keyboard.h"

namespace usg
{

	bool Keyboard::GetKey(uint32 keyboardKey, ButtonState eState) const
	{
		switch(eState)
		{
		case BUTTON_STATE_PRESSED:
			{
				return (m_keysDown[keyboardKey] && !m_prevKeysDown[keyboardKey]);
			}
		case BUTTON_STATE_RELEASED:
			{
				return (m_prevKeysDown[keyboardKey] && !m_keysDown[keyboardKey]);
			}
		case BUTTON_STATE_HELD:
			{
				return (m_keysDown[keyboardKey] && m_prevKeysDown[keyboardKey]);
			}
		default:
			ASSERT(false);	// Unhandled state
		}
		return false;
	}

}
