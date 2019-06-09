#pragma once
#ifndef GAME_SIMPLE_GAME_BASE_H
#define GAME_SIMPLE_GAME_BASE_H

#include "Engine/Game/GameInterface.h"
#include "Engine/Core/Timer/Timer.h"
#include "Engine/Debug/DebugStats.h"
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

		virtual void Init(usg::GFXDevice* pDevice) override;
		virtual void CleanUp(usg::GFXDevice* pDevice) override;
		virtual void Update(usg::GFXDevice* pDevice) override;
		virtual void Draw(usg::GFXDevice* pDevice) override;
		virtual void OnMessage(usg::GFXDevice* const pDevice, const uint32 messageID, const void* const pParameters) override;

	protected:
		void StartNextMode(usg::GFXDevice* pDevice);
		virtual uint32 GetNextMode() const = 0;
		virtual void ModeFinished() = 0;
		virtual ModeLoadFunc GetLoadFunc() const = 0;

		enum State
		{
			STATE_ACTIVE = 0,
			STATE_FADE_OUT,
			STATE_TRANSITION,
			STATE_LOADING,
			STATE_END_LOADING
		};


		usg::RenderPassHndl	m_transitionRenderPass;
		usg::DebugRender	m_debugRender;
		usg::DebugStats		m_debug;
		usg::Mode*			m_pActiveMode;
		usg::Timer			m_timer;
		usg::ProfilingTimer m_cpuTimer;
		usg::ModeTransition	m_modeTransition;
		State				m_eState;

		struct InternalData;
		usg::unique_ptr<InternalData> m_pInternalData;
	};

}

#endif // GAME_SIMPLE_GAME_BASE_H