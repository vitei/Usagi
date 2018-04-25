/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Maths/MathUtil.h"
#include OS_HEADER(Engine/HID, Input_ps.h)
#include "VirtualGamepad.h"
#include "glfw3.h"

// NOTE: These are defined in the game-specific code, so we need to declare
//       these externs outside of the usg namespace.
extern float g_mouseX,g_mouseY;

namespace usg
{

struct KeyboardMapping
{
	GamepadButton	buttonMap;
	int			keyboardKey;
};

static const KeyboardMapping g_buttonMapping[] =
{
	{ GAMEPAD_BUTTON_A, GLFW_KEY_SPACE },
	{ GAMEPAD_BUTTON_B, GLFW_KEY_X },
	{ GAMEPAD_BUTTON_X, GLFW_KEY_W },
	{ GAMEPAD_BUTTON_Y, GLFW_KEY_A },

	{ GAMEPAD_BUTTON_ZL, GLFW_KEY_Q },
	{ GAMEPAD_BUTTON_ZR, GLFW_KEY_E },

	{ GAMEPAD_BUTTON_L, GLFW_KEY_A },
	{ GAMEPAD_BUTTON_R, GLFW_KEY_D },
	
	{ GAMEPAD_BUTTON_UP, GLFW_KEY_UP },
	{ GAMEPAD_BUTTON_DOWN, GLFW_KEY_DOWN },
	{ GAMEPAD_BUTTON_LEFT, GLFW_KEY_LEFT },
	{ GAMEPAD_BUTTON_RIGHT,GLFW_KEY_RIGHT },

	{ GAMEPAD_BUTTON_START, GLFW_KEY_H },
	{ GAMEPAD_BUTTON_SELECT,GLFW_KEY_J },

	{ GAMEPAD_BUTTON_NONE, 0 },
};

VirtualGamepad::VirtualGamepad():
Gamepad()
{
}

void VirtualGamepad::Init(Input_ps* pOwner)
{
	m_pOwner = pOwner;

	// Debug keys
	memset(keysDown, 0, sizeof(keysDown));
	memset(keysTrig, 0, sizeof(keysTrig));
	memset(keysRel, 0, sizeof(keysRel));
}

VirtualGamepad::~VirtualGamepad()
{

}


void VirtualGamepad::Update()
{
	m_uButtonsPrevDown = m_uButtonsDown;
	m_uButtonsDown = 0;

	for(int i=0; i<GAMEPAD_AXIS_COUNT; i++)
	{
		m_fAxisValues[i] = 0.0f;
	}

	// Debug pressing issues
	for (uint16 i = 0; i < VIRTUAL_GAMEPAD_NUM_KEYS; i++)
	{
		keysTrig[i] = false;
		keysRel[i] = false;
		if (m_pOwner->GetKeyDown((int)i))
		{
			if (keysDown[i] == false)
				keysTrig[i] = true;
			keysDown[i] = true;
		}
		else
		{
			if (keysDown[i] == true)
				keysRel[i] = true;
			keysDown[i] = false;
		}
	}

	const KeyboardMapping* pMapping = g_buttonMapping;
	while(pMapping->buttonMap != GAMEPAD_BUTTON_NONE )
	{
		if(m_pOwner->GetKeyDown(pMapping->keyboardKey))
		{
			m_uButtonsDown |= pMapping->buttonMap;
		}
		pMapping++;
	}
	
	m_mGyro.MakeRotateYPR(g_mouseX * Math::pi*2, g_mouseY * Math::pi, 0.0);
		
//    m_fAxisValues[GAMEPAD_AXIS_LEFT_X] = m_pOwner->GetMouseX() * 2.0;
//    m_fAxisValues[GAMEPAD_AXIS_LEFT_Y] = m_pOwner->GetMouseY() * 2.0;
  
	
	if (m_pOwner->GetKeyDown(GLFW_KEY_R))
		m_fAxisValues[GAMEPAD_AXIS_LEFT_X] = 1.0f;
	if (m_pOwner->GetKeyDown(GLFW_KEY_Y))
		m_fAxisValues[GAMEPAD_AXIS_LEFT_X] = -1.0f;
	if (m_pOwner->GetKeyDown(GLFW_KEY_T))
		m_fAxisValues[GAMEPAD_AXIS_LEFT_Y] = 1.0f;
	if (m_pOwner->GetKeyDown(GLFW_KEY_G))
		m_fAxisValues[GAMEPAD_AXIS_LEFT_Y] = -1.0f;

	if (m_pOwner->GetKeyDown(GLFW_KEY_P))
		m_fAxisValues[GAMEPAD_AXIS_RIGHT_X] = 1.0f;
	if (m_pOwner->GetKeyDown(GLFW_KEY_I))
		m_fAxisValues[GAMEPAD_AXIS_RIGHT_X] = -1.0f;
	if (m_pOwner->GetKeyDown(GLFW_KEY_O))
		m_fAxisValues[GAMEPAD_AXIS_RIGHT_Y] = 1.0f;
	if (m_pOwner->GetKeyDown(GLFW_KEY_L))
		m_fAxisValues[GAMEPAD_AXIS_RIGHT_Y] = -1.0f;
	
}

}

