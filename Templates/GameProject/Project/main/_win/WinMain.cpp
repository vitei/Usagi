/***************************************************************************
//	Filename: WinMain.cpp
//	Description: Entry point for Windows based systems
//	Must be included in all projects due to entry point requirements,
//	DO NOT ADD game specific code!
*****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/HID/Input.h"
#include "Engine/Graphics/Device/GFXDevice.h"
#include "Engine/Core/OS.h"
#include API_HEADER(Engine/Graphics/Device, GFXDevice_ps.h)
#include OS_HEADER(Engine/Core, WinUtil.h)
#include OS_HEADER(Engine/HID, Input_ps.h)



WINDOWPLACEMENT g_OldWindowPlacement = { sizeof(g_OldWindowPlacement) };

using namespace usg;

// Declare the games main function
namespace usg
{
	bool GameMain(const char** dllModules, uint32 uModuleCount);
	bool GameExit();
	void GameMessage(const uint32 messageID, const void* const pParameters);

}


using namespace usg;

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

void UpdateCursorClamp(HWND hwnd)
{
	RECT screen;
	GetWindowRect(hwnd, &screen);
	ClipCursor(&screen);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	static const WPARAM VK_RESERVED = 0xFF;
	static bool bIsSizing = false;
	static bool bIsActive = false;
	static bool bIsActiveApp = false;
	static bool bHasFocus = false;

	switch (msg)
	{
		case WM_CREATE:
			break;

		case WM_ACTIVATEAPP:
		{
			bIsActiveApp = wparam ? true : false;
			break;
		}

		case WM_ACTIVATE:
		{
			if (wparam & WA_ACTIVE || wparam & WA_CLICKACTIVE)
			{
				bIsActive = true;
			}
			else
			{
				bIsActive = false;
			}
			break;
		}
		case WM_PAINT:
			{
				ValidateRect(hwnd, NULL);
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
						// Removing this as it's now on the menu
						//ToggleFullScreen(hwnd);
						//UpdateCursorClamp(hwnd);
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
			if (wparam == SIZE_RESTORED)
			{

			}
			if (wparam != SIZE_MINIMIZED)
			{
				GameMessage('ONSZ', nullptr);
				GameMessage('WSZE', nullptr);
			}
			UpdateCursorClamp(hwnd);
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

		case WM_XBUTTONDOWN:
		{
			DWORD fwButton = GET_XBUTTON_WPARAM(wparam);
			Input::GetPlatform().RegisterMouseButtonChange(2 + fwButton, true);
		}
		break;

		case WM_XBUTTONUP:
		{
			DWORD fwButton = GET_XBUTTON_WPARAM(wparam);
			Input::GetPlatform().RegisterMouseButtonChange(2 + fwButton, false);
		}
		break;

		case WM_MOUSEWHEEL:
			{
				Input::GetPlatform().RegisterMouseWheel((long)wparam);
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


	WINUTIL::SetInFocus((bIsActive&& bIsActiveApp));
	if( (!bIsActive || !bIsActiveApp) && bHasFocus )
	{
		ClipCursor(nullptr);
		bHasFocus = false;
	}
	else if((bIsActive && bIsActiveApp) && !bHasFocus)
	{
		UpdateCursorClamp(hwnd);
		ShowCursor(FALSE);
		bHasFocus = true;
	}

	return DefWindowProc(hwnd, msg, wparam, lparam);

}



int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpcmdline, int ncmdshow)
{
	WINUTIL::Init(hInstance);

	usg::OS::Initialize();
	GameMain(nullptr, 0);
	usg::OS::ShutDown();

	return 0;
}