#include "Engine/Common/Common.h"
#include "Engine/Core/OS.h"
#include "Engine/Graphics/Device/GFXDevice.h"
#include "Engine/Resource/ResourceMgr.h"
#include "Engine/Framework/ModelMgr.h"
#include "Engine/Game/Mode.h"
#include "InitThread.h"

namespace usg
{

	InitThread::InitThread()
		: m_pDevice(NULL)
		, m_ppLoadMode(NULL)
		, m_uNextMode(0)
		, m_bPauseMode(false)
	{
	}

	InitThread::~InitThread()
	{

	}

	void InitThread::Init(usg::GFXDevice* pDevice, ModeLoadFunc fnLoad)
	{
		m_pDevice = pDevice;
		m_fnLoad = fnLoad;
	}


	bool InitThread::IsModePaused(uint32 uMode) const
	{
		return m_pausedModes.find(uMode) != m_pausedModes.end();
	}

	void InitThread::SetNextMode(Mode** ppLoadMode, uint32 uNextMode, bool bPauseMode)
	{
		ASSERT(m_pDevice != nullptr);
		if (bPauseMode)
		{
			m_pausedModes[m_uNextMode] = *m_ppLoadMode;
		}
		m_uNextMode = uNextMode;
		m_ppLoadMode = ppLoadMode;
		m_bPauseMode = bPauseMode;
	}

	void InitThread::Cleanup(usg::GFXDevice* pDevice)
	{
		for (auto itr : m_pausedModes)
		{
			itr.second->Cleanup(pDevice);
			vdelete itr.second;

		}
		m_pausedModes.clear();
	}

	void InitThread::NotifyResize(GFXDevice* pDevice, uint32 uDisplay, uint32 uWidth, uint32 uHeight)
	{
		for (auto itr : m_pausedModes)
		{
			itr.second->NotifyResize(pDevice, uDisplay, uWidth, uHeight);
		}
	}

	void InitThread::NotifyDeviceChange()
	{
		for (auto itr : m_pausedModes)
		{
			itr.second->NotifyDeviceChange();
		}
	}

	void InitThread::Run(bool bThreaded)
	{
		// We can't clear the memory if we are pausing
		if(!m_bPauseMode)
		{
			// FIXME: Moving into simple game mode for now as handles such as audio aren't thread safe
			//(*m_ppLoadMode)->Cleanup(m_pDevice);
			//vdelete* m_ppLoadMode;
		}
		if (m_pausedModes.empty())
		{
			// FIXME: We can only clear the resources if there are no paused modes!
			// Rewrite to use tagging system for all resources
			// FIXME: 
			// Disabling cleanup between modes for now as not needed for demo which keeps re-running the same mission and introduces bugs
			// FIXME: 
			// FIXME: 
			usg::ResourceMgr::Inst()->ClearDynamicResources(m_pDevice);
			m_pDevice->ClearDynamicResources();
			mem::FreeToLastTag();
		}
		*m_ppLoadMode = NULL;

		// Probably overkill unless going from one heavy scene to another
		#if 0
		uint32 uStartFrame = m_pDevice->GetFrameCount();
		if(bThreaded)
		{
			// FIXME: In single threaded case we should wait a few frames
			// Wait a few frames for resources to release keeping VRAM usage as low as possible
			while (m_pDevice->GetFrameCount() < (uStartFrame + 6))
			{
				Thread::Sleep(5);
			}
		}
		#endif

		if(m_pausedModes.find(m_uNextMode) == m_pausedModes.end())
		{
			// Create the mode
			m_fnLoad(m_uNextMode, m_ppLoadMode);

			(*m_ppLoadMode)->Init(m_pDevice, usg::ResourceMgr::Inst());
		}
		else
		{
			// Re-use one that went into hibernation
			*m_ppLoadMode = m_pausedModes[m_uNextMode];
			m_pausedModes.erase(m_uNextMode);
		}
	}

	void InitThread::Exec()
	{
		Run(true);
		EndThread();
	}
}

