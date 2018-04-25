/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Core/Thread/Thread.h"

namespace usg
{
	bool Thread_ps::IsAlive()
	{
		DWORD exitCode;
		if(!GetExitCodeThread(m_hThread, &exitCode))
			return false;

		return exitCode == STILL_ACTIVE ;
	}

	DWORD WINAPI Thread_ps::CallFromThread( LPVOID pParam )
	{
		Thread_ps* pThread = (Thread_ps*)pParam;
		pThread->m_threadFunc(pThread->m_pData);
		return 0;
	}
}