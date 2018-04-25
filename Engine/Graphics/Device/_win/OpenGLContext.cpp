/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include <Windows.h>
#include <stdlib.h>
#include <assert.h>
#include "openglcontext.h"
#include "GL/glew.h"
#include "GL/wglext.h"

namespace usg {

struct OpenGLContext::PIMPL
{
	HWND	m_hWnd;
	HDC		m_hDC;
	HGLRC	m_hRC;
};

OpenGLContext::OpenGLContext()
	:m_pImpl(nullptr)
{
}

OpenGLContext::~OpenGLContext()
{
	if(m_pImpl)
	{
		Destroy();
		free(m_pImpl);
		m_pImpl = nullptr;
	}
}

void OpenGLContext::Init(WindHndl hndl, uint32 uSyncInterval)
{
	m_pImpl = (PIMPL*)malloc(sizeof(PIMPL));
	reset();

	m_pImpl->m_hWnd = hndl;
	m_pImpl->m_hDC = GetDC( m_pImpl->m_hWnd );

	// create the render context (RC)
	m_pImpl->m_hRC = wglCreateContext( m_pImpl->m_hDC );

	// make it the current render context
	wglMakeCurrent( m_pImpl->m_hDC, m_pImpl->m_hRC );

	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		ASSERT(false);
	}

	CreateGLXXContext(3, 3); // request OpenGL 3.3 context

	// Extension is supported, init pointers.
	typedef BOOL(WINAPI * PFNWGLSWAPINTERVALPROC)(int);
	PFNWGLSWAPINTERVALPROC wglSwapIntervalEXT = (PFNWGLSWAPINTERVALPROC) wglGetProcAddress("wglSwapIntervalEXT");

	if (wglSwapIntervalEXT)
	{
		wglSwapIntervalEXT(uSyncInterval);
	}
}

void OpenGLContext::SetActive(WindHndl hndl)
{
	if (m_pImpl->m_hWnd != hndl)
	{
		HDC hdc = ::GetDC(hndl);
		m_pImpl->m_hDC = hdc;
		m_pImpl->m_hWnd = hndl;
		// make it the current render context
		wglMakeCurrent(hdc, m_pImpl->m_hRC);
	}
}

void OpenGLContext::Deactivate()
{
	wglMakeCurrent(nullptr, m_pImpl->m_hRC);
}

void OpenGLContext::Destroy()
{
	if ( m_pImpl->m_hRC )
	{
		wglMakeCurrent( nullptr, nullptr );
		wglDeleteContext( m_pImpl->m_hRC );
	}
	if ( m_pImpl->m_hWnd && m_pImpl->m_hDC )
	{
		ReleaseDC( m_pImpl->m_hWnd, m_pImpl->m_hDC );
	}
	reset();
}

void OpenGLContext::reset()
{
	m_pImpl->m_hWnd = nullptr;
	m_pImpl->m_hDC = nullptr;
	m_pImpl->m_hRC = nullptr;
}

void OpenGLContext::CreateGLXXContext(const int iMajorVersion, const int iMinorVersion)
{
	PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB = reinterpret_cast<PFNWGLCHOOSEPIXELFORMATARBPROC>(wglGetProcAddress("wglChoosePixelFormatARB"));
	PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = reinterpret_cast<PFNWGLCREATECONTEXTATTRIBSARBPROC>(wglGetProcAddress("wglCreateContextAttribsARB"));

	if (wglChoosePixelFormatARB != nullptr)
	{
		const int iContextAttribs[] =
		{
			WGL_CONTEXT_MAJOR_VERSION_ARB,	iMajorVersion,
			WGL_CONTEXT_MINOR_VERSION_ARB,	iMinorVersion,
			WGL_CONTEXT_PROFILE_MASK_ARB,	WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
			0 // End of attributes list
		};

		const HGLRC hRC = wglCreateContextAttribsARB(m_pImpl->m_hDC, 0, iContextAttribs);
		if (hRC != nullptr)
		{
			wglMakeCurrent(nullptr, nullptr);
			wglDeleteContext(m_pImpl->m_hRC); // Delete old context
			wglMakeCurrent(m_pImpl->m_hDC, hRC);
			m_pImpl->m_hRC = hRC;
		}
	}
}

void OpenGLContext::PerformBufferSwap(HDC hdc)
{
	SwapBuffers( hdc );
}

}
