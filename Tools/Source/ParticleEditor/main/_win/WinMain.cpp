/***************************************************************************
//	Filename: WinMain.cpp
//	Description: Entry point for Windows based systems
//	Must be included in all projects due to entry point requirements,
//	DO NOT ADD game specific code!
*****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/HID/Input.h"
#include "Engine/Graphics/Device/GFXDevice.h"
#include API_HEADER(Engine/Graphics/Device, GFXDevice_ps.h)
#include OS_HEADER(Engine/Core, WinUtil.h)
#include OS_HEADER(Engine/HID, Input_ps.h)
#include <stdlib.h>

bool	 g_bFullScreen = false;
uint32 g_uWindowWidth = 1570;
uint32 g_uWindowHeight = 650;

using namespace usg;

WINDOWPLACEMENT g_OldWindowPlacement = { sizeof(g_OldWindowPlacement) };

// Declare the games main function
namespace usg
{
bool GameMain(const char** dllModules, uint32 uModuleCount);
bool GameExit();
void GameMessage(const uint32 messageID, const void* const pParameters);
}


static void ToggleFullScreen(const HWND hwnd)
{
	const DWORD dwStyle = GetWindowLong(hwnd, GWL_STYLE);

	GameMessage('ONSZ', nullptr);

	if (dwStyle & WS_OVERLAPPEDWINDOW)
	{
		MONITORINFO monitorInfo = { sizeof(monitorInfo) };

		if (GetWindowPlacement(hwnd, &g_OldWindowPlacement) && GetMonitorInfo(MonitorFromWindow(hwnd, MONITOR_DEFAULTTOPRIMARY), &monitorInfo))
		{
			SetWindowLong(hwnd, GWL_STYLE, (dwStyle & ~WS_OVERLAPPEDWINDOW));

			SetWindowPos(hwnd,
				HWND_TOP,
				monitorInfo.rcMonitor.left, monitorInfo.rcMonitor.top,
				(monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left),
				(monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top),
				(SWP_NOOWNERZORDER | SWP_FRAMECHANGED));

		}
	}
	else
	{
		const UINT uFlags = SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_FRAMECHANGED;
		SetWindowLong(hwnd, GWL_STYLE, (dwStyle | WS_OVERLAPPEDWINDOW));
		SetWindowPlacement(hwnd, &g_OldWindowPlacement);
		SetWindowPos(hwnd, nullptr, 0, 0, 0, 0, uFlags);
	}
	GameMessage('WSZE', nullptr);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	static const WPARAM VK_RESERVED = 0xFF;
	static bool bIsSizing = false;

	switch (msg)
	{
	case WM_CREATE:
		break;

	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		BeginPaint(hwnd, &ps);
		EndPaint(hwnd, &ps);
	}
	break;

	case WM_KEYDOWN:  // keyboard key
	{
		if (wparam < VK_RESERVED)
		{
			Input::GetPlatform().RegisterKeyStateChange((uint8)wparam, true);
		}
	}
	break;

	case WM_KEYUP:
	{
		if (wparam < VK_RESERVED)
		{
			Input::GetPlatform().RegisterKeyStateChange((uint8)wparam, false);

			if (wparam == VK_F11)
			{
				ToggleFullScreen(hwnd);
			}
		}

	}
	break;

	case WM_SIZE:
	{
		if (wparam == SIZE_MINIMIZED)
		{
			GameMessage('WMIN', nullptr);
		}
		else if (!bIsSizing)
		{
			GameMessage('ONSZ', nullptr);
			GameMessage('WSZE', nullptr);
		}
	}
	break;

	case WM_ENTERSIZEMOVE:
	{
		bIsSizing = true;
	}
	break;
	case WM_EXITSIZEMOVE:
	{
		if (wparam != SIZE_MINIMIZED)
		{
			GameMessage('ONSZ', nullptr);
			GameMessage('WSZE', nullptr);
		}

		bIsSizing = false;
	}
	break;

	case WM_CHAR:
	{
		Input::GetPlatform().RegisterInputChar((uint16)wparam);
	}
	break;

	case WM_MOUSEMOVE:
		break;

	case WM_DEVICECHANGE:
	{
		Input::RenumberGamepads();
	}
	break;

	case WM_LBUTTONDOWN:
	{
		Input::GetPlatform().RegisterMouseButtonChange(0, true);
	}
	break;

	case WM_LBUTTONUP:
	{
		Input::GetPlatform().RegisterMouseButtonChange(0, false);
	}
	break;

	case WM_RBUTTONDOWN:
	{
		Input::GetPlatform().RegisterMouseButtonChange(1, true);
	}
	break;

	case WM_RBUTTONUP:
	{
		Input::GetPlatform().RegisterMouseButtonChange(1, false);
	}
	break;

	case WM_MBUTTONDOWN:
	{
		Input::GetPlatform().RegisterMouseButtonChange(2, true);
	}
	break;

	case WM_MBUTTONUP:
	{
		Input::GetPlatform().RegisterMouseButtonChange(2, false);
	}
	break;

	case WM_DESTROY:
		GameExit();
		break;

	case WM_ERASEBKGND:
		return 0;

	default:
		break;
	}

	return (DefWindowProc(hwnd, msg, wparam, lparam));

}

int WINAPI WinMain(	HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpcmdline, int ncmdshow)
{
	WINUTIL::Init(hInstance);
	const char* szUsagi = getenv("USAGI_DIR");
	if(szUsagi != NULL)
	{
		char path[512];
		sprintf_s(path, 512, "%s\\..\\_romfiles\\win", szUsagi);
		SetCurrentDirectory(path);
	}
	else
	{
		DEBUG_PRINT("USAGI_DIR enviroment variable not located, please set it!!!!");
	}

	usg::DisplaySettings settings;
	settings.uX = 0; settings.uY = 0; settings.uWidth = g_uWindowWidth; settings.uHeight = g_uWindowHeight;
	settings.bWindowed = !g_bFullScreen; settings.hardwareHndl = NULL;
	const char * const p_string = "Virtual Screen";
	str::Copy(settings.name, p_string, sizeof(settings.name));
	WindHndl hndl = WINUTIL::CreateDisplayWindow(WindowProc, "Usagi", &settings, false);
	usg::Input::GetPlatform().RegisterHwnd(0, hndl);

#ifndef USE_VULKAN
	GFXDevice_ps::InitOGLContext(hndl, 1);
#endif

	GameMain(nullptr, 0);

	return 0;
}
 
