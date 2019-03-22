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
uint32 g_uWindowWidth = 1800;
uint32 g_uWindowHeight = 1100;

using namespace usg;

// Declare the games main function
namespace usg
{
bool GameMain();
bool GameExit();
Input* GameGetInput();
}




LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	switch(msg)
	{	
	case WM_CREATE: 
		break;
	case WM_PAINT:
		{
			PAINTSTRUCT		ps;
			BeginPaint(hwnd,&ps);	 
			EndPaint(hwnd,&ps);
			break;
		}
	case WM_KEYDOWN:  // keyboard key
	{
		Input::GetPlatform().RegisterKeyStateChange((uint8)wparam, true);
		break;
	}
	case WM_KEYUP:
	{
		Input::GetPlatform().RegisterKeyStateChange((uint8)wparam, false);
		break;
	}
	case WM_CHAR:
	{			
		Input::GetPlatform().RegisterInputChar((uint16)wparam);
		break;
	}
	case WM_MOUSEWHEEL:
	{
		Input::GetPlatform().RegisterMouseWheel(GET_WHEEL_DELTA_WPARAM(wparam));
		break;
	}
	case WM_LBUTTONDOWN:
	{
		Input::GetPlatform().RegisterMouseButtonChange(0, true);
		break;
	}
	case WM_LBUTTONUP:
	{
		Input::GetPlatform().RegisterMouseButtonChange(0, false);
		break;
	}
	case WM_RBUTTONDOWN:
	{
		Input::GetPlatform().RegisterMouseButtonChange(1, true);
		break;
	}
	case WM_RBUTTONUP:
	{
		Input::GetPlatform().RegisterMouseButtonChange(1, false);
		break;
	}
	case WM_MBUTTONDOWN:
	{
		Input::GetPlatform().RegisterMouseButtonChange(2, true);
		break;
	}
	case WM_MBUTTONUP:
	{
		Input::GetPlatform().RegisterMouseButtonChange(2, false);
		break;
	}
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

	GameMain();

	return 0;
}
 
