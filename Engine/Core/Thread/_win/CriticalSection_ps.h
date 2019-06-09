/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef _USG_ENGINE_THREAD_CRITICAL_SECTION_PS_H_
#define	_USG_ENGINE_THREAD_CRITICAL_SECTION_PS_H_

#include <Synchapi.h>

namespace usg
{


class CriticalSection_ps
{
public:	
	CriticalSection_ps() {  }
	~CriticalSection_ps() {  }


	void Initialize() { InitializeCriticalSection(&m_cs); }
	void Finalize() { DeleteCriticalSection(&m_cs); }
	void Enter() { EnterCriticalSection(&m_cs); }
	void Leave() { LeaveCriticalSection(&m_cs); }

	bool TryEnter() { return TryEnterCriticalSection(&m_cs) == TRUE; }
	
private:
	CRITICAL_SECTION m_cs;
};

}

#endif // #ifndef _USG_ENGINE_THREAD_CRITICAL_SECTION_PS_H_
