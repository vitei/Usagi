/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#define WIN32_LEAN_AND_MEAN
//#include <Windows.h>
#include <stdlib.h>
#include <assert.h>
#include "openglcontext.h"
//#include "glew/include/gl/glew.h"
#include "window.h"
#include "glfw3.h"

#include <iostream>
#include <chrono>
#include <thread>

OpenGLContext::OpenGLContext()
{
}

OpenGLContext::~OpenGLContext()
{
}

void OpenGLContext::Init(const Window* pWindow)
{
	m_lastSwapTime = glfwGetTime();
}

void OpenGLContext::SetActive(const Window* pWindow)
{
    GLFWwindow *glfw = (GLFWwindow*)pWindow->GetDeviceContext();
    
    glfwMakeContextCurrent(glfw);
    
}

void OpenGLContext::Deactivate()
{
}

void OpenGLContext::Destroy()
{
}

void OpenGLContext::reset()
{
}


void OpenGLContext::PerformBufferSwap(const Window* pWindow)
{
    GLFWwindow *glfw = (GLFWwindow*)pWindow->GetDeviceContext();
    
	
	const double targetFrameRate = 30.0;
	
	double delta = glfwGetTime() - m_lastSwapTime;
	
	if (delta < 1.0/targetFrameRate)
	{
		std::chrono::milliseconds dura( (long) (((1.0/targetFrameRate) - delta) * 1000)  );
		std::this_thread::sleep_for( dura );
		
		
	}
	
	m_lastSwapTime = glfwGetTime();
	
	glfwSwapBuffers(glfw);

	
}