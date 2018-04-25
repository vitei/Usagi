/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Platform specific thread functions
****************************************************************************/
#ifndef _USG_ENGINE_THREAD_THREAD_PS_H_
#define	_USG_ENGINE_THREAD_THREAD_PS_H_
#include "Engine/Common/Common.h"

namespace usg
{

	typedef  void (*ThreadFunc)(void*);

class Thread_ps
{
public:
	static const size_t DEFAULT_THREAD_STACK_SIZE = 0;	// Default
	static const int	DEFAULT_THREAD_PRIORITY = THREAD_PRIORITY_NORMAL;

	Thread_ps() { m_hThread = NULL; }

	~Thread_ps() { }


	void StartThread( void (*f)(void*), void* pData, uint32 uStackSize, int threadPriority )
	{
		ASSERT(m_hThread == NULL);
		m_threadFunc = f;
		m_pData = pData;
		m_hThread = CreateThread( 
			NULL,                   // default security attributes
			uStackSize,                 // use default stack size  
			CallFromThread,				// thread function name
			this,				// argument to thread function 
			0,					// use default creation flags 
			&m_dwThreadId);   // returns the thread identifier 
		SetThreadPriority(m_hThread, threadPriority);
	}


	void Join( void ) { WaitForSingleObject(m_hThread, INFINITE); }
	bool IsAlive();
	bool IsValid() { return m_hThread != NULL; }
	void Detach() {}
	void Finalize() { CloseHandle(m_hThread); m_hThread = NULL; }

	static void Sleep( uint32 uMilliseconds ) { ::Sleep(uMilliseconds); }

private:
	ThreadFunc	m_threadFunc;
	void*		m_pData;

	static DWORD WINAPI CallFromThread( LPVOID pParam );

	HANDLE	m_hThread;
	DWORD	m_dwThreadId;
};

}

#endif // #ifndef _USG_ENGINE_THREAD_THREAD_PS_H_
