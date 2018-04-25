/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Maths/MathUtil.h"
#include "PSPad.h"
#include "glfw3.h"

// NOTE: These are defined in the game-specific code, so we need to declare
//       these externs outside of the usg namespace.
extern float g_mouseX,g_mouseY;
extern bool g_mouseDown;
extern float g_mouseScrollX,g_mouseScrollY;

namespace usg
{

enum INPUT_AXIS
{
	INPUT_AXIS_LEFT_THUMB_X	=	0,
	INPUT_AXIS_LEFT_THUMB_Y	=	1,
	INPUT_AXIS_RIGHT_THUMB_X	=	2,
	INPUT_AXIS_RIGHT_THUMB_Y	=	3,
	
	INPUT_AXIS_L2				=	4,
	INPUT_AXIS_R2				=	5
	
};

enum INPUT_BUTTONS
{
	INPUT_BUTTON_DPAD_UP		=	14,
	INPUT_BUTTON_DPAD_RIGHT		=	15,
	INPUT_BUTTON_DPAD_DOWN		=	16,
	INPUT_BUTTON_DPAD_LEFT		=	17,

	INPUT_BUTTON_L1				=	4,
	INPUT_BUTTON_R1				=	5,

	INPUT_BUTTON_SHARE			=	8,
	INPUT_BUTTON_OPTIONS		=	9,

	INPUT_BUTTON_PS				=	12,
	
	INPUT_BUTTON_SQUARE			=	0,
	INPUT_BUTTON_X				=	1,
	INPUT_BUTTON_O				=	2,
	INPUT_BUTTON_TRIANGLE		=	3,
	
	INPUT_BUTTON_TOUCHPAD		=	13,
	
	INPUT_BUTTON_THUMB_L		=	10,
	INPUT_BUTTON_THUMB_R		=	11

};

struct ControlMapping
{
	uint32	uAbstractID;
	uint32	uXInputID;
	bool	bReverse;
};

static const	ControlMapping g_axisMappings[] =
{
	{ GAMEPAD_AXIS_LEFT_X,		INPUT_AXIS_LEFT_THUMB_X,	false },
	{ GAMEPAD_AXIS_LEFT_Y,		INPUT_AXIS_LEFT_THUMB_Y,	true },
	{ GAMEPAD_AXIS_RIGHT_X,		INPUT_AXIS_RIGHT_THUMB_X,	false },
	{ GAMEPAD_AXIS_RIGHT_Y,		INPUT_AXIS_RIGHT_THUMB_Y,	true }
};

static const	ControlMapping g_buttonMappings[] =
{
	{ GAMEPAD_BUTTON_B,		INPUT_BUTTON_X,			false },
	{ GAMEPAD_BUTTON_A,		INPUT_BUTTON_O,			false },
	{ GAMEPAD_BUTTON_X,		INPUT_BUTTON_SQUARE,	false },
	{ GAMEPAD_BUTTON_Y,		INPUT_BUTTON_TRIANGLE,	false },
	{ GAMEPAD_BUTTON_L,		INPUT_BUTTON_L1,		false },
	{ GAMEPAD_BUTTON_R,		INPUT_BUTTON_R1,		false },
//	{ GAMEPAD_BUTTON_HOME,	INPUT_BUTTON_PS,		false },
	{ GAMEPAD_BUTTON_LEFT,	INPUT_BUTTON_DPAD_LEFT,	false },
	{ GAMEPAD_BUTTON_RIGHT,	INPUT_BUTTON_DPAD_RIGHT,false },
	{ GAMEPAD_BUTTON_UP,	INPUT_BUTTON_DPAD_UP,	false },
	{ GAMEPAD_BUTTON_DOWN,	INPUT_BUTTON_DPAD_DOWN,	false },
	
	{ GAMEPAD_BUTTON_SELECT, INPUT_BUTTON_SHARE,	false },
	{ GAMEPAD_BUTTON_START, INPUT_BUTTON_PS,	false },
	
	{ GAMEPAD_BUTTON_THUMB_L, INPUT_BUTTON_THUMB_L,	false },
	{ GAMEPAD_BUTTON_THUMB_R, INPUT_BUTTON_THUMB_R,	false }

	
};




const float g_sDeadZone = 0.2;

void PSPad::Init(uint32 uPadNum)
{
	m_inputID = uPadNum;
}


bool PSPad::IsConnected()
{
	return glfwJoystickPresent(m_inputID);
}

void PSPad::Update()
{
	int count;
	
	
	m_uButtonsPrevDown = m_uButtonsDown;
	m_uButtonsDown = 0;
	
	const float * pAxis = glfwGetJoystickAxes(m_inputID, &count);
	
	if (pAxis)
	{
		for(int i=0; i<sizeof(g_axisMappings)/sizeof(ControlMapping); i++)
		{
			float v = pAxis[g_axisMappings[i].uXInputID] * (g_axisMappings[i].bReverse ? -1. : 1.);
			
			if (fabs(v) < g_sDeadZone)
				v = 0.0f;
			
			m_fAxisValues[g_axisMappings[i].uAbstractID] = v;
			
		}
		
		if ( pAxis[INPUT_AXIS_L2] > -0.5 )
			m_uButtonsDown |= GAMEPAD_BUTTON_ZL;
		
		if ( pAxis[INPUT_AXIS_R2] > -0.5 )
			m_uButtonsDown |= GAMEPAD_BUTTON_ZR;
		
	}
	
	
	
	m_fAxisValues[GAMEPAD_AXIS_SCROLL_X] = g_mouseScrollX;
	m_fAxisValues[GAMEPAD_AXIS_SCROLL_Y] = g_mouseScrollY;
	
	
	m_fAxisValues[GAMEPAD_AXIS_POINTER_X] = g_mouseX + 0.5f;
	m_fAxisValues[GAMEPAD_AXIS_POINTER_Y] = g_mouseY * 2.0f;
	m_vTouchPos.x = m_fAxisValues[GAMEPAD_AXIS_POINTER_X];
	m_vTouchPos.y = 1.0f - m_fAxisValues[GAMEPAD_AXIS_POINTER_Y];
	
	m_bScreenTouch = g_mouseDown;
	
	
//	DEBUG_PRINT("%d, %f, %f\n", g_mouseDown, m_vTouchPos.x, m_vTouchPos.y);
	
	
	const unsigned char* pButtons = glfwGetJoystickButtons(m_inputID, &count);

	
	
	if (pButtons)
	{
		//for(int i=0; i<count; i++)
		//	if (pButtons[i])
		//		DEBUG_PRINT("%d = %d\n",i,pButtons[i]);
		

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
