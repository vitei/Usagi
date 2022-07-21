#pragma once
#ifndef GAME_SIMPLE_GAME_BASE_H
#define GAME_SIMPLE_GAME_BASE_H

#include "Engine/Game/GameInterface.h"
#include "Engine/Core/Timer/Timer.h"
#include "Engine/Debug/DebugStats.h"
#include "Engine/Core/stl/stack.h"
#include "Engine/Core/Timer/ProfilingTimer.h"
#include "Engine/Game/InitThread.h"
#include "Engine/Game/ModeTransition.h"

namespace usg
{

	class InitThread;

	class SimpleGameBase : public usg::GameInterface
	{
		typedef GameInterface Inherited;
	public:
		explicit SimpleGameBase();
		virtual ~SimpleGameBase();

		virtual void Init(usg::GFXDevice* pDevice, usg::ResourceMgr* pResMgr) override;
		virtual void Cleanup(usg::GFXDevice* pDevice) override;
		virtual void Update(usg::GFXDevice* pDevice) override;
		virtual void Draw(usg::GFXDevice* pDevice) override;
		virtual void OnMessage(usg::GFXDevice* const pDevice, const uint32 messageID, const void* const pParameters) override;
	
	protected:
		void HandleMessages(usg::GFXDevice* pDevice);
		bool IsDoingThreadedLoading() const;

		virtual bool DrawLoadingScreen() const { return true; }
		virtual bool PauseCurrentMode() const { return false; }
		virtual void PreModeUpdate(float fElapsed) {}
		virtual void OverlayRender(usg::GFXContext* pImmContext, Display* pDisplay, IHeadMountedDisplay* pHMD) {}
		void StartNextMode(usg::GFXDevice* pDevice);
		virtual uint32 GetNextMode() const = 0;
		virtual void ModeFinished() = 0;
		virtual ModeLoadFunc GetLoadFunc() const = 0;
		virtual usg::ModeTransition* CreateTransitionMode(usg::GFXDevice* pDevice, usg::ResourceMgr* pResMgr);
		virtual usg::Mode* CreateSplashMode(usg::GFXDevice* pDevice, usg::ResourceMgr* pResMgr) { return nullptr; }
		void FinishedStaticLoad(usg::GFXDevice* pDevice);
		virtual void PostSplashInit(usg::GFXDevice* pDevice, ResourceMgr* pResMgr);

		enum State
		{
			STATE_ACTIVE = 0,
			STATE_FADE_OUT,
			STATE_TRANSITION,
			STATE_LOADING,
			STATE_END_LOADING,
			STATE_SPLASH
		};


		usg::RenderPassHndl		m_transitionRenderPass;
		usg::DebugRender		m_debugRender;
		usg::DebugStats			m_debug;
		usg::Mode*				m_pActiveMode;
		usg::ModeTransition*	m_pTransitionMode;
		usg::Timer				m_timer;
		usg::ProfilingTimer		m_cpuTimer;
		State					m_eState;
		usg::stack<DWORD>		m_messages;


		struct InternalData;
		usg::unique_ptr<InternalData> m_pInternalData;
	};

}

#endif // GAME_SIMPLE_GAME_BASE_H