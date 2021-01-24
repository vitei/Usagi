#include "Engine/Common/Common.h"
#include "Engine/Core/Utility.h"
#include "Engine/Graphics/Device/GFXDevice.h"
#include "Engine/Audio/Audio.h"
#include "Engine/Audio/MusicManager.h"
#include "Engine/Scene/Common/Fader.h"
#include "Engine/Resource/ResourceMgr.h"
#include "Engine/Graphics/Device/Display.h"
#include "Engine/Framework/ComponentManager.h"
#include "Engine/Layout/StringTable.h"
#include "Engine/HID/Input.h"
#include "Engine/Physics/PhysX.h"
#include "Engine/Game/InitThread.h"
#include "Engine/Graphics/Device/GFXContext.h"
#include "Engine/Network/UsagiInetCore.h"
#include "SimpleGameBase.h"

//#ifdef PLATFORM_SWITCH
#define USE_THREADED_LOADING
//#endif


namespace usg
{


	struct SimpleGameBase::InternalData
	{
		usg::unique_ptr<usg::InitThread> m_pInitThread;
		usg::unique_ptr<usg::UsagiInetCore> m_pUsagiInetCore;
	};


	SimpleGameBase::SimpleGameBase() :
		Inherited(),
		m_pInternalData(vnew(usg::ALLOC_OBJECT)InternalData())
	{
		m_pActiveMode = NULL;
	}

	SimpleGameBase::~SimpleGameBase()
	{
		ASSERT(m_pActiveMode == nullptr);


		usg::physics::deinit();
	}

	void SimpleGameBase::Init(usg::GFXDevice* pDevice, ResourceMgr* pResMgr)
	{
		m_transitionRenderPass = pDevice->GetDisplay(0)->GetRenderPass();

		m_pInternalData->m_pInitThread.reset(vnew(usg::ALLOC_OBJECT)usg::InitThread());
		m_pInternalData->m_pInitThread->Init(pDevice, GetLoadFunc());
		m_pInternalData->m_pUsagiInetCore.reset(vnew(usg::ALLOC_NETWORK)usg::UsagiInetCore());
		usg::physics::init();
		usg::Fader::Create()->Init(pDevice, m_transitionRenderPass);
		usg::Audio::Create()->Init();
		usg::MusicManager::Create();
		
		m_eState = STATE_LOADING;

		m_debugRender.Init(pDevice, pResMgr, m_transitionRenderPass);
		m_debugRender.SetDrawArea(0.0f, 0.0f, 1280.f, 720.f);
		m_debug.Init(pDevice, &m_debugRender);
		m_debug.RegisterCPUTimer(&m_cpuTimer);

		m_pTransitionMode = CreateTransitionMode(pDevice, pResMgr);
	}

	usg::ModeTransition* SimpleGameBase::CreateTransitionMode(usg::GFXDevice* pDevice, usg::ResourceMgr* pResMgr)
	{
		usg::ModeTransition* pTransition = vnew(ALLOC_OBJECT) usg::ModeTransition;
		pTransition->Init(pDevice, pResMgr);
		return pTransition;
	}

	void SimpleGameBase::FinishedStaticLoad(usg::GFXDevice* pDevice)
	{
		// Clean this up...
		usg::mem::AddMemoryTag();
		usg::ResourceMgr::Inst()->FinishedStaticLoad();
		pDevice->FinishedStaticLoad();
	}

	void SimpleGameBase::Cleanup(usg::GFXDevice* pDevice)
	{
		if (m_pActiveMode)
		{
			pDevice->WaitIdle();
			m_pActiveMode->Cleanup(pDevice);
			vdelete m_pActiveMode;
			m_pActiveMode = nullptr;

			usg::ResourceMgr::Inst()->ClearDynamicResources(pDevice);
			pDevice->ClearDynamicResources();
			mem::FreeToLastTag();
		}


		usg::Fader::Inst()->CleanUpDeviceData(pDevice);
		usg::Fader::Cleanup();
		m_debugRender.Cleanup(pDevice);
	}

	//----------------------------------------------------
	void SimpleGameBase::StartNextMode(usg::GFXDevice* pDevice)
	{
		if (m_pActiveMode && !PauseCurrentMode())
		{
			m_pActiveMode->Cleanup(pDevice);
		}
		m_pInternalData->m_pInitThread->SetNextMode(&m_pActiveMode, GetNextMode(), PauseCurrentMode());
#ifdef USE_THREADED_LOADING
		m_pInternalData->m_pInitThread->StartThread(4096 * 10);
#else
		m_pInternalData->m_pInitThread->Run();
#endif

	}

	//----------------------------------------------------
	void SimpleGameBase::Update(usg::GFXDevice* pDevice)
	{
		m_cpuTimer.ClearAndStart();
		m_timer.Update();
		float fElapsed = m_timer.GetDeltaGameTime();
		bool bFinished = true;
		usg::Fader::Inst()->Update(fElapsed);
		usg::Fader::Inst()->GPUUpdate(pDevice);
		m_debug.Update(fElapsed);

		PreModeUpdate(fElapsed);

		switch (m_eState)
		{
		case STATE_ACTIVE:
			bFinished = m_pActiveMode->Update(fElapsed);

			if (bFinished)
			{
				usg::Fader::Inst()->StartFade(usg::Fader::FADE_OUT);
				m_eState = STATE_FADE_OUT;
				ModeFinished();
			}
			break;
		case STATE_FADE_OUT:
			if (usg::Fader::Inst()->IsBlackout())
			{
				// DeInitHomeButtonDisabledAnimation();
				m_eState = STATE_TRANSITION;
				m_pTransitionMode->Reset();
				if( DrawLoadingScreen() )
				{
					usg::Fader::Inst()->StartFade(usg::Fader::FADE_IN);
				}
				usg::Audio::Inst()->StopAll(AUDIO_TYPE_SFX, 0.12f);
			}
			else
			{
				m_pActiveMode->Update(fElapsed);
			}
			break;
		case STATE_TRANSITION:
			if (!DrawLoadingScreen() || m_pTransitionMode->Update(fElapsed))
			{
				StartNextMode(pDevice);
				m_eState = STATE_LOADING;
				// Normally we would fade in, but we are doing that manually due to the connection method;
			}
			break;
		case STATE_LOADING:
			if(DrawLoadingScreen())
			{
				m_pTransitionMode->Update(fElapsed);
			}
#ifdef USE_THREADED_LOADING
			if (m_pInternalData->m_pInitThread->IsThreadEnd())
#endif
			{
				m_pInternalData->m_pInitThread->JoinThread();	// Make sure to finalize so we can re-use it
				m_eState = STATE_END_LOADING;
				if(DrawLoadingScreen())
				{
					usg::Fader::Inst()->StartFade(usg::Fader::FADE_OUT);
				}
			}
			break;
		case STATE_END_LOADING:
			if (!DrawLoadingScreen() || usg::Fader::Inst()->IsBlackout())
			{
				m_eState = STATE_ACTIVE;
				m_pActiveMode->Update(fElapsed);	// Run the update first so everything is valid
				usg::Fader::Inst()->StartFade(usg::Fader::FADE_IN);
				usg::File::ResetReadTime();
			}
			else
			{
				m_pTransitionMode->Update(fElapsed);
			}
			break;
		}

		m_debug.Draw();


		usg::MusicManager::Inst()->Update(fElapsed);
		usg::Audio::Inst()->Update(fElapsed);

#ifdef PLATFORM_PC
		if (usg::Input::GetGamepad(0)->GetButtonDown(usg::GAMEPAD_BUTTON_START, usg::BUTTON_STATE_HELD) && usg::Input::GetGamepad(0)->GetButtonDown(usg::GAMEPAD_BUTTON_SELECT, usg::BUTTON_STATE_HELD))
		{
			m_bIsRunning = false;
		}
#endif
	}
	//----------------------------------------------------
	void SimpleGameBase::Draw(usg::GFXDevice* pDevice)
	{
		usg::Display* pDisplay = pDevice->GetDisplay(0);
		usg::IHeadMountedDisplay* pHMD = pDevice->GetHMD();
		usg::Mode* pRenderMode = nullptr;
		switch (m_eState)
		{
		case STATE_END_LOADING:
		case STATE_LOADING:
		case STATE_TRANSITION:
			pRenderMode = m_pTransitionMode;
			break;
		default:
			pRenderMode = m_pActiveMode;
			break;
		};

		m_cpuTimer.Pause();
		pDevice->Begin();
		m_cpuTimer.Start();
		usg::GFXContext* pImmContext = pDevice->GetImmediateCtxt();
		pImmContext->Begin(true);

		pRenderMode->PreDraw(pDevice, pImmContext);
		m_debugRender.Updatebuffers(pDevice);
		pRenderMode->Draw(pDisplay, pHMD, pImmContext);
		if (!pRenderMode->FinalTargetIsDisplay())
		{
			pImmContext->RenderToDisplay(pDisplay);
		}
		OverlayRender(pImmContext, pDisplay, pHMD);
		m_debugRender.Draw(pImmContext);
		usg::Fader::Inst()->Draw(pImmContext, true);
		pDisplay->Present();
		pRenderMode->PostDraw(pDevice);
		m_debug.PostDraw(pDevice);
		m_debugRender.Clear();

		m_cpuTimer.Pause();
		pImmContext->End();
		pDevice->End();
		m_cpuTimer.Start();
	}
	//----------------------------------------------------
	void SimpleGameBase::OnMessage(usg::GFXDevice* const pDevice, const uint32 messageID, const void* const pParameters)
	{
		switch (messageID)
		{
		case 'WSZE':
		{
			usg::Display* const pDisplay = pDevice->GetDisplay(0);
			uint32 uWidth, uHeight;
			uint32 uWidthOld, uHeightOld;

			pDisplay->GetDisplayDimensions(uWidthOld, uHeightOld, false);
			pDisplay->Resize(pDevice); // Before obtaining dimensions, we need to force display to update internal size
			pDisplay->GetDisplayDimensions(uWidth, uHeight, false);
			// Could be an eroneous call if restoring from being minimized
			if (m_pActiveMode && (uWidthOld != uWidth || uHeightOld != uHeight))
			{
				m_pActiveMode->NotifyResize(pDevice, 0, uWidth, uHeight);
			}
			m_pTransitionMode->NotifyResize(pDevice, 0, uWidth, uHeight);
			m_pInternalData->m_pInitThread->NotifyResize(pDevice, 0, uWidth, uHeight);
		}
		break;
		case 'WMIN':
		{
			usg::Display* const pDisplay = pDevice->GetDisplay(0);

			pDisplay->Minimized(pDevice);

		}
		case 'ONSZ':
		{
			// About to resize
			pDevice->WaitIdle();
		}
		break;
		default:
			// Does nothing
			break;
		}
	}

}