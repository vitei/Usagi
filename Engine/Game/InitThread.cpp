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

	void InitThread::SetNextMode(Mode** ppLoadMode, uint32 uNextMode)
	{
		ASSERT(m_pDevice != nullptr);
		m_uNextMode = uNextMode;
		m_ppLoadMode = ppLoadMode;
	}

	void InitThread::Run()
	{
		if (*m_ppLoadMode)
		{
			vdelete *m_ppLoadMode;
			*m_ppLoadMode = NULL;
			m_pDevice->ClearDynamicResources();
			usg::ResourceMgr::Inst()->ClearDynamicResources(m_pDevice);
			mem::FreeToLastTag();
		}

		m_fnLoad(m_uNextMode, m_ppLoadMode);

		(*m_ppLoadMode)->Init(m_pDevice);
	}

	void InitThread::Exec()
	{
		Run();
		EndThread();
	}
}

