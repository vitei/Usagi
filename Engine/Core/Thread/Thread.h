/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: A thread function which can be joined
****************************************************************************/
#ifndef _USG_ENGINE_THREAD_THREAD_H_
#define	_USG_ENGINE_THREAD_THREAD_H_

#include OS_HEADER(Engine/Core/Thread, Thread_ps.h)

namespace usg
{
	typedef void (*ThreadCallback) (void* pData);

class Thread
{
public:

	Thread( void )
	{
		m_bExec = false;
		m_uDelay = 0;
	}

	virtual ~Thread()
	{
		m_bExec = false;
	}

	void StartThread(uint32 uStackSize = Thread_ps::DEFAULT_THREAD_STACK_SIZE, int uPriority = Thread_ps::DEFAULT_THREAD_PRIORITY)
	{
		ASSERT( m_thread.IsValid() == false );
		m_bExec = true;
		m_thread.StartThread( Thread::CallFromThread, this, uStackSize, uPriority );
	}
	

	void EndThread( void )
	{
		m_bExec = false;
	}
	
	void JoinThread()
	{
		if( m_thread.IsValid() /*&& !m_thread.IsAlive()*/ )
		{
			m_thread.Join();
			m_thread.Finalize();
		}
	}

	// Not for general use, but we can't wait on the loading thread when the power button is pressed
	void DetachAndFinalize()
	{
		if( m_thread.IsValid() && !m_thread.IsAlive() )
		{
			m_thread.Detach();
			m_thread.Finalize();
		}	
	}
	
	void ThreadFunc()
	{
		if(m_uDelay > 0)
		{
			Thread_ps::Sleep(m_uDelay);
		}
		while( m_bExec )
		{
			Exec();
			if(m_bExec)
				Thread_ps::Sleep( 10 );
		}
	}
	

	bool IsThreadEnd()
	{
		return !m_thread.IsValid() || ( m_thread.IsValid() && !m_thread.IsAlive() );
	}
	
	virtual void Exec() = 0;

	static void Sleep( uint32 uMilliseconds ) { Thread_ps::Sleep( uMilliseconds ); }
	
protected:
	uint32		m_uDelay;	// Hide a stutter

private:
	
	static void CallFromThread( void* pParam )
	{
		Thread* pThread = (Thread*)pParam;
		pThread->ThreadFunc();
	}
	
	volatile bool	m_bExec;
	Thread_ps		m_thread;
};

}

#endif
