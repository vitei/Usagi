/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include OS_HEADER(Engine/HID, Input_ps.h)
#include "Input.h"

namespace usg{

static Input_ps	g_platform; 
enum
{
	MAX_CONTROLLERS = 8,
};

static Gamepad	g_gamepads[MAX_CONTROLLERS];
static uint32	g_uGamepads;

void Input::Init()
{
	g_platform.Init();
	RenumberGamepads();
}

void Input::RenumberGamepads()
{
	g_platform.RegisterDeviceChange();
	usg::vector<IGamepad*> gamepads;
	g_platform.GetActiveGamepads(gamepads);
	g_uGamepads = 0;
	for (auto it : gamepads)
	{
		g_gamepads[g_uGamepads++].BindHardware(it);

		if (g_uGamepads == MAX_CONTROLLERS)
			break;
	}
}

void Input::Update(GFXDevice* pDevice)
{
	g_platform.Update(pDevice);

	for (uint32 i = 0; i < g_uGamepads; i++)
	{
		g_gamepads[i].Update(pDevice);
	}
}

void Input::RegisterGamepad(IGamepad* pGamepad)
{
	g_platform.RegisterGamepad(pGamepad);
	RenumberGamepads();
}

Gamepad* Input::GetGamepad(uint32 uGamepad)
{
	if (uGamepad < g_uGamepads)
	{
		return &g_gamepads[uGamepad];
	}

	return nullptr;
}


Microphone* Input::GetMicrophone(uint32 uMicrophone)
{
	return g_platform.GetMicrophone(uMicrophone);
}

Keyboard* Input::GetKeyboard()
{
	return g_platform.GetKeyboard();
}

Mouse* Input::GetMouse()
{
	return g_platform.GetMouse();
}

uint32 Input::GetGamepadCount()
{
	return g_uGamepads;
}

Input_ps& Input::GetPlatform()
{
	return g_platform;
}

Gamepad* Input::GetAccerometer()
{
	return g_platform.GetAccerometer();
}

}
