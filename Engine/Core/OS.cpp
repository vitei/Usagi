#include "Engine/Common/Common.h"
#include "Engine/Core/OS.h"

namespace usg
{

	void OS::RequestExit()
	{
		OS_ps::RequestExit();
	}


	void OS::ShowManual()
	{
		OS_ps::ShowManual();
	}

	void OS::Initialize()
	{
		OS_ps::Initialize();
	}

	void OS::ShutDown()
	{
		OS_ps::ShutDown();
	}

	bool OS::ShouldQuit()
	{
		return OS_ps::ShouldQuit();
	}

	bool OS::DidPreventEnterBackground()
	{
		return OS_ps::DidPreventEnterBackground();
	}

	void OS::Update()
	{
		OS_ps::Update();
	}

	bool OS::GetShouldAllowApplicationEnterBackground()
	{
		return OS_ps::GetShouldAllowApplicationEnterBackground();
	}	

	void OS::SetShouldAllowApplicationEnterBackground(bool bValue)
	{
		OS_ps::SetShouldAllowApplicationEnterBackground(bValue);
	}

	CriticalSection& OS::GetSaveDataCriticalSection()
	{
		return OS_ps::GetSaveDataCriticalSection();
	}
}