/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "TimeTracker.h"

namespace usg {

TimeTracker::TimeTracker()
{
	QueryPerformanceFrequency(&m_ticksPerSecond);
	QueryPerformanceCounter(&m_lastTick);
	m_firstTick = m_lastTick;
}

float TimeTracker::GetDeltaTime()
{	
	LARGE_INTEGER now;
	int passed;
	QueryPerformanceCounter(&now);
	
	passed = (int)(now.QuadPart - m_lastTick.QuadPart);
	
	m_lastTick = now;
	return (float)passed/m_ticksPerSecond.QuadPart;
}

float TimeTracker::PeekDeltaTime() const
{
	LARGE_INTEGER now;
	int passed;
	QueryPerformanceCounter(&now);
	passed = (int)(now.QuadPart - m_lastTick.QuadPart);

	return (float)passed/m_ticksPerSecond.QuadPart;
}

float TimeTracker::GetTotalTime() const
{
	LARGE_INTEGER now;
	int passed;
	QueryPerformanceCounter(&now);
	passed = (int)(now.QuadPart - m_firstTick.QuadPart);

	return (float)passed/m_ticksPerSecond.QuadPart;
}

uint64 TimeTracker::GetSystemTime()
{
	LARGE_INTEGER now;
	QueryPerformanceCounter(&now);
	return now.QuadPart;
}

uint64 TimeTracker::GetTickFrequency()
{
	LARGE_INTEGER ticks;
	QueryPerformanceFrequency(&ticks);
	return ticks.QuadPart;
}

}

