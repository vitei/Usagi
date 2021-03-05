/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Windows specific hardware level functions
*****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Core/_win/WinUtil.h"
#include "Engine/HID/Input.h"
#include "Engine/HID/_win/Input_ps.h"

namespace usg
{
	void GameMessage(const uint32 messageID, const void* const pParameters);
}

namespace WINUTIL
{
	static HINSTANCE g_hInst;
	static WindHndl g_wndHndl = NULL;
	static bool g_bInFocus = true;

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

	void GetSettings(const usg::DisplayMode* pDisplaySettings, DWORD& dwStyle, DWORD& dwExStyle, RECT& mRcClient)
	{
		dwStyle &= (WS_VISIBLE| WS_DISABLED);	// Flags we need to continue to obey
		if (!pDisplaySettings->bWindowed)
		{
			dwExStyle = WS_EX_APPWINDOW;
			dwStyle |= WS_POPUP;
			dwStyle &= ~WS_OVERLAPPEDWINDOW;
		}
		else
		{
			dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
			// Forcefully disable sys menu so games don't pause when you press alt
			dwStyle |= WS_OVERLAPPEDWINDOW;
			if (!pDisplaySettings->bMenu)
			{
				dwStyle &= (~WS_SYSMENU);
			}
		}

		dwStyle |= WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

		mRcClient.top = pDisplaySettings->screenDim.x;
		mRcClient.left = pDisplaySettings->screenDim.y;
		mRcClient.right = pDisplaySettings->screenDim.x + pDisplaySettings->screenDim.width;
		mRcClient.bottom = pDisplaySettings->screenDim.y + pDisplaySettings->screenDim.height;

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
	}

	WindHndl CreateDisplayWindow(WNDPROC wndProc, const char* szName, const usg::DisplayMode* pDisplaySettings, bool bHidden)
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

		DWORD dwStyle = 0;
		DWORD dwExStyle = 0;
		RECT mRcClient;

		GetSettings(pDisplaySettings, dwStyle, dwExStyle, mRcClient);

		if (!(hndl = CreateWindowEx(dwExStyle, szName, // class
			szName,	     // title
			dwStyle,
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
			DevMode.dmPelsWidth = pDisplaySettings->screenDim.width;   // X Resolution
			DevMode.dmPelsHeight = pDisplaySettings->screenDim.height;  // Y Resolution
			DevMode.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT;// | DM_BITSPERPEL;
			ChangeDisplaySettings(&DevMode, CDS_FULLSCREEN);
		}

		// We should really be caching all windows we own
		SetWindow(hndl);
		SetForegroundWindow(hndl);

		return hndl;
	}

	void UpdateWindow(const usg::DisplayMode* pDisplaySettings)
	{
		HWND hwnd = GetWindow();
		DWORD dwStyle = GetWindowLong(hwnd, GWL_STYLE);

		usg::GameMessage('ONSZ', nullptr);

		DWORD dwExStyle = 0;
		RECT mRcClient;

		GetSettings(pDisplaySettings, dwStyle, dwExStyle, mRcClient);

		if (!pDisplaySettings->bWindowed)
		{
			DEVMODE DevMode;
			memset(&DevMode, 0, sizeof(DEVMODE));
			DevMode.dmSize = sizeof(DEVMODE);
			DevMode.dmBitsPerPel = 32;
			DevMode.dmPelsWidth = pDisplaySettings->screenDim.width;   // X Resolution
			DevMode.dmPelsHeight = pDisplaySettings->screenDim.height;  // Y Resolution
			DevMode.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT;// | DM_BITSPERPEL;
			ChangeDisplaySettings(&DevMode, CDS_FULLSCREEN);

			SetWindowLong(hwnd, GWL_STYLE, dwStyle);
			SetWindowLong(hwnd, GWL_EXSTYLE, dwExStyle);

			// FIXME: Second monitor would be offset
			SetWindowPos(hwnd,
				HWND_TOP,
				mRcClient.left,
				mRcClient.top,
				mRcClient.right - mRcClient.left,
				mRcClient.bottom - mRcClient.top,
				(SWP_NOOWNERZORDER | SWP_FRAMECHANGED));
		}
		else
		{

			const UINT uFlags = SWP_NOMOVE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_FRAMECHANGED;
			
			ChangeDisplaySettings(NULL, CDS_FULLSCREEN);

			SetWindowLong(hwnd, GWL_STYLE, dwStyle);
			SetWindowLong(hwnd, GWL_EXSTYLE, dwExStyle);

			SetWindowPos(hwnd, nullptr,
				mRcClient.left,
				mRcClient.right,
				mRcClient.right - mRcClient.left,
				mRcClient.bottom - mRcClient.top,
				uFlags);
		}

		UpdateWindow(hwnd);
		ShowWindow(hwnd, SW_SHOWNORMAL);
		SetForegroundWindow(hwnd);


		usg::GameMessage('WSZE', nullptr);

	

	}

	void OnLostFocus()
	{
		HWND hwnd = GetWindow();
		const DWORD dwStyle = GetWindowLong(hwnd, GWL_STYLE);

		if (dwStyle& WS_POPUP)
		{
			// If we were full screen minimise us and restore the resolution
			ShowWindow(hwnd, SW_MINIMIZE);
			ChangeDisplaySettings(NULL, CDS_FULLSCREEN);
		}
	}

	void OnGainedFocus(uint32 uWidth, uint32 uHeight)
	{
		HWND hwnd = GetWindow();
		const DWORD dwStyle = GetWindowLong(hwnd, GWL_STYLE);

		if (dwStyle & WS_POPUP)
		{
			const UINT uFlags = SWP_NOMOVE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_FRAMECHANGED;
		
			DEVMODE DevMode;
			memset(&DevMode, 0, sizeof(DEVMODE));
			DevMode.dmSize = sizeof(DEVMODE);
			DevMode.dmBitsPerPel = 32;
			DevMode.dmPelsWidth = uWidth;
			DevMode.dmPelsHeight = uHeight;
			DevMode.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT;// | DM_BITSPERPEL;
			ChangeDisplaySettings(&DevMode, CDS_FULLSCREEN);

			SetWindowPos(hwnd, nullptr,
				0,
				0,
				uWidth,
				uHeight,
				uFlags);

			UpdateWindow(hwnd);
			ShowWindow(hwnd, SW_SHOWNORMAL);
			SetForegroundWindow(hwnd);
		}
	}

	void SetInFocus(bool bInFocus)
	{
		g_bInFocus = bInFocus;
	}
	bool GetInFocus()
	{
		return g_bInFocus;
	}

};
