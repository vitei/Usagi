/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: For tracking game performance
*****************************************************************************/
#pragma once

#ifndef USG_PROFILE_TIMER_H
#define USG_PROFILE_TIMER_H
#include "Engine/Common/Common.h"

namespace usg {

#ifndef FINAL_BUILD
#define ENABLE_PROFILE_TIMERS
#endif

#ifdef ENABLE_PROFILE_TIMERS
class ProfilingTimer
{
public:
	ProfilingTimer();
	~ProfilingTimer() {}

	void ClearAndStart() { Clear(); Start(); }
	void Clear();
	void Start();
	void Pause();
	void Stop();
	uint64	Read();
	float GetMilliSeconds();
	float GetSeconds();

	float GetAverageMilliSeconds() const;
	float GetAverageSeconds() const;
	float GetTotalMilliSeconds() const;
	float GetTotalSeconds() const;
	uint64 GetTimerTotal() const { return m_uTimerTotal; }

private:

	bool	m_bTimerIsRunning;
	uint64	m_uTimerStart;
	uint64	m_uTimer;

	uint64	m_uTimerTotal;
	uint32	m_uTimerIncrement;
};
#else
	class ProfilingTimer
	{
	public:
		ProfilingTimer() {}
		~ProfilingTimer() {}

		void ClearAndStart() {}
		void Clear() {}
		void Start() {}
		void Pause() {}
		void Stop() {}
		uint64	Read() { return 1; }
		float GetMilliSeconds() { return 0.0f;  }
		float GetSeconds() { return 0.0f; }
		float GetTotalSeconds() const { return 0.0f; }

		float GetAverageMilliSeconds() const { return 0.0f; }
		float GetAverageSeconds() const { return 0.0f; }
		float GetTotalMilliSeconds() const { return 0.0f; }
		uint64 GetTimerTotal() const { return 1; }
	};
#endif

} // namespace usg

#endif // USG_PROFILE_TIMER_H
