/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Core/Thread/CriticalSection.h"

namespace usg{

class OS_ps
{
public:
	OS_ps();
	~OS_ps();

	static void Initialize();
	static void ShutDown();
	static void Update();
	static bool ShouldQuit();
	static bool GetShouldAllowApplicationEnterBackground();
	static bool DidPreventEnterBackground() { return false; }
	static void SetShouldAllowApplicationEnterBackground(bool bValue);
	static CriticalSection& GetSaveDataCriticalSection();
	static void RequestExit();
	static void ShowManual();
private:
	static bool m_bExitRequested;
};

}