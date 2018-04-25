/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Core/_win/os_ps.h"

namespace usg {

	static CriticalSection g_saveDataCriticalSection;
	bool OS_ps::m_bExitRequested = false;

	void OS_ps::RequestExit()
	{
		m_bExitRequested = true;
	}


	void OS_ps::ShowManual()
	{
		DEBUG_PRINT("trying to show manual on windows...\n");
	}

	void OS_ps::Initialize()
	{
		g_saveDataCriticalSection.Initialize();
	}

	bool OS_ps::ShouldQuit()
	{
		return m_bExitRequested;
	}

	CriticalSection& OS_ps::GetSaveDataCriticalSection()
	{
		return g_saveDataCriticalSection;
	}

	void OS_ps::Update()
	{
	}

	void OS_ps::ShutDown()
	{
	}

	bool OS_ps::GetShouldAllowApplicationEnterBackground()
	{
		return true;
	}

	void OS_ps::SetShouldAllowApplicationEnterBackground(bool)
	{

	}

}
