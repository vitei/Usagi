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

bool	 g_bFullScreen = false;
uint32 g_uWindowWidth = 1280;
uint32 g_uWindowHeight = 720;


WINDOWPLACEMENT g_OldWindowPlacement = { sizeof(g_OldWindowPlacement) };

using namespace usg;

// Declare the games main function
namespace usg
{
	bool GameMain();
	bool GameExit();
	void GameMessage(const uint32 messageID, const void* const pParameters);

}

static void ToggleFullScreen(const HWND hwnd)
{
	const DWORD dwStyle = GetWindowLong(hwnd, GWL_STYLE);

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
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	static const WPARAM VK_RESERVED = 0xFF;

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
			if(wparam != SIZE_MINIMIZED)
			{
				GameMessage('WSZE', nullptr);
			}
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
			PostQuitMessage(0);
			GameExit();
			break;

		case WM_ERASEBKGND:
			return 0;

		default:
			break;
	}

	return (DefWindowProc(hwnd, msg, wparam, lparam));

}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpcmdline, int ncmdshow)
{
	WINUTIL::Init(hInstance);

	usg::DisplaySettings settings;
	settings.uX = 0; settings.uY = 0; settings.uWidth = g_uWindowWidth; settings.uHeight = g_uWindowHeight;
	settings.bWindowed = !g_bFullScreen; settings.hardwareHndl = nullptr;
	str::Copy(settings.name, "Virtual Screen", sizeof(settings.name));
	const WindHndl hndl = WINUTIL::CreateDisplayWindow("Usagi", &settings, false);

#ifndef USE_VULKAN
	GFXDevice_ps::InitOGLContext(hndl, 1);
#endif

	usg::OS::Initialize();
	GameMain();
	usg::OS::ShutDown();

	return 0;
}