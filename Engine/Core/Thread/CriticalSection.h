/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef _USG_ENGINE_CRITICAL_SECTION_H_
#define	_USG_ENGINE_CRITICAL_SECTION_H_

#include OS_HEADER(Engine/Core/Thread, CriticalSection_ps.h)

namespace usg
{

class CriticalSection
{
public:

	CriticalSection() { m_bInitialised = false; }
	~CriticalSection()
	{
		if (m_bInitialised)
		{
			Finalize();
		}
	}

	class ScopedLock
	{
	public:
		ScopedLock(CriticalSection& critical) : m_critSec(critical)
		{
			m_critSec.Enter();
		}

		~ScopedLock()
		{
			m_critSec.Leave();
		}

	private:
		CriticalSection& m_critSec;
	};

	void Initialize() { ASSERT(!m_bInitialised); m_bInitialised = true; return m_platform.Initialize(); }
	void Finalize() { ASSERT(m_bInitialised); m_bInitialised = false; return m_platform.Finalize(); }

	void Enter() { ASSERT(m_bInitialised); m_platform.Enter(); }
	void Leave() { ASSERT(m_bInitialised); m_platform.Leave(); }
	bool TryEnter() { ASSERT(m_bInitialised);  return m_platform.TryEnter(); }

private:
	CriticalSection_ps	m_platform;
	bool				m_bInitialised;
};

}

#endif
