/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Main entry point into the project for windows
*****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Game/GameInterface.h"
#include "Engine/Graphics/GFX.h"
#include "Engine/Graphics/Device/GFXDevice.h"
#include "Engine/HID/Input.h"
#include "Engine/Framework/ComponentGetter.h"
#include "Engine/Framework/ComponentSystemInputOutputs.h"
#include "Engine/Audio/Audio.h"
#include "Engine/Core/Containers/List.h"
#include "Engine/Core/File/File.h"
#include "Engine/Maths/MathUtil.h"
#include "Engine/Physics/PhysX.h"
#include "Engine/Core/Modules/ModuleManager.h"
#include "Engine/Resource/ResourceMgr.h"
#if (defined PLATFORM_PC || defined PLATFORM_SWITCH_EMU)
#include "Engine/Core/_win/WinUtil.h"
#endif
#include "Engine/Core/OS.h"

namespace usg {

bool		InitEngine(const char** dllModules, uint32 uModuleCount);
void		EngineCleanup();

static GFXDevice*	g_pGFXDevice = nullptr;
static bool			g_gameExit = false;

bool GameExit()
{
	g_gameExit = true;
	return true;
}

GFXDevice* GetGFXDevice()
{
	return g_pGFXDevice;
}


static void error_callback(int error, const char* description)
{
    fputs(description, stderr);
}

static GameInterface* game;

bool GameInit()
{
	game = CreateGame();

	Math::SeedRand();
	game->Init(g_pGFXDevice, usg::ResourceMgr::Inst());

	if (!game->IsRunning())
	{
		FATAL_RELEASE(false, "Game failed to initialise");
		return false;
	}

	return true;
}

void GameLoop()
{
	Input::Update(g_pGFXDevice);
	// FIXME: Remove timing from this input (not needed)
	game->Update(g_pGFXDevice);
	if(GFX::HasFocus())
	{
		game->Draw(g_pGFXDevice);		
	}
	GFX::PostUpdate();
	OS::Update();
}

void GameCleanup()
{
	game->Cleanup(g_pGFXDevice);
	vdelete game;
	game = nullptr;
	ResourceMgr::Cleanup(g_pGFXDevice);
	EngineCleanup();
}

bool GameMain(const char** dllModules, uint32 uModuleCount)
{
	if (OS::ShouldQuit())
	{
		return true;
	}

	if (!InitEngine(dllModules, uModuleCount))
	{
		FATAL_RELEASE(false, "Hardware setup failed");
		return false;
	}

	if(!GameInit())
		return false;

	if (OS::ShouldQuit())
	{
		GameExit();
		GameCleanup();
		return true;
	}

	// enter main event loop
	while(game->IsRunning() && !g_gameExit )
	{
		GameLoop();

		if(game->ResetReq())
		{
			//mem::FreezeStack(false);
			game = CreateGame();
			game->Init(g_pGFXDevice, usg::ResourceMgr::Inst());
			//mem::FreezeStack(true);
		}
		if (OS::ShouldQuit())
		{
			break;
		}
	}

	GameCleanup();
    
	return true;
}

void GameMessage(const uint32 messageID, const void* const pParameters)
{
	if (game != nullptr)
	{
		game->OnMessage(g_pGFXDevice, messageID, pParameters);
	}
}

bool InitInput()
{
	Input::Init();

	return true;
}

bool InitEngine(const char** dllModules, uint32 uModuleCount)
{
	mem::InitialiseDefault();
	U8String::InitPool();
	InitListMemory();
	File::InitFileSystem();

	usg::ModuleManager::Inst()->Create();
	usg::ModuleManager::Inst()->PreInit();
	for (uint32 i = 0; i < uModuleCount; i++) 
	{
		// We pre-load as some of these may need to be known about when initing gfx and input
		usg::ModuleManager::Inst()->LoadModule(dllModules[i]);
	}
    
    if(!InitInput())
	{
		return false;
	}

	g_pGFXDevice = GFX::Initialise();

	if(!g_pGFXDevice)
	{
		return false;
	}
#if (defined PLATFORM_PC)
	g_pGFXDevice->InitDisplay(WINUTIL::GetWindow());
#else
	g_pGFXDevice->InitAllHardwareDisplays();
#endif

	// Moved these out of here to prevent dependencies, should probably remove this file
	//Audio::Create()->Init();

	//physics::init();

	// Everything should be set up now, time to finalize any early load modules
	usg::ModuleManager::Inst()->PostInit(g_pGFXDevice);

	return true;
}


void EngineCleanup()
{ 
	//physics::deinit();
	Input::Cleanup();
	File::FinalizeFileSystem();
	GFX::Reset();
	U8String::CleanupPool();
	ComponentSystemInputOutputsSharedBase::Cleanup();
	usg::ModuleManager::Inst()->Cleanup();
	mem::Cleanup();
}

}


