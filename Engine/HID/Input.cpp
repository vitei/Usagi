/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include OS_HEADER(Engine/HID, Input_ps.h)
#include "Input.h"

namespace usg{

static Input_ps	g_platform; 
static bool g_bInitCalled = false;
enum
{
	MAX_CONTROLLERS = 8,
};

static Gamepad	g_gamepads[MAX_CONTROLLERS];
static uint32	g_uGamepads;

void Input::Init()
{
	g_platform.Init();
	g_bInitCalled = true;
	RenumberGamepads();
}

void Input::Cleanup()
{
	g_platform.Cleanup();
}

void Input::RenumberGamepads(usg::vector<uint32> uCapRequestList)
{
	if(!g_bInitCalled)
		return;

	g_platform.RegisterDeviceChange();
	usg::vector<IGamepad*> gamepads;
	g_platform.GetActiveGamepads(gamepads);
	g_uGamepads = 0;

	usg::vector<bool> bound;
	bound.resize(gamepads.size());
	for (memsize i=0; i<bound.size(); i++)
	{
		bound[i] = false;
	}

	if(uCapRequestList.size() > 0)
	{
		for (auto it : uCapRequestList)
		{
			for(memsize i=0; i<gamepads.size(); i++)
			{
				if(bound[i])
					continue;

				uint32 uPrefferedCaps = it;
				if ((gamepads[i]->GetCaps() & uPrefferedCaps) == uPrefferedCaps)
				{
					bound[i] = true;
					g_gamepads[g_uGamepads++].BindHardware(gamepads[i]);
				}

				if (g_uGamepads == MAX_CONTROLLERS)
					break;
			}
		}
	}
	else
	{
		// Bind everything in any order
		for (auto it : gamepads)
		{
			g_gamepads[g_uGamepads++].BindHardware(it);

			if (g_uGamepads == MAX_CONTROLLERS)
				break;
		}
	}

}

void Input::Update(GFXDevice* pDevice, float fDelta)
{
	if(!g_bInitCalled)
		return;

	g_platform.Update(pDevice);

	for (uint32 i = 0; i < g_uGamepads; i++)
	{
		g_gamepads[i].Update(pDevice, fDelta);
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
