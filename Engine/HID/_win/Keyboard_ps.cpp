/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Input_ps.h"
#include "Keyboard_ps.h"

namespace usg
{
	void Keyboard_ps::Update()
	{
		for(uint32 i=0; i<KEYBOARD_KEY_COUNT; i++)
		{
			m_prevKeysDown[i] = m_keysDown[i];
			m_keysDown[i] = m_pOwner->GetKeyDown(i);
		}

		// We never get a key event about this
		m_keysDown[VK_MENU] = (GetKeyState(VK_MENU) & 0x8000) != 0;

		m_bToggles[KEYBOARD_TOGGLE_CTRL] = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
		m_bToggles[KEYBOARD_TOGGLE_SHIFT] = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
		m_bToggles[KEYBOARD_TOGGLE_ALT] = (GetKeyState(VK_MENU) & 0x8000) != 0;

		m_uInputChars = m_pOwner->GetInputChars();
		for(uint32 i=0; i<m_uInputChars; i++)
		{
			m_inputChars[i] = m_pOwner->GetInputChar(i);
		}
	}

}