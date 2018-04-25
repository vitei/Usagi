/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Maths/MathUtil.h"
#include "XBoxPad.h"
#include "glfw3.h"

// NOTE: These are defined in the game-specific code, so we need to declare
//       these externs outside of the usg namespace.
extern float g_mouseX,g_mouseY;
extern float g_mouseScrollX,g_mouseScrollY;

namespace usg
{

enum XINPUT_AXIS
{
	XINPUT_AXIS_LEFT_THUMB_X	=	0,
	XINPUT_AXIS_LEFT_THUMB_Y	=	1,
	XINPUT_AXIS_RIGHT_THUMB_X	=	2,
	XINPUT_AXIS_RIGHT_THUMB_Y	=	3,
	
	XINPUT_AXIS_LT				=	4,
	XINPUT_AXIS_RT				=	5
	
};

enum XINPUT_BUTTONS
{
	XINPUT_BUTTON_DPAD_UP		=	0,
	XINPUT_BUTTON_DPAD_DOWN		=	1,
	XINPUT_BUTTON_DPAD_LEFT		=	2,
	XINPUT_BUTTON_DPAD_RIGHT	=	3,

	XINPUT_BUTTON_START			=	4,
	XINPUT_BUTTON_BACK			=	5,
	XINPUT_BUTTON_L				=	8,
	XINPUT_BUTTON_R				=	9,
	
	XINPUT_BUTTON_A				=	12,
	XINPUT_BUTTON_B				=	11,
	XINPUT_BUTTON_X				=	13,
	XINPUT_BUTTON_Y				=	14
};

struct ControlMapping
{
	uint32	uAbstractID;
	uint32	uXInputID;
	bool	bReverse;
};

static const	ControlMapping g_axisMappings[] =
{
	{ GAMEPAD_AXIS_LEFT_X,		XINPUT_AXIS_LEFT_THUMB_X,	false },
	{ GAMEPAD_AXIS_LEFT_Y,		XINPUT_AXIS_LEFT_THUMB_Y,	true },
	{ GAMEPAD_AXIS_RIGHT_X,		XINPUT_AXIS_RIGHT_THUMB_X,	false },
	{ GAMEPAD_AXIS_RIGHT_Y,		XINPUT_AXIS_RIGHT_THUMB_Y,	true }
};

static const	ControlMapping g_buttonMappings[] =
{
	{ GAMEPAD_BUTTON_A,		XINPUT_BUTTON_A,			false },
	{ GAMEPAD_BUTTON_B,		XINPUT_BUTTON_B,			false },
	{ GAMEPAD_BUTTON_X,		XINPUT_BUTTON_X,			false },
	{ GAMEPAD_BUTTON_Y,		XINPUT_BUTTON_Y,			false },
	{ GAMEPAD_BUTTON_L,		XINPUT_BUTTON_L,			false },
	{ GAMEPAD_BUTTON_R,		XINPUT_BUTTON_R,			false },
	{ GAMEPAD_BUTTON_START,	XINPUT_BUTTON_START,		false },
	{ GAMEPAD_BUTTON_LEFT,	XINPUT_BUTTON_DPAD_LEFT,	false },
	{ GAMEPAD_BUTTON_RIGHT,	XINPUT_BUTTON_DPAD_RIGHT,	false },
	{ GAMEPAD_BUTTON_UP,	XINPUT_BUTTON_DPAD_UP,		false },
	{ GAMEPAD_BUTTON_DOWN,	XINPUT_BUTTON_DPAD_DOWN,	false },
	
	{ GAMEPAD_BUTTON_SELECT, XINPUT_BUTTON_BACK,		false }
};




const float g_sDeadZone = 0.2;

void XBoxPad::Init(uint32 uPadNum)
{
	m_xinputID = uPadNum;
}


bool XBoxPad::IsConnected()
{
	return glfwJoystickPresent(m_xinputID);
}

void XBoxPad::Update()
{
	int count;
	
	
	m_uButtonsPrevDown = m_uButtonsDown;
	m_uButtonsDown = 0;
	
	const float * pAxis = glfwGetJoystickAxes(m_xinputID, &count);
	
	if (pAxis)
	{
		for(int i=0; i<sizeof(g_axisMappings)/sizeof(ControlMapping); i++)
		{
			float v = pAxis[g_axisMappings[i].uXInputID] * (g_axisMappings[i].bReverse ? -1. : 1.);
			
			if (fabs(v) < g_sDeadZone)
				v = 0.0f;
			
			m_fAxisValues[g_axisMappings[i].uAbstractID] = v;
			
		}
		
		if ( pAxis[XINPUT_AXIS_LT] > -0.5 )
			m_uButtonsDown |= GAMEPAD_BUTTON_ZL;
		
		if ( pAxis[XINPUT_AXIS_RT] > -0.5 )
			m_uButtonsDown |= GAMEPAD_BUTTON_ZR;
		
	}
	
	m_fAxisValues[GAMEPAD_AXIS_SCROLL_X] = g_mouseScrollX;
	m_fAxisValues[GAMEPAD_AXIS_SCROLL_Y] = g_mouseScrollY;
	
	
	const unsigned char* pButtons = glfwGetJoystickButtons(m_xinputID, &count);

	
	
	if (pButtons)
	{
		for(int i=0; i<sizeof(g_buttonMappings)/sizeof(ControlMapping); i++)
		{
			if (g_buttonMappings[i].uXInputID < count)
			{
				if ( pButtons[ g_buttonMappings[i].uXInputID ] )
				{
					m_uButtonsDown |= g_buttonMappings[i].uAbstractID;
				}
			}
		}
	}
	
	
	m_mGyro.MakeRotateYPR(g_mouseX * Math::pi*2, g_mouseY * Math::pi, 0.0);
	
	
		
	
}

}

