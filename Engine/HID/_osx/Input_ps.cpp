/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
//#include OS_HEADER(Engine/HID, VirtualGamepad.h)
#include "Input_ps.h"
#include <string.h>
#include "glfw3.h"

// NOTE: These are defined in the game-specific code, so we need to declare
//       these externs outside of the usg namespace.
extern bool g_keysDown[GLFW_KEY_LAST];
extern float g_mouseX,g_mouseY;

namespace usg
{

Input_ps::Input_ps()
{
	m_uGamepads = 0;
}

Input_ps::~Input_ps()
{

}

static void error_callback(int error, const char* description)
{
    fputs(description,stderr);
}

void Input_ps::Init()
{
    static bool glfwInited=false;
    
    if (!glfwInited)
    {
        glfwSetErrorCallback(error_callback);
        if (!glfwInit())
            ASSERT(false);
        glfwInited = true;
    }
    
	
	
    for(int i=0; i<4; i++)
    {
        const char *name = glfwGetJoystickName(i);
        
       if (strcmp(name,"Controller") == 0 || strcmp(name,"Xbox 360 Wired Controller")==0)
       {
           printf("Detected XBox controller %d\n",i);
           m_xboxPad.Init(i);
           m_gamepads[m_uGamepads++] = &m_xboxPad;
        }else if (strcmp(name,"Wireless Controller") == 0)
        {
            printf("Detected PlayStation controller %d\n",i);
            m_psPad.Init(i);
            m_gamepads[m_uGamepads++] = &m_psPad;
        }else if (name!=NULL)
		{
			printf("Detected unknown joystick type %s\n",name);
		}
    }

	// Always expect a keyboard.
	{
		printf("Detected Keyboard controller\n");
		m_keyboard.Init(this);
		m_gamepads[m_uGamepads++] = &m_keyboard;
	}
	
	
#ifdef USE_VR
	m_ovrSensor.InitHMD();
	m_ovrSensor.Init();
	m_gamepads[m_uGamepads++] = &m_ovrSensor;
#endif
	
	
	
 
}

void Input_ps::Update()
{
	for(uint32 i=0; i<m_uGamepads; i++)
	{
		m_gamepads[i]->Update();
	}
}


void Input_ps::RegisterMouseMove(uint32 uAxis, long move)
{
	
}


bool Input_ps::GetKeyDown(int uKey) const
{
	return g_keysDown[uKey];
}

float Input_ps::GetMouseX() const
{
    return g_mouseX ;
}
float Input_ps::GetMouseY() const
{
    return g_mouseY;
}


bool	Input_ps::GetMouseButtonDown(uint32 uButton) const
{
//	return m_mouseButtonsDown[uButton];
    return false;
}

}
