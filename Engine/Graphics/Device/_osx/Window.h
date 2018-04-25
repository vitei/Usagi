/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef _USG_GRAPHICS_PC_WINDOW
#define _USG_GRAPHICS_PC_WINDOW

#include "Engine/Common/Common.h"

class GLFWwindow;

class Window
{
public:
	Window();
	~Window();

	bool Init(const char* szName);
	int GetWidth() { return m_width; }
	int GetHeight() { return m_height; }
	void* GetWindowHandle() const;
	void* GetDeviceContext() const;
	bool IsFullscreen() { return m_bFullScreen; }
	bool IsValid()	{ return m_bValid; }

	void SetWidth(int val) { m_width = val; }
	void SetHeight(int val) { m_height = val; }
	void SetFullscreen(bool val) { m_bFullScreen = val; }

	void SetWindow(unsigned int uX, unsigned int uY);

    float GetPixelScaleX() const;
    float GetPixelScaleY() const;
	
	static void GetMaxDisplayResolution(unsigned int uDisplay, unsigned int &widthOut, unsigned int &heightOut);
	

private:
	int			m_width;
	int			m_height;
	bool		m_bFullScreen;

    GLFWwindow * m_glfw;

	int			m_origWidth;
	int			m_origHeight;
	bool		m_bValid;
    
    float      m_fPixelScaleX;
    float      m_fPixelScaleY;

};


#endif