/***************************************************************************
//	Filename: WinMain.cpp
//	Description: Entry point for Windows based systems
//	Must be included in all projects due to entry point requirements,
//	DO NOT ADD game specific code!
*****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/HID/Input.h"
#include OS_HEADER(Engine/Core, WinUtil.h)
#include OS_HEADER(Engine/HID, Input_ps.h)

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
	switch (msg)
	{
	case WM_CREATE:
		break;
	case WM_PAINT:
	{
		 PAINTSTRUCT		ps;
		 BeginPaint(hwnd, &ps);
		 EndPaint(hwnd, &ps);
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
	case WM_MOUSEMOVE:
	{

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

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpcmdline, int ncmdshow)
{
	WINUTIL::Init(hInstance);

	GameMain();

	return 0;
}

