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
		{ DIK_ESCAPE , KEYBOARD_KEY_ESCAPE },
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
		{ DIK_MINUS, KEYBOARD_KEY_MINUS },
		{ DIK_EQUALS, KEYBOARD_KEY_PLUS },
		{ DIK_BACK, KEYBOARD_KEY_BACK },
		{ DIK_TAB, KEYBOARD_KEY_TAB },
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
		{ DIK_LBRACKET, '[' },
		{ DIK_RBRACKET, ']' },
		{ DIK_RETURN, KEYBOARD_KEY_RETURN },
		{ DIK_LCONTROL, KEYBOARD_KEY_CONTROL },
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
		{ DIK_LSHIFT, KEYBOARD_KEY_SHIFT }, 
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
		{ DIK_RSHIFT, KEYBOARD_KEY_SHIFT },
		{ DIK_MULTIPLY, '*' },
		{ DIK_LMENU, KEYBOARD_KEY_ALT },
		{ DIK_SPACE, ' ' },
		{ DIK_CAPITAL, KEYBOARD_KEY_CAPS },
		{ DIK_F1, KEYBOARD_KEY_F1 },
		{ DIK_F2, KEYBOARD_KEY_F2 },
		{ DIK_F3, KEYBOARD_KEY_F3 },
		{ DIK_F4, KEYBOARD_KEY_F4 },
		{ DIK_F5, KEYBOARD_KEY_F5 },
		{ DIK_F6, KEYBOARD_KEY_F6 },
		{ DIK_F7, KEYBOARD_KEY_F7 },
		{ DIK_F8, KEYBOARD_KEY_F8 },
		{ DIK_F9, KEYBOARD_KEY_F9 },
		{ DIK_F10, KEYBOARD_KEY_F10 },
		{ DIK_NUMLOCK, KEYBOARD_KEY_NUMLOCK },
		{ DIK_SCROLL, KEYBOARD_KEY_SCROLL },
		{ DIK_NUMPAD7, KEYBOARD_KEY_NUMPAD7 },
		{ DIK_NUMPAD8, KEYBOARD_KEY_NUMPAD8 },
		{ DIK_NUMPAD9, KEYBOARD_KEY_NUMPAD9 },
		{ DIK_SUBTRACT, KEYBOARD_KEY_SUBTRACT },
		{ DIK_NUMPAD4, KEYBOARD_KEY_NUMPAD4 },
		{ DIK_NUMPAD5, KEYBOARD_KEY_NUMPAD5 },
		{ DIK_NUMPAD6, KEYBOARD_KEY_NUMPAD6 },
		{ DIK_ADD, KEYBOARD_KEY_ADD },
		{ DIK_NUMPAD1, KEYBOARD_KEY_NUMPAD1 },
		{ DIK_NUMPAD2, KEYBOARD_KEY_NUMPAD2 },
		{ DIK_NUMPAD3, KEYBOARD_KEY_NUMPAD3 },
		{ DIK_NUMPAD0, KEYBOARD_KEY_NUMPAD0 },
		{ DIK_DECIMAL, '.' },
		{ DIK_OEM_102, '<' },    /* <> or \| on RT 102-key keyboard (Non-U.S.) */
		{ DIK_F11, KEYBOARD_KEY_F11 },
		{ DIK_F12, KEYBOARD_KEY_F12 },
		{ DIK_F13, KEYBOARD_KEY_F13 },
		{ DIK_F14, KEYBOARD_KEY_F14 },
		{ DIK_F15, KEYBOARD_KEY_F15 },
		{ DIK_KANA, KEYBOARD_KEY_KANA },
		{ DIK_ABNT_C1, '?' },   /* /? on Brazilian keyboard */
		{ DIK_CONVERT, KEYBOARD_KEY_CONVERT },          /* (Japanese keyboard)            */
		{ DIK_NOCONVERT, KEYBOARD_KEY_NONCONVERT },     /* (Japanese keyboard)            */
		{ DIK_YEN, 165 },					 /* (Japanese keyboard)            */
		{ DIK_ABNT_C2, '.' },
		{ DIK_NUMPADEQUALS, '=' },
//		{ DIK_PREVTRACK, KEYBOARD_KEY_MEDIA_PREV_TRACK },
		{ DIK_AT, '@' },
		{ DIK_COLON, ':' },
		{ DIK_KANJI, KEYBOARD_KEY_KANJI },
		{ DIK_STOP, KEYBOARD_KEY_STOP },
		{ DIK_AX, KEYBOARD_KEY_AX }, 
		//{ DIK_NEXTTRACK, KEYBOARD_KEY_MEDIA_NEXT_TRACK },
		{ DIK_NUMPADENTER, KEYBOARD_KEY_RETURN }, 
		{ DIK_RCONTROL, KEYBOARD_KEY_CONTROL },
		//{ DIK_MUTE, KEYBOARD_KEY_VOLUME_MUTE }, 
		//{ DIK_PLAYPAUSE, KEYBOARD_KEY_MEDIA_PLAY_PAUSE },
		//{ DIK_MEDIASTOP, KEYBOARD_KEY_MEDIA_STOP },
		//{ DIK_VOLUMEDOWN, KEYBOARD_KEY_VOLUME_DOWN },
		//{ DIK_VOLUMEUP, KEYBOARD_KEY_VOLUME_UP },
		{ DIK_NUMPADCOMMA, ',' },
		{ DIK_RMENU, KEYBOARD_KEY_ALT },
		{ DIK_UP, KEYBOARD_KEY_UP },
		{ DIK_PAUSE, KEYBOARD_KEY_PAUSE },
		{ DIK_HOME, KEYBOARD_KEY_HOME },
		{ DIK_LEFT, KEYBOARD_KEY_LEFT },
		{ DIK_RIGHT, KEYBOARD_KEY_RIGHT },
		{ DIK_END, KEYBOARD_KEY_END },
		{ DIK_DOWN, KEYBOARD_KEY_DOWN },
		{ DIK_PGDN, KEYBOARD_KEY_PGDN },
		{ DIK_PGUP, KEYBOARD_KEY_PGUP },
		{ DIK_INSERT, KEYBOARD_KEY_INSERT },
		{ DIK_DELETE, KEYBOARD_KEY_DELETE },
		{ DIK_DIVIDE, '/' },
		{ DIK_SYSRQ, KEYBOARD_KEY_PRINT }
	};

	// Unmapped keys
	/*
	DIK_UNDERLINE, DIK_UNLABELED, DIK_CALCULATOR, DIK_WEBHOME,
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

		if (FAILED(m_pDevice->SetCooperativeLevel(pInput->GetWindow(), DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY)))
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
		char keys[256] = {};

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


		#if 0
		m_uInputChars = 0;
		for (uint32 i = 0; i < m_objectDataSize; i++)
		{
			DWORD key = m_objectData[i].dwOfs;
			if (m_objectData[i].dwData & 0x80 && m_uInputChars < MAX_INPUT_CHARS)
			{
				for (memsize j = 0; j < ARRAY_SIZE(g_sKeyMapping); j++)
				{
					if (g_sKeyMapping[j].uDICode == key)
					{
						char16 keyOut = (char16)g_sKeyMapping[j].uVKCode;
						m_inputChars[m_uInputChars] = keyOut;
						m_uInputChars++;
						break;
					}
				}

			}
		}
		#else
		m_uInputChars = m_pOwner->GetInputChars();
		for(uint32 i=0; i<m_uInputChars; i++)
		{
			m_inputChars[i] = m_pOwner->GetInputChar(i);
		}
		#endif
	}

}