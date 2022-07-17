/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Input_ps.h"
#include "DirectInputKeyboard.h"

namespace usg
{
	struct CodeMap
	{
		int uDICode;
		int uVKCode;
	};

	// TODO: We probably want our own key code defines
	static const CodeMap g_sKeyMapping[] =
	{
		{ DIK_ESCAPE , VK_ESCAPE },
		{ DIK_1, '1' },
		{ DIK_2, '2' },
		{ DIK_3, '3' },
		{ DIK_4, '4' },
		{ DIK_5, '5' },
		{ DIK_6, '6' },
		{ DIK_7, '7' },
		{ DIK_8, '8' },
		{ DIK_9, '9' },
		{ DIK_0, '0' },
		{ DIK_MINUS, VK_OEM_MINUS },
		{ DIK_EQUALS, VK_OEM_PLUS },
		{ DIK_BACK, VK_BACK },
		{ DIK_TAB, VK_TAB },
		{ DIK_Q, 'Q' },
		{ DIK_W, 'W' },
		{ DIK_E, 'E' },
		{ DIK_R, 'R' },
		{ DIK_T, 'T' },
		{ DIK_Y, 'Y' },
		{ DIK_U, 'U' },
		{ DIK_I, 'I' },
		{ DIK_O, 'O' },
		{ DIK_P, 'P' },
		{ DIK_LBRACKET, '(' },
		{ DIK_RBRACKET, ')' },
		{ DIK_RETURN, VK_RETURN },
		{ DIK_LCONTROL, VK_CONTROL },
		{ DIK_A, 'A' },
		{ DIK_S, 'S' },
		{ DIK_D, 'D' },
		{ DIK_F, 'F' },
		{ DIK_G, 'G' },
		{ DIK_H, 'H' },
		{ DIK_J, 'J' },
		{ DIK_K, 'K' },
		{ DIK_L, 'L' },
		{ DIK_SEMICOLON, ';' },
		{ DIK_APOSTROPHE, '\'' },
		{ DIK_GRAVE, '`' },
		{ DIK_LSHIFT, VK_SHIFT }, 
		{ DIK_BACKSLASH, '\\' },
		{ DIK_Z, 'Z' },
		{ DIK_X, 'X' },
		{ DIK_C, 'C' },
		{ DIK_V, 'V' },
		{ DIK_B, 'B' },
		{ DIK_N, 'N' },
		{ DIK_M, 'M' },
		{ DIK_COMMA, ',' },
		{ DIK_PERIOD, '.' },
		{ DIK_SLASH, '/' },
		{ DIK_RSHIFT, VK_SHIFT },
		{ DIK_MULTIPLY, '*' },
		{ DIK_LMENU, VK_MENU },
		{ DIK_SPACE, ' ' },
		{ DIK_CAPITAL, VK_CAPITAL },
		{ DIK_F1, VK_F1 },
		{ DIK_F2, VK_F2 },
		{ DIK_F3, VK_F3 },
		{ DIK_F4, VK_F4 },
		{ DIK_F5, VK_F5 },
		{ DIK_F6, VK_F6 },
		{ DIK_F7, VK_F7 },
		{ DIK_F8, VK_F8 },
		{ DIK_F9, VK_F9 },
		{ DIK_F10, VK_F10 },
		{ DIK_NUMLOCK, VK_NUMLOCK },
		{ DIK_SCROLL, VK_SCROLL },
		{ DIK_NUMPAD7, VK_NUMPAD7 },
		{ DIK_NUMPAD8, VK_NUMPAD8 },
		{ DIK_NUMPAD9, VK_NUMPAD9 },
		{ DIK_SUBTRACT, VK_SUBTRACT },
		{ DIK_NUMPAD4, VK_NUMPAD4 },
		{ DIK_NUMPAD5, VK_NUMPAD5 },
		{ DIK_NUMPAD6, VK_NUMPAD6 },
		{ DIK_ADD, VK_ADD },
		{ DIK_NUMPAD1, VK_NUMPAD1 },
		{ DIK_NUMPAD2, VK_NUMPAD2 },
		{ DIK_NUMPAD3, VK_NUMPAD3 },
		{ DIK_NUMPAD0, VK_NUMPAD0 },
		{ DIK_DECIMAL, '.' },
		{ DIK_OEM_102, '<' },    /* <> or \| on RT 102-key keyboard (Non-U.S.) */
		{ DIK_F11, VK_F11 },
		{ DIK_F12, VK_F12 },
		{ DIK_F13, VK_F13 },
		{ DIK_F14, VK_F14 },
		{ DIK_F15, VK_F15 },
		{ DIK_KANA, VK_KANA },
		{ DIK_ABNT_C1, '?' },   /* /? on Brazilian keyboard */
		{ DIK_CONVERT, VK_CONVERT },          /* (Japanese keyboard)            */
		{ DIK_NOCONVERT, VK_NONCONVERT },     /* (Japanese keyboard)            */
		{ DIK_YEN, 165 },					 /* (Japanese keyboard)            */
		{ DIK_ABNT_C2, '.' },
		{ DIK_NUMPADEQUALS, '=' },
//		{ DIK_PREVTRACK, VK_MEDIA_PREV_TRACK },
		{ DIK_AT, '@' },
		{ DIK_COLON, ':' },
		{ DIK_KANJI, VK_KANJI },
		{ DIK_STOP, VK_MEDIA_STOP },
		{ DIK_AX, VK_OEM_AX }, 
		//{ DIK_NEXTTRACK, VK_MEDIA_NEXT_TRACK },
		{ DIK_NUMPADENTER, VK_RETURN }, 
		{ DIK_RCONTROL, VK_CONTROL },
		//{ DIK_MUTE, VK_VOLUME_MUTE }, 
		//{ DIK_PLAYPAUSE, VK_MEDIA_PLAY_PAUSE },
		//{ DIK_MEDIASTOP, VK_MEDIA_STOP },
		//{ DIK_VOLUMEDOWN, VK_VOLUME_DOWN },
		//{ DIK_VOLUMEUP, VK_VOLUME_UP },
		{ DIK_NUMPADCOMMA, ',' },
		{ DIK_RMENU, VK_MENU },
		{ DIK_UP, VK_UP },
		{ DIK_PAUSE, VK_PAUSE },
		{ DIK_HOME, VK_HOME },
		{ DIK_LEFT, VK_LEFT },
		{ DIK_RIGHT, VK_RIGHT },
		{ DIK_END, VK_END },
		{ DIK_DOWN, VK_DOWN },
		{ DIK_NEXT, VK_NEXT },
		{ DIK_INSERT, VK_INSERT },
		{ DIK_DELETE, VK_DELETE },
		{ DIK_DIVIDE, '/' },
	};

	// Unmapped keys
	/*
	DIK_PRIOR, DIK_UNDERLINE, DIK_UNLABELED, DIK_CALCULATOR, DIK_WEBHOME,
	DIK_SYSRQ, DIK_LWIN, DIK_RWIN, DIK_APPS, DIK_POWER, DIK_SLEEP, DIK_WAKE,
	DIK_WEBSEARCH, DIK_WEBFAVORITES, DIK_WEBREFRESH, DIK_WEBSTOP, DIK_WEBFORWARD,
	DIK_WEBBACK, DIK_MYCOMPUTER, DIK_MAIL, DIK_MEDIASELECT*/



	bool DirectInputKeyboard::Init(Input_ps* pOwner, DirectInput* pInput, uint32 uDeviceNum)
	{
		m_pOwner = pOwner;
		for (uint32 i = 0; i < KEYBOARD_KEY_COUNT; i++)
		{
			m_prevKeysDown[i] = false;
			m_keysDown[i] = false;
		}

		//Try to create the device
		if (FAILED(pInput->GetDirectInput()->CreateDevice(GUID_SysKeyboard, &m_pDevice, NULL)))
		{
			return false;
		}

		//Set the data format
		if (FAILED(m_pDevice->SetDataFormat(&c_dfDIKeyboard)))
		{
			return false;
		}

		if (FAILED(m_pDevice->SetCooperativeLevel(pInput->GetWindow(), DISCL_FOREGROUND | DISCL_EXCLUSIVE)))
		{
			return false;
		}

		if (!(m_hEvent = CreateEvent(NULL, FALSE, FALSE, NULL)))
		{
			ASSERT_MSG(false, "CreateEvent Failed");
			return false;
		}

		if (FAILED(m_pDevice->SetEventNotification(m_hEvent)))
		{
			ASSERT_MSG(false, "Set event notification failed");
			return false;
		}

		DIPROPDWORD dipdw;
		dipdw.diph.dwSize = sizeof(DIPROPDWORD);
		dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
		dipdw.diph.dwObj = 0;
		dipdw.diph.dwHow = DIPH_DEVICE;
		dipdw.dwData = 16;

		if (FAILED(m_pDevice->SetProperty(DIPROP_BUFFERSIZE, &dipdw.diph)))
		{
			ASSERT_MSG(false, "Set property failed");
			return false;
		}

		m_pDevice->Acquire();


		return true;
	}

	void DirectInputKeyboard::Update()
	{
		char keys[KEYBOARD_KEY_COUNT] = {};

		m_pDevice->Poll();

		HRESULT hr;
		hr = m_pDevice->GetDeviceState(sizeof(keys), keys);
		if (FAILED(hr))
		{
			HRESULT hr = m_pDevice->Acquire();
			if (SUCCEEDED(hr))
			{
				hr = m_pDevice->GetDeviceState(sizeof(keys), keys);
			}
		}

		m_objectDataSize = ARRAY_SIZE(m_objectData);
		m_pDevice->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), m_objectData, &m_objectDataSize, 0);


		for(uint32 i=0; i<KEYBOARD_KEY_COUNT; i++)
		{
			m_prevKeysDown[i] = m_keysDown[i];

		}

		for (uint32 i = 0; i < KEYBOARD_KEY_COUNT; i++)
		{
			m_keysDown[i] = false;
		}

		for (memsize i = 0; i < ARRAY_SIZE(g_sKeyMapping); i++)
		{
			if ( keys[ g_sKeyMapping[i].uDICode ] & 0x80)
			{
				m_keysDown[ g_sKeyMapping[i].uVKCode ] = true;
			}
		}

		m_bToggles[KEYBOARD_TOGGLE_CTRL] = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
		m_bToggles[KEYBOARD_TOGGLE_SHIFT] = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
		m_bToggles[KEYBOARD_TOGGLE_ALT] = (GetKeyState(VK_MENU) & 0x8000) != 0;


		m_uInputChars = 0;
		for (uint32 i = 0; i < m_objectDataSize; i++)
		{
			DWORD key = m_objectData[i].dwOfs;
			if (m_objectData[i].dwData & 0x80 && m_uInputChars < MAX_INPUT_CHARS)
			{
				m_inputChars[m_uInputChars] = (char16)key;
				m_uInputChars++;
			}
		}
		
		#if 0
		m_uInputChars = m_pOwner->GetInputChars();
		for(uint32 i=0; i<m_uInputChars; i++)
		{
			m_inputChars[i] = m_pOwner->GetInputChar(i);
		}
		#endif
	}

}