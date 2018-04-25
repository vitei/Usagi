/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include OS_HEADER(Engine/Core/Timer, TimeTracker.h)
#include "ProfilingTimer.h"

#ifdef ENABLE_PROFILE_TIMERS
namespace usg {

ProfilingTimer::ProfilingTimer() :
	m_bTimerIsRunning(false),
	m_uTimerStart(0),
	m_uTimer(0),
	m_uTimerTotal(0),
	m_uTimerIncrement(0)
{
	Clear();
}

void ProfilingTimer::Clear()
{
	m_uTimer = 0;
	m_uTimerTotal = 0;
	m_uTimerIncrement = 0;
}

void ProfilingTimer::Start()
{
	m_uTimerStart = TimeTracker::GetSystemTime();
	m_bTimerIsRunning = true;
}

void ProfilingTimer::Pause()
{
	m_bTimerIsRunning = false;
	m_uTimer = ( TimeTracker::GetSystemTime() - m_uTimerStart );
	m_uTimerTotal += m_uTimer;
}

void ProfilingTimer::Stop()
{
	if(m_bTimerIsRunning)
	{
		m_bTimerIsRunning = false;
		uint64 t = TimeTracker::GetSystemTime();
		m_uTimer = (t - m_uTimerStart);

		m_uTimerTotal += m_uTimer;
		m_uTimerIncrement++;
	}
}

uint64 ProfilingTimer::Read()
{
	if( m_bTimerIsRunning )
	{
		Stop();
		Start();
	}

	return m_uTimer;
}

float ProfilingTimer::GetSeconds()
{
	return ((float)Read()) / ((float)TimeTracker::GetTickFrequency());
}

float ProfilingTimer::GetTotalSeconds() const
{
	return ((float)m_uTimerTotal) / ((float)TimeTracker::GetTickFrequency());
}

float ProfilingTimer::GetMilliSeconds() 
{
	return ((float)Read())/((float)(TimeTracker::GetTickFrequency()))*1000.f;
}


float ProfilingTimer::GetAverageSeconds() const
{
	return ((float)m_uTimerTotal / (float)m_uTimerIncrement) / ((float)TimeTracker::GetTickFrequency());
}

float ProfilingTimer::GetAverageMilliSeconds() const
{
	return ((float)(m_uTimerTotal) / (float)m_uTimerIncrement) / ((float)(TimeTracker::GetTickFrequency()/1000));
}


float ProfilingTimer::GetTotalMilliSeconds() const
{
	return (float)(m_uTimerTotal) / ((float)(TimeTracker::GetTickFrequency()/1000));
}

}
#endif

