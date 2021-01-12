/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Windows specific hardware level functions
*****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Core/_win/WinUtil.h"
#include "Engine/HID/Input.h"
#include "Engine/HID/_win/Input_ps.h"

namespace WINUTIL
{
	static HINSTANCE g_hInst;
	static WindHndl g_wndHndl = NULL;

	bool Init(HINSTANCE hInst)
	{
		SetProcessDPIAware();
		g_hInst = hInst;
		return true;
	}

	HINSTANCE GetInstanceHndl()
	{
		return g_hInst;
	}

	WindHndl GetWindow()
	{
		return g_wndHndl;
	}


	void SetWindow(WindHndl hndl)
	{
		g_wndHndl = hndl;
	}

	WindHndl CreateDisplayWindow(WNDPROC wndProc, const char* szName, const usg::DisplaySettings* pDisplaySettings, bool bHidden)
	{
		WindHndl hndl;
		WNDCLASS		winclass;	// this will hold the class we create

		winclass.style = CS_HREDRAW | CS_VREDRAW;
		winclass.lpfnWndProc = wndProc;
		winclass.cbClsExtra = 0;
		winclass.cbWndExtra = 0;
		winclass.hInstance = WINUTIL::GetInstanceHndl();
		winclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
		winclass.hCursor = LoadCursor(NULL, IDC_ARROW);
		winclass.hbrBackground = NULL;
		winclass.lpszMenuName = NULL;
		winclass.lpszClassName = szName;
		//winclass.hIconSm			= LoadIcon(NULL, IDI_WINLOGO);

		if (!RegisterClass(&winclass))
		{
			return false;
		}

		DWORD dwStyle;
		DWORD dwExStyle;

		if (!pDisplaySettings->bWindowed)
		{
			dwExStyle = WS_EX_APPWINDOW;
			dwStyle = WS_POPUP;
		}
		else
		{
			dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
			// Forcefully disable sys menu so games don't pause when you press alt
			dwStyle = WS_OVERLAPPEDWINDOW;
			if (!pDisplaySettings->bMenu)
			{
				dwStyle &= (~WS_SYSMENU);
			}
		}

		// Setup a RECT to describe the requested client area size
		RECT mRcClient;
		mRcClient.top = pDisplaySettings->uY;
		mRcClient.left = pDisplaySettings->uX;
		mRcClient.right = pDisplaySettings->uWidth;   // client area width
		mRcClient.bottom = pDisplaySettings->uHeight;   // client area height

												// Adjust rcClient for the window borders, given the window style
		AdjustWindowRectEx(&mRcClient, dwStyle, FALSE, dwExStyle);
		if (mRcClient.top < 0)
		{
			mRcClient.bottom -= mRcClient.top;
			mRcClient.top = 0;
		}
		if (mRcClient.left < 0)
		{
			mRcClient.right -= mRcClient.left;
			mRcClient.left = 0;
		}

		if (!(hndl = CreateWindowEx(dwExStyle, szName, // class
			szName,	     // title
			dwStyle | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
			mRcClient.left,
			mRcClient.top,
			mRcClient.right - mRcClient.left,
			mRcClient.bottom - mRcClient.top,
			//m_width,
			//m_height,
			NULL,
			NULL,
			WINUTIL::GetInstanceHndl(),
			NULL)))
		{
			int error = GetLastError();
			return false;
		}

		ShowWindow(hndl, bHidden ? SW_HIDE : SW_SHOW);
		UpdateWindow(hndl);


		HDC hdc = GetDC(hndl);

		// set the pixel format for the DC
		PIXELFORMATDESCRIPTOR pfd =
		{
			sizeof(PIXELFORMATDESCRIPTOR),
			1,
			PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,    //Flags
			PFD_TYPE_RGBA,			// RGBA (not palette)
			32,						// Color depth
			0, 0, 0, 0, 0, 0,
			0,
			0,
			0,
			0, 0, 0, 0,
			32,						// Depth buffer
			0,						// Stencil buffer
			0,						// Aux buffers
			PFD_MAIN_PLANE,
			0,
			0, 0, 0
		};

		int format = ChoosePixelFormat(hdc, &pfd);
		SetPixelFormat(hdc, format, &pfd);

		if (!pDisplaySettings->bWindowed)
		{
			DEVMODE DevMode;
			memset(&DevMode, 0, sizeof(DEVMODE));
			DevMode.dmSize = sizeof(DEVMODE);
			DevMode.dmBitsPerPel = 32;
			DevMode.dmPelsWidth = pDisplaySettings->uWidth;   // X Resolution
			DevMode.dmPelsHeight = pDisplaySettings->uHeight;  // Y Resolution
			DevMode.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL;
			ChangeDisplaySettings(&DevMode, CDS_FULLSCREEN);
		}

		// We should really be caching all windows we own
		SetWindow(hndl);
		return hndl;
	}

};
