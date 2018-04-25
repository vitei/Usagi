/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "TimeTracker.h"
#include <dispatch/dispatch.h>
#include <stdio.h>
#include <sys/time.h>

#include "glfw3.h"

using namespace usg;

#define TICKS_PER_SECOND 1000000

TimeTracker::TimeTracker()
{
    m_ticks = 0;
    m_ticksLastRead = 0;
}

uint64 TimeTracker::GetTickFrequency()
{
    return TICKS_PER_SECOND;
}

float TimeTracker::GetDeltaTime()
{
	uint64 m_ticks = GetSystemTime();
	
	uint64 t = m_ticks - m_ticksLastRead;
	
    m_ticksLastRead = m_ticks;
    
    return ((float)(t)) / (float)TICKS_PER_SECOND;
}

float TimeTracker::GetTotalTime() const
{
    return ((float)(m_ticks)) / (float)TICKS_PER_SECOND;
}

uint64 TimeTracker::GetSystemTime()
{
	/*
    timeval time;
    gettimeofday(&time,NULL);
    uint64 ticks = (time.tv_sec * TICKS_PER_SECOND) + (time.tv_usec / (1000000/TICKS_PER_SECOND));
    return ticks;
	 */
	
	return (uint64) (glfwGetTime() * TICKS_PER_SECOND);
	
	
}
