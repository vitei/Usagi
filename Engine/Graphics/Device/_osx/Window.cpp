/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#define WIN32_LEAN_AND_MEAN
//#include <Windows.h>
#include <stdlib.h>
#include <Stdio.h>
#include <String.h>
#include "glfw3.h"
#include "window.h"

#include "Engine/Common/Common.h"
#include "Engine/Maths/MathUtil.h"

//#include "Debug.h"
#include "OpenGLIncludes.h"

#include <OpenGL/OpenGL.h>

namespace usg
{
	bool GameExit();
}

bool g_keysDown[GLFW_KEY_LAST];
float g_mouseX=0,g_mouseY=0;
bool g_mouseDown;
bool g_mouseActive;
float g_mouseScrollX=0,g_mouseScrollY=0;

/*
LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

struct Window::PIMPL
{
	RECT		mRcClient;
	HWND		mHwnd;
	HDC			mHDC;
};
 */

Window::Window():
m_width(640),
m_height(480),
m_bFullScreen(false),
m_bValid(false)
//m_pImpl(NULL)
{

    g_mouseX = 0.0f;
    g_mouseY = 0.0f;
    
    for(int i=0; i<GLFW_KEY_LAST; i++)
        g_keysDown[i] = false;
    
    g_mouseDown = false;
    g_mouseActive = true;
}



static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key > 0 && key < GLFW_KEY_LAST)
    {
        if (action == GLFW_PRESS)
            g_keysDown[key] = true;
        else if (action == GLFW_RELEASE)
            g_keysDown[key] = false;
    }
    
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        usg::GameExit();
        glfwSetWindowShouldClose(window, GL_TRUE);
    }
    
}

static void cursor_callback(GLFWwindow *window, double x, double y)
{
    if (g_mouseActive)
    {
        int width, height;
        glfwGetWindowSize(window, &width, &height);
		
		// NASTY
		if (y > 480)
		{
			x -= 80.0;
			width = 640.0;
		}
		
        g_mouseX = (float)(x / (double)width) - 0.5;
        g_mouseY = (float)(y / (double)height) - 0.5;
    }
    
}

static void mousebutton_callback(GLFWwindow *window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT)
    {
        if (action == GLFW_PRESS)
        {
            g_mouseDown = true;
            //g_mouseActive = !g_mouseActive;
			
			/*
            if (!g_mouseActive)
            {
                g_mouseX = 0.0;
                g_mouseY = 0.0;
            }
			 */
        }else if (action == GLFW_RELEASE)
            g_mouseDown = false;
        
    }
}

static void scroll_callback(GLFWwindow* window, double xpos, double ypos)
{
	g_mouseScrollX = (float)xpos;
	g_mouseScrollY = (float)ypos;
}



bool Window::Init(const char* title)
{
	
	
	
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	
	
	
	int count;
	
	GLFWmonitor** pMonitors = glfwGetMonitors(&count);
	GLFWmonitor* pDK1 = NULL;
	GLFWmonitor* pDK1HD = NULL;

	printf("Found %d monitors:\n",count);
	for(int i=0; i<count; i++)
	{
		const char* pName = glfwGetMonitorName(pMonitors[i]);
		
		printf("  %d : %s \n",i,pName);
		
		if (strcmp(pName,"Rift DK")==0)
			pDK1 = pMonitors[i];
		else if (strcmp(pName,"Rift DK HD")==0)
			pDK1HD = pMonitors[i];
		
	}
	
	GLFWmonitor* pMonitor=NULL;
	
	if (pDK1)
	{
		printf("Found Oculus Rift DK1, using Fullscreen.\n");
		m_width = 1280;
		m_height = 800;
		pMonitor = pDK1;
	}else if (pDK1HD)
	{
		printf("Found Oculus Rift DK1 HD, using Fullscreen.\n");
		m_width = 1920;
		m_height = 1080;
		//m_width = 1280;
		//m_height = 800;
		pMonitor = pDK1HD;
	}
	
	
#if !defined(USE_VR) && !defined(USE_VR_SIMPLE)
	if (m_bFullScreen)
	{
//		pMonitor = pMonitors[0];
		//pMonitor = NULL;
		//glfwWindowHint(GLFW_DECORATED, 0);
	}else
	{
		pMonitor = NULL;
	}
	/*
	m_width = 1400;
	m_height = 480;
	 */
	
	
	
#endif
	
//	pMonitor = pMonitors[0];



	
	printf("Creating glfw window %d x %d\n",m_width,m_height);
    
    m_glfw = glfwCreateWindow(m_width, m_height, title, pMonitor, NULL );
	
    ASSERT(m_glfw);
    
    glfwMakeContextCurrent(m_glfw);
	
	
	/*
	CGLError err;
	CGLContextObj ctx = CGLGetCurrentContext();
	
	err =  CGLEnable( ctx, kCGLCEMPEngine);
	
	if (err != kCGLNoError )
		printf("OpenGL multithreading ON error\n");
	else
		printf("OpenGL multithreading ON\n");
	*/
	
	//glfwSwapInterval(0);
	
	
    glViewport(0, 0, m_width, m_height);
    glClear(GL_COLOR_BUFFER_BIT);

    glfwSwapBuffers(m_glfw);
    
    glfwSetKeyCallback(m_glfw, key_callback);
    
    glfwSetCursorPosCallback(m_glfw, cursor_callback);
    
    glfwSetMouseButtonCallback(m_glfw, mousebutton_callback);
    
	glfwSetScrollCallback(m_glfw, scroll_callback);
	
    
    
    int fbWidth, fbHeight;
    glfwGetFramebufferSize(m_glfw, &fbWidth,&fbHeight);
    
	printf("FB size: %d x %d\n",fbWidth,fbHeight);

		
    m_fPixelScaleX = (float)fbWidth / (float)m_width;
    m_fPixelScaleY = (float)fbHeight / (float)m_height;
    
    printf("Pixel scale : %f x %f\n",m_fPixelScaleX,m_fPixelScaleY);
    
    m_bValid = true;
    
	return true;
}

Window::~Window()
{
}

void Window::SetWindow(unsigned int uX, unsigned int uY)
{
    ASSERT(m_glfw);
    
    glfwSetWindowPos(m_glfw, uX, uY);
    

}

void* Window::GetDeviceContext() const
{
    return m_glfw;
}

void* Window::GetWindowHandle() const
{
    return  NULL;
}

float Window::GetPixelScaleX() const
{
    return m_fPixelScaleX;
}
float Window::GetPixelScaleY() const
{
    return m_fPixelScaleY;
}



void Window::GetMaxDisplayResolution(unsigned int uDisplay, unsigned int &uWidthOut, unsigned int &uHeightOut)
{

	int count;

	GLFWmonitor** pMonitors = glfwGetMonitors(&count);

	ASSERT(uDisplay < count);

	GLFWmonitor* pMon = pMonitors[uDisplay];

	
	const GLFWvidmode * mode = glfwGetVideoMode(pMon);
	
	uWidthOut = mode->width;
	uHeightOut = mode->height;
}
