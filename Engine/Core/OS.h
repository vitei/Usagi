/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Singleton baseclass
*****************************************************************************/
#ifndef _USG_CORE_OS_
#define	_USG_CORE_OS_


#include OS_HEADER(Engine/Core, os_ps.h)

namespace usg
{

	class OS
	{
	public:
		static void Initialize();
		static void ShutDown();
		static void Update();
		static bool GetShouldAllowApplicationEnterBackground();
		static void SetShouldAllowApplicationEnterBackground(bool value);
		static bool DidPreventEnterBackground();
		static CriticalSection& GetSaveDataCriticalSection();
		static bool ShouldQuit();
		static void RequestExit();
		static void ShowManual();
	};

}

#endif
